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
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cli.h"
#include "common.h"
#include "ctype.h"
#include "flash.h"
#include "printf.h"
#include "sleep.h"
#include "spi.h"
#include "sysctl.h"
#include "uarths.h"

#define CONFIG_SYS_HELP_CMD_WIDTH 8
#define CONFIG_SYS_CMD_COUNT 16

/*
 * Error codes that commands return to cmd_process(). We use the standard 0
 * and 1 for success and failure, but add one more case - failure with a
 * request to call cmd_usage(). But the cmd_process() function handles
 * CMD_RET_USAGE itself and after calling cmd_usage() it will return 1.
 * This is just a convenience for commands to avoid them having to call
 * cmd_usage() all over the place.
 */
enum command_ret_t {
	CMD_RET_SUCCESS, /* 0 = Success */
	CMD_RET_FAILURE, /* 1 = Failure */
	CMD_RET_USAGE = -1, /* Failure, please report 'usage' error */
};

/*
 * Monitor Command Table
 */
struct cmd_tbl_s {
	char *name; /* Command Name                 */
	int maxargs; /* maximum number of arguments  */
	/* Implementation function      */
	int (*cmd)(struct cmd_tbl_s *, int, char *const[]);
	char *usage; /* Usage message        (short) */
};

static struct cmd_tbl_s *cmd_array[CONFIG_SYS_CMD_COUNT];
static int cmd_items = 0;

/* find command table entry for a command */
struct cmd_tbl_s *find_cmd_tbl(const char *cmd, struct cmd_tbl_s *table[],
			       int cmd_items)
{
	struct cmd_tbl_s *cmdtp_temp = table[0]; /* Init value */
	const char *p;
	int i, len;
	int n_found = 0;

	if (!cmd)
		return NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen(cmd) : (p - cmd);

	for (i = 0; i < cmd_items; i++) {
		if (strncmp(cmd, table[i]->name, len) == 0) {
			if (len == strlen(table[i]->name))
				return table[i]; /* full match */

			cmdtp_temp = table[i]; /* abbreviated command ? */
			n_found++;
		}
	}
	if (n_found == 1) { /* exactly one match */
		return cmdtp_temp;
	}

	return NULL; /* not found or ambiguous command */
}

int cmd_usage(const struct cmd_tbl_s *cmdtp)
{
	printk("%s - %s\n\n", cmdtp->name, cmdtp->usage);
	return 1;
}

int do_help(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) { /* show list of commands */
		int i, j, swaps;

		/* Sort command list (trivial bubble sort) */
		for (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			for (j = 0; j < i; ++j) {
				if (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					struct cmd_tbl_s *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			if (!swaps)
				break;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			if (usage == NULL)
				continue;
			printk("%-*s- %s\n", CONFIG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		return 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_array, cmd_items);
		if (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} else {
			printk("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	return rcode;
}

/**
 * print_buffer() - Print data buffer in hex and ascii form
 *
 * Data reads are buffered so that each memory address is only read once.
 * This is useful when displaying the contents of volatile registers.
 *
 * @addr:       Starting address to display at start of line
 * @data:       pointer to data buffer
 * @width:      data value width.  May be 1, 2, or 4.
 * @count:      number of values to display
 * @linelen:    Number of values to print per line; specify 0 for default length
 */
#define MAX_LINE_LENGTH_BYTES (64)
#define DEFAULT_LINE_LENGTH_BYTES (16)
static int print_buffer(uint32_t addr, const void *data, uint32_t width,
			uint32_t count, uint32_t linelen)
{
	/* linebuf as a union causes proper alignment */
	union linebuf {
		uint32_t ui[MAX_LINE_LENGTH_BYTES / sizeof(uint32_t) + 1];
		uint16_t us[MAX_LINE_LENGTH_BYTES / sizeof(uint16_t) + 1];
		uint8_t uc[MAX_LINE_LENGTH_BYTES / sizeof(uint8_t) + 1];
	} lb;
	int i;
	uint32_t x;

	if (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	if (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	while (count) {
		uint thislinelen = linelen;
		printk("%08X:", addr);

		/* check for overflow condition */
		if (count < thislinelen)
			thislinelen = count;

		/* Copy from memory into linebuf and print hex values */
		for (i = 0; i < thislinelen; i++) {
			if (width == 4)
				x = lb.ui[i] = *(volatile uint32_t *)data;
			else if (width == 2)
				x = lb.us[i] = *(volatile uint16_t *)data;
			else
				x = lb.uc[i] = *(volatile uint8_t *)data;
			printk(" %0*x", width * 2, x);
			data += width;
		}

		while (thislinelen < linelen) {
			/* fill line with whitespace for nice ASCII print */
			for (i = 0; i < width * 2 + 1; i++)
				uart_puts(" ");
			linelen--;
		}

		/* Print data in ASCII characters */
		for (i = 0; i < thislinelen * width; i++) {
			if (!isprint(lb.uc[i]) || lb.uc[i] >= 0x80)
				lb.uc[i] = '.';
		}
		lb.uc[i] = '\0';
		printk("    %s\n", lb.uc);

		/* update references */
		addr += thislinelen * width;
		count -= thislinelen;

		if (uart_ctrlc())
			return -1;
	}

	return 0;
}

int do_fldump(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
#define FLASH_DUMP_BUF_SIZE 16
	uint32_t addr, offset = simple_strtoul(argv[1], NULL, 16);
	uint8_t buf[FLASH_DUMP_BUF_SIZE];
	int i, j, length = (int)simple_strtoul(argv[2], NULL, 16);

	flash_init(1);
	flash_enable_quad_mode();
	debug_parser("[DEBUG] offset = 0x%08X, length = 0x%08X.\n", offset,
		     length);

	for (i = 0; length > 0; length -= FLASH_DUMP_BUF_SIZE, i++) {
		addr = offset + i * FLASH_DUMP_BUF_SIZE;
		j = length > FLASH_DUMP_BUF_SIZE ? FLASH_DUMP_BUF_SIZE : length;
		flash_read_data(addr, buf, j, FLASH_QUAD_SINGLE);
		print_buffer(addr, buf, 1, j, 16);
	}

	printk("\n");
	return 0;
}

/* flread <floffset> <length> <ramaddr> */
int do_flread(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	uint32_t offset = simple_strtoul(argv[1], NULL, 16);
	uint32_t length = simple_strtoul(argv[2], NULL, 16);
	intptr_t ramaddr = simple_strtoul(argv[3], NULL, 16);

	debug_parser(
		"[DEBUG] floffset: 0x%08X, length: 0x%08X, ramaddr: 0x%08lX\n",
		offset, length, ramaddr);

	flash_init(1);
	flash_enable_quad_mode();
	msleep(100);

	flash_read_data(offset, (uint8_t *)ramaddr, length, FLASH_QUAD_SINGLE);

	return 0;
}

/* TODO: Cauculate length for 4k/32K/64K and whole chip, check overflow with fast erase */
int do_flash_erase(uint32_t offset, uint32_t length)
{
#define FLASH_SECTOR_SIZE (4 * 1024)
	int i, sectors;

	if (offset % FLASH_SECTOR_SIZE != 0) {
		printk("\n## ERROR: floffset must be aligned with 0x1000!\n\n");
		return -1;
	} else if (length == 0) {
		printk("\n## ERROR: length must be larger than 0!\n\n");
		return -1;
	}
	debug_parser("[DEBUG] floffset: 0x%08X, length: 0x%08X\n", offset,
		     length);

	flash_init(1);
	flash_enable_quad_mode();
	sectors = length / FLASH_SECTOR_SIZE +
		  ((length % FLASH_SECTOR_SIZE) == 0 ? 0 : 1);

	flash_disable_protect();

	printk("## Erasing flash from 0x%08X to 0x%08X:\n", offset,
	       offset + sectors * FLASH_SECTOR_SIZE - 1);
	for (i = 0; i < sectors; i++) {
		printk(".");
		debug_parser("Erase from: 0x%08X, size = 0x1000\n",
			     FLASH_SECTOR_SIZE * i + offset);
		if (i != 0 && i % 64 == 0)
			printk("\n");
		flash_sector_erase(FLASH_SECTOR_SIZE * i + offset);
		if (uart_ctrlc()) {
			printk("## Interrupted!\n");
			return -1;
		}
	}
	printk("\n");
#ifdef DEBUG
	debug_parser("Checking flash content:\n");
	uint8_t buf[32];
	flash_read_data(offset, buf, 32, FLASH_QUAD_SINGLE);
	print_buffer(offset, buf, 1, 16, 16);
	print_buffer(offset + 16, buf + 16, 1, 16, 16);
	debug_parser("\n");
#endif

	return 0;
}

/* flerase <floffset> <length> */
int do_flerase(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	return do_flash_erase(simple_strtoul(argv[1], NULL, 16),
			      simple_strtoul(argv[2], NULL, 16));
}

int do_flash_write(uint32_t offset, uint32_t length, uint8_t *ramptr)
{
#define FLASH_WRITE_SIZE 128
	uint32_t lenwrite;
	int i = 0;

	printk("## Writing data into flash from 0x%08X to 0x%08X:\n", offset,
	       offset + length - 1);
	flash_init(1);
	flash_enable_quad_mode();

	while (length > 0) {
		lenwrite =
			(length > FLASH_WRITE_SIZE) ? FLASH_WRITE_SIZE : length;
		flash_write_data(offset, ramptr, lenwrite);
		offset += lenwrite;
		ramptr += lenwrite;
		length -= lenwrite;
		i++;
		printk(".");
		if (i % 64 == 0)
			printk("\n");
	}
	printk("\n");

	return 0;
}

/* flwrite <ramaddr> <length> <floffset> */
int do_flwrite(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	uint32_t offset = simple_strtoul(argv[1], NULL, 16);
	uint32_t length = simple_strtoul(argv[2], NULL, 16);
	uintptr_t ramaddr = simple_strtoul(argv[3], NULL, 16);

	debug_parser(
		"[DEBUG] ramaddr: 0x%08lX, length: 0x%08X, floffset: 0x%08X\n",
		ramaddr, length, offset);

	if (do_flash_erase(offset, length) != 0) {
		return -1;
	}

	return do_flash_write(offset, length, (uint8_t *)ramaddr);
}

/* crc16 <ramaddr> <length> */
int do_crc16(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	unsigned char *data_p =
		(unsigned char *)simple_strtoul(argv[1], NULL, 16);
	uint32_t length = simple_strtoul(argv[2], NULL, 16);

	printk("## Caculated CRC16: 0x%04X\n\n", crc_16(data_p, length));
	return 0;
}

/* loadb <ramaddr> */
int do_loadb(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	uintptr_t ramaddr = simple_strtoul(argv[1], NULL, 16);
	int size, rcode = 0;

	printk("## Ready for binary (kermit) download to 0x%08lX...\n",
	       ramaddr);

	size = load_serial_bin(ramaddr);

	if (size == ~0) {
		printk("## Binary (kermit) download aborted\n");
		rcode = 1;
	} else {
		printk("## Start Addr      = 0x%08lX\n", ramaddr);
	}

	printk("## Total Size      = 0x%08X = %d Bytes\n", size, size);
	printk("## Caculated CRC16 = 0x%04X\n",
	       crc_16((uint8_t *)ramaddr, size));
	return rcode;
}

/* md <ramaddr> <length> */
int do_md(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	unsigned char *data_p =
		(unsigned char *)simple_strtoul(argv[1], NULL, 16);
	uint32_t length = simple_strtoul(argv[2], NULL, 16);

	print_buffer(simple_strtoul(argv[1], NULL, 16), data_p, 1, length, 16);

	return 0;
}

int do_reset(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	sysctl_reset(SYSCTL_RESET_SOC);
	return 0;
}

struct cmd_tbl_s cmd_tbl_help = { .name = "help",
				  .maxargs = 2,
				  .cmd = &do_help,
				  .usage = "help <command>" };
struct cmd_tbl_s cmd_tbl_fldump = { .name = "fldump",
				    .maxargs = 3,
				    .cmd = &do_fldump,
				    .usage = "fldump <floffset> <length>" };
struct cmd_tbl_s cmd_tbl_flread = {
	.name = "flread",
	.maxargs = 4,
	.cmd = &do_flread,
	.usage = "flread <floffset> <length> <ramaddr>"
};
struct cmd_tbl_s cmd_tbl_flerase = { .name = "flerase",
				     .maxargs = 3,
				     .cmd = &do_flerase,
				     .usage = "flerase <floffset> <length>" };
struct cmd_tbl_s cmd_tbl_flwrite = {
	.name = "flwrite",
	.maxargs = 4,
	.cmd = &do_flwrite,
	.usage = "flwrite <floffset> <length> <ramaddr>"
};
struct cmd_tbl_s cmd_tbl_crc16 = { .name = "crc16",
				   .maxargs = 3,
				   .cmd = &do_crc16,
				   .usage = "crc16 <ramaddr> <length>" };
struct cmd_tbl_s cmd_tbl_loadb = { .name = "loadb",
				   .maxargs = 2,
				   .cmd = &do_loadb,
				   .usage = "loadb <ramaddr> (kermit mode)" };
struct cmd_tbl_s cmd_tbl_md = { .name = "md",
				.maxargs = 3,
				.cmd = &do_md,
				.usage = "md <ramaddr> <length>" };
struct cmd_tbl_s cmd_tbl_reset = { .name = "reset",
				   .maxargs = 1,
				   .cmd = &do_reset,
				   .usage = "reset" };

void cmd_init(void)
{
	int i = 0;
	memset(&cmd_array, 0, sizeof(cmd_array));
	cmd_array[i++] = &cmd_tbl_help;
	cmd_array[i++] = &cmd_tbl_fldump;
	cmd_array[i++] = &cmd_tbl_flread;
	cmd_array[i++] = &cmd_tbl_flwrite;
	cmd_array[i++] = &cmd_tbl_flerase;
	cmd_array[i++] = &cmd_tbl_crc16;
	cmd_array[i++] = &cmd_tbl_loadb;
	cmd_array[i++] = &cmd_tbl_md;
	cmd_array[i++] = &cmd_tbl_reset;
	cmd_items = i;
}

struct cmd_tbl_s *find_cmd(const char *cmd)
{
	return find_cmd_tbl(cmd, cmd_array, cmd_items);
}

/**
 * Call a command function. This should be the only route in U-Boot to call
 * a command, so that we can track whether we are waiting for input or
 * executing a command.
 *
 * @param cmdtp		Pointer to the command to execute
 * @param argc		Number of arguments (arg 0 must be the command text)
 * @param argv		Arguments
 * @return 0 if command succeeded, else non-zero (CMD_RET_...)
 */
static int cmd_call(struct cmd_tbl_s *cmdtp, int argc, char *const argv[])
{
	int result;

	result = (cmdtp->cmd)(cmdtp, argc, argv);
	if (result)
		printk("## Command failed, result=%d\n", result);
	return result;
}

int cmd_process(int argc, char *const argv[])
{
	int rc = 0;
	struct cmd_tbl_s *cmdtp;

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	if (cmdtp == NULL) {
		printk("## Unknown command '%s' - try 'help'\n", argv[0]);
		return 1;
	}

	/* found - check max args */
	if (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

	/* If OK so far, then do the command */
	if (!rc) {
		rc = cmd_call(cmdtp, argc, argv);
	}

	if (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);

	return rc;
}
