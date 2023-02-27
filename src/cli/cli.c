// SPDX-License-Identifier: Apache-2.0
/* Copyright (C) 2013-2019 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "common.h"
#include "ctype.h"
#include "printf.h"
#include "sleep.h"
#include "uarths.h"

/*
 * Rather than doubling the size of the _ctype lookup table to hold a 'blank'
 * flag, just check for space or tab.
 */
#define isblank(c) (c == ' ' || c == '\t')

static const char erase_seq[] = "\b \b"; /* erase sequence */
static const char tab_seq[] = "        "; /* used to expand TABs */

char console_buffer[CONFIG_SYS_CBSIZE + 1]; /* console I/O buffer */

static char *delete_char(char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0)
		return p;

	if (*(--p) == '\t') { /* will retype the whole line */
		while (*colp > plen) {
			printk(erase_seq);
			(*colp)--;
		}
		for (s = buffer; s < p; ++s) {
			if (*s == '\t') {
				printk(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				uart_putchar(*s);
			}
		}
	} else {
		printk(erase_seq);
		(*colp)--;
	}
	(*np)--;

	return p;
}

int cli_readline_into_buffer(const char *const prompt, char *buffer,
			     int timeout)
{
	char *p = buffer;

	char *p_buf = p;
	int n = 0; /* buffer index */
	int plen = 0; /* prompt length */
	int col; /* output column cnt */
	int c;

	/* print prompt */
	if (prompt) {
		plen = strlen(prompt);
		printk(prompt);
	}
	col = plen;

	for (;;) {
		msleep(100);

		c = uart_getc();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r': /* Enter */
		case '\n':
			*p = '\0';
			printk("\r\n");
			return p - p_buf;

		case '\0': /* nul */
			continue;

		case 0x03: /* ^C - break */
			p_buf[0] = '\0'; /* discard input */
			return -1;

		case 0x15: /* ^U - erase line */
			while (col > plen) {
				printk(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17: /* ^W - erase word */
			p = delete_char(p_buf, p, &col, &n, plen);
			while ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		case 0x08: /* ^H  - backspace */
		case 0x7F: /* DEL - backspace */
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/*
			 * Ignore non-printable, e.g. control escape sequences
			 */
			if (!isprint(c)) {
				continue;
			}
			/*
			 * Must be a normal character then
			 */
			if (n < CONFIG_SYS_CBSIZE - 2) {
				if (c == '\t') { /* expand TABs */
					printk(tab_seq + (col & 07));
					col += 8 - (col & 07);
				} else {
					char buf[2];

					/*
					 * Echo input using puts() to force an
					 * LCD flush if we are using an LCD
					 */
					++col;
					buf[0] = c;
					buf[1] = '\0';
					printk(buf);
				}
				*p++ = c;
				++n;
			} else { /* Buffer full */
				uart_putchar('\a');
			}
		}
	}
}

int cli_readline(const char *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	return cli_readline_into_buffer(prompt, console_buffer, 0);
}

int cli_simple_parse_line(char *line, char *argv[])
{
	int nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	while (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		while (isblank(*line))
			++line;

		if (*line == '\0') { /* end of line, no more args */
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			return nargs;
		}

		argv[nargs++] = line; /* begin of argument string */

		/* find end of string */
		while (*line && !isblank(*line))
			++line;

		if (*line == '\0') { /* end of line, no more args */
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			return nargs;
		}

		*line++ = '\0'; /* terminate current arg */
	}

	printk("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	return nargs;
}

/*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
int cli_simple_run_command(const char *cmd)
{
	char cmdbuf[CONFIG_SYS_CBSIZE]; /* working copy of cmd */
	char *token; /* start of token in cmdbuf */
	char *sep; /* end of token (separator) in cmdbuf */
	char *str = cmdbuf;
	char *argv[CONFIG_SYS_MAXARGS + 1]; /* NULL terminated */
	int argc, inquotes;
	int rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
#ifdef DEBUG
	/* use puts - string may be loooong */
	uart_puts(cmd ? cmd : "NULL");
	printk("\"\n");
#endif

	if (!cmd || !*cmd)
		return -1; /* empty command */

	if (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		printk("## Command too long!\n");
		return -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid commands */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	while (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (inquotes = 0, sep = str; *sep; sep++) {
			if ((*sep == '\'') && (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			if (!inquotes &&
			    (*sep == ';') && /* separator */
			    (sep != str) && /* past string start */
			    (*(sep - 1) != '\\')) /* and NOT escaped */
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		if (*sep) {
			str = sep + 1; /* start of command for next pass */
			*sep = '\0';
		} else {
			str = sep; /* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* Extract arguments */
		argc = cli_simple_parse_line(token, argv);
		if (argc == 0) {
			rc = -1; /* no command at all */
			continue;
		}

		if (cmd_process(argc, argv))
			rc = -1;
	}

	return rc;
}

void cli_simple_loop(void)
{
	static char lastcommand[CONFIG_SYS_CBSIZE + 1] = {
		0,
	};
	int len;

	debug_parser("Entering CLI now...\n");
	cmd_init();

	for (;;) {
		len = cli_readline(CONFIG_SYS_PROMPT);

		if (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);

		if (len == -1)
			printk("<INTERRUPT>\n");
		else if (len > 0)
			cli_simple_run_command(lastcommand);
	}
}
