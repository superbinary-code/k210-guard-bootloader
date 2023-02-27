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

#include <stdlib.h>
#include "clint.h"
#include "common.h"
#include "entry.h"
#include "fpioa.h"
#include "platform.h"
#include "plic.h"
#include "syscalls.h"
#include "sysctl.h"
#include "syslog.h"
#include "uarths.h"

static volatile uint64_t g_wake_up[2] = {0, 0};

core_instance_t core1_instance;

volatile char *const ram = (volatile char *)RAM_BASE_ADDR;

extern char _heap_start[];
extern char _heap_end[];

void thread_entry(int core_id)
{
	while (!g_wake_up[core_id])
		;
}

void core_enable(int core_id)
{
	g_wake_up[core_id] = 1;
}

void core_disable(int core_id)
{
	g_wake_up[core_id] = 0;
}

int register_core1(core_function func, void *ctx)
{
	if (func == NULL)
		return -1;
	core1_instance.callback = func;
	core1_instance.ctx = ctx;
	core_enable(1);
	return 0;
}

int __attribute__((weak))
os_entry(int core_id, int number_of_cores, int (*user_main)(int, char **))
{
	/* Call main if there is no OS */
	return user_main(0, 0);
}

void _init_bsp(int core_id, int number_of_cores)
{
	extern int main(int argc, char *argv[]);
	extern void __libc_init_array(void);
	extern void __libc_fini_array(void);
	static volatile int env_ready = 0;

	if (core_id == 0) {
		/* Initialize bss data to 0 */
		init_bss();
		/* Init UART */
		uart_init();
		/* Init FPIOA */
		fpioa_init();
#if	0
		/* Flash LED */
#define LED_RED 2
		fpioa_set_function(15, FUNC_GPIOHS2);
		gpiohs_set_drive_mode(LED_RED, GPIO_DM_OUTPUT);
		gpiohs_set_pin(LED_RED, GPIO_PV_HIGH);
		for (int i = 0; i < 2; i++) {
			msleep(200);
			gpiohs_set_pin(LED_RED, GPIO_PV_LOW);
			msleep(200);
			gpiohs_set_pin(LED_RED, GPIO_PV_HIGH);
		}
#endif
		/* Register finalization function */
		atexit(__libc_fini_array);
		/* Init libc array for C++ */
		__libc_init_array();
		env_ready = 1;
	}

	int ret = 0;
	if (core_id == 0) {
		core1_instance.callback = NULL;
		core1_instance.ctx = NULL;
		ret = os_entry(core_id, number_of_cores, main);
	} else {
		while (env_ready == 0)
			;

		thread_entry(core_id);
		if (core1_instance.callback != NULL) {
			ret = core1_instance.callback(core1_instance.ctx);
		}
	}
	sys_exit(ret);
}

int pthread_setcancelstate(int __state, int *__oldstate)
{
	return 0;
}
