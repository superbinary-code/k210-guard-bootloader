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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "aes.h"
#include "cli.h"
#include "clint.h"
#include "common.h"
#include "encoding.h"
#include "flash.h"
#include "fpioa.h"
#include "otp.h"
#include "printf.h"
#include "sha256.h"
#include "sleep.h"
#include "syscalls.h"
#include "sysctl.h"
#include "uarths.h"

#ifndef BUILD_VERSION
#define BUILD_VERSION "unstable dev"
#endif

/*
 * FLASH MAP (3712KB)
 *
 * Alive		320K	0x0035,0000 - 0x0039,FFFF	
 * Feature		1024K	0x0025,0000 - 0x0034,FFFF
 * Detect       320K    0x0020,0000 - 0x0024,FFFF
 * Keypoint     192K    0x001D,0000 - 0x001F,FFFF
 * algorithm    1024K	0x000D,0000 - 0x001C,FFFF
 * BAK          320K   	0x0008,0000 - 0x000C,FFFF
 * APP          320K   	0x0003,0000 - 0x0007,FFFF
 * Stage2_BAK   64K     0x0002,0000 - 0x0002,FFFF
 * Stage2_APP   64K     0x0001,0000 - 0x0001,FFFF
 * Stage1       64K     0x0000,0000 - 0x0000,FFFF
 */
#ifdef LOADER_STAGE1
#define FLASH_APP_ADDR (64 * 1024)
#define FLASH_BAK_ADDR (2 * 64 * 1024)
#define FLASH_NEXT_SIZE (64 * 1024) /* Loader Stage2 APP & BAK, both 64K */
#else
#define FLASH_APP_ADDR (3 * 64 * 1024)
#define FLASH_BAK_ADDR ((3 * 64 + 320) * 1024)
#define FLASH_NEXT_SIZE (320 * 1024) /* APP & BAK, both 320KB */
#endif

#define FLASH_SHA256_LEN 32

#ifdef DEBUG
#warning "THIS IS A DEBUG BUILD, DO NOT USE IT IN PRODUCTION!!!"
#endif

/* Core 0 and Core 1 will call go_boot() almost the same time */
static void go_boot(void)
{
	static int jump_flag = 0;

	if (current_coreid() == 0) {
		debug_parser("[DEBUG] Sending IPI.\n");
		clint_ipi_send(1);
		while (jump_flag == 0)
			;
	} else {
		jump_flag = 1;
		msleep(10);
	}
	printk("Core %lX is jumping to 0x%lX now...\n", current_coreid(),
	       (intptr_t)_boot);

	/* Waiting for print ready */
	msleep(100);

	/* Disable Interrupt */
	asm volatile("csrw mideleg, 0");
	asm volatile("csrw medeleg, 0");
	asm volatile("csrw mie, 0");
	asm volatile("csrw mip, 0");
	asm volatile("csrs mstatus, 0");
	asm volatile("csrw sie, 0");
	asm volatile("csrw sip, 0");
	asm volatile("csrs sstatus, 0");
	/* Clear I-Cache */
	asm volatile("fence.i");

	_boot();
}

static int flash_image_check(uint32_t flash_addr, uintptr_t *ramptr,
			     uint32_t length)
{
	uint8_t firmware_aes_enabled = 0;
	uint32_t codes_length;
	uint8_t sha256_sign[FLASH_SHA256_LEN];
	uint8_t sha256_sign_firmware[FLASH_SHA256_LEN];

	// 1 for internal (SPI3) 0 for external (SPI0)
	flash_init(1);
	flash_enable_quad_mode();

	/* 2. Copy user data to SRAM */
	// 1 byte AES flag
	flash_read_data(flash_addr, &firmware_aes_enabled, 1,
			FLASH_QUAD_SINGLE);
	// 4 bytes length
	flash_read_data(flash_addr + 1, (uint8_t *)(uintptr_t)&codes_length, 4,
			FLASH_QUAD_SINGLE);

	debug_parser("[DEBUG] Code length: 0x%08X = %u\n", codes_length,
		     codes_length);

	if (codes_length > length - 1 - 4 - FLASH_SHA256_LEN) {
		debug_parser(
			"[DEBUG] Code length 0x%08X is larger than 0x%08X\n",
			codes_length, length);
		return -(EXIT_REASON_OVERSIZE);
	}

	// codes_length for app
	flash_read_data(flash_addr + 5, (uint8_t *)ramptr, codes_length,
			FLASH_QUAD_SINGLE);
	// 32 bytes sha256
	flash_read_data(flash_addr + 5 + codes_length,
			(uint8_t *)sha256_sign_firmware, FLASH_SHA256_LEN,
			FLASH_QUAD_SINGLE);

	/* 3. Check user data's SHA256 hash */
	SHA256Context sha256_context;

	debug_parser(
		"[DEBUG] start calculate SHA256, sha256_context addr: %p, data_len: %d\n",
		&sha256_context, (codes_length) + 5);

	sha256_init(DISABLE_SHA_DMA, DISABLE_DOUBLE_SHA, (codes_length) + 5,
		    &sha256_context);
	sha256_update(&sha256_context, &firmware_aes_enabled, 1);
	sha256_update(&sha256_context, &codes_length, 4);
	sha256_update(&sha256_context, (uint8_t *)ramptr, codes_length);
	sha256_final(&sha256_context, sha256_sign);

	// check if SHA256 checksum matches
	for (int i = 0; i < FLASH_SHA256_LEN; i++) {
		if (sha256_sign_firmware[i] != sha256_sign[i]) {
			debug_parser("[DEBUG] SHA256 hash does not match\n");
			debug_parser("[DEBUG] SHA256(firmware): ");
			for (int i = 0; i < FLASH_SHA256_LEN; i++)
				debug_parser("%02x", sha256_sign_firmware[i]);

			debug_parser("\n[DEBUG] SHA256(calculate): ");

			for (int i = 0; i < FLASH_SHA256_LEN; i++)
				debug_parser("%02x", sha256_sign[i]);
			debug_parser("\n");

			/* Exit may due to sha256 fail. */
			return -(EXIT_REASON_SHA256FLASH);
		}
	}

	debug_parser("[DEBUG] SHA256 hash check pass.\n");

	/* 4. Decipher firmware */
	if ((firmware_aes_enabled & 1U) == 1U) {
		// NOTE: Firmware must aligned with 16bytes, and padding 0 at tail

		// enable OTP key
		otp_key_output_enable();

		uint8_t aes_key[16] = { 0 };
		uint8_t aes_iv[16] = { 0 };
		uint64_t aes_output[2];
		uint64_t *aes_input_ptr = ramptr;

		debug_parser("[DEBUG] AES deciphering\n");
		debug_parser("[DEBUG] AES iv\n");
		for (int i = 0; i < 16; i++)
			debug_parser("%02x", aes_iv[i]);
		debug_parser("\n");

		sysctl_clock_enable(SYSCTL_CLOCK_AES);
		sysctl_reset(SYSCTL_RESET_AES);

		aes_init(aes_key, 16, aes_iv, 16, NULL, AES_CBC, AES_DENCRPTION,
			 0, codes_length);

		while (codes_length > 0) {
			// decrypt
			aes_process((uint8_t *)aes_input_ptr,
				    (uint8_t *)aes_output, 16, AES_CBC);

			*(aes_input_ptr++) = aes_output[0];
			*(aes_input_ptr++) = aes_output[1];

			codes_length -= 16;
		}

		otp_key_output_disable(); // disable OTP aeskey output
	} else {
		debug_parser("[DEBUG] Firmware cipher DISABLED.\n");
	}

	return 0;
}

static int flash_image_compare(uint32_t flash_addr1, uint32_t flash_addr2)
{
	uint32_t codes_length;
	uint8_t sha256_sign1[FLASH_SHA256_LEN], sha256_sign2[FLASH_SHA256_LEN];

	// 4 bytes length
	flash_read_data(flash_addr1 + 1, (uint8_t *)(uintptr_t)&codes_length, 4,
			FLASH_QUAD_SINGLE);
	// 32 bytes sha256
	flash_read_data(flash_addr1 + 5 + codes_length, (uint8_t *)sha256_sign1,
			FLASH_SHA256_LEN, FLASH_QUAD_SINGLE);

	// 4 bytes length
	flash_read_data(flash_addr2 + 1, (uint8_t *)(uintptr_t)&codes_length, 4,
			FLASH_QUAD_SINGLE);
	// 32 bytes sha256
	flash_read_data(flash_addr2 + 5 + codes_length, (uint8_t *)sha256_sign2,
			FLASH_SHA256_LEN, FLASH_QUAD_SINGLE);

	return memcmp(sha256_sign1, sha256_sign2, FLASH_SHA256_LEN);
}

static void flash_image_backup(uint32_t from_addr, uint32_t to_addr,
			       uint8_t *ramptr)
{
	uint8_t firmware_aes_enabled = 0;
	uint32_t codes_length;

	// 1 byte AES flag
	flash_read_data(from_addr, &firmware_aes_enabled, 1, FLASH_QUAD_SINGLE);
	// 4 bytes length
	flash_read_data(from_addr + 1, (uint8_t *)(uintptr_t)&codes_length, 4,
			FLASH_QUAD_SINGLE);
	// codes_length for app
	flash_read_data(from_addr + 1 + 4, ramptr,
			codes_length + FLASH_SHA256_LEN, FLASH_QUAD_SINGLE);

	do_flash_erase(to_addr, 1 + 4 + codes_length + FLASH_SHA256_LEN);

	flash_write_data(to_addr, (uint8_t *)(uintptr_t)&firmware_aes_enabled,
			 1);
	flash_write_data(to_addr + 1, (uint8_t *)(uintptr_t)&codes_length, 4);

	do_flash_write(to_addr + 1 + 4, codes_length + FLASH_SHA256_LEN,
		       ramptr);
}

int core1_entry(void *ctx)
{
	clint_ipi_init();
	clint_ipi_clear(current_coreid());
	clint_ipi_enable();
	debug_parser("[DEBUG] Before wfi\n");
	asm volatile("wfi");
	debug_parser("[DEBUG] After wfi\n");
	clint_ipi_disable();
	clint_ipi_clear(current_coreid());
	clint_ipi_unregister();

	go_boot();

	return 0;
}

int main()
{
#ifdef LOADER_STAGE1
	uint32_t stage = 1;
#else
	uint32_t stage = 2;
#endif

	printk("\nMAIX Bootloader stage %d running with core: %ld\n", stage,
	       current_coreid());
	printk("Build: " BUILD_VERSION "\n");
	debug_parser("Next boot RAM address = 0x%08lX\n", (intptr_t)_boot);

	register_core1(core1_entry, NULL);
	msleep(20);

#if (defined DEBUG && defined LOADER_STAGE1)
	/* Show command line interface */
	debug_parser("Please press 'k' or 'K' to enter CLI in 3 seconds");

	/*for (int i = 0; i < 30; i++) {
		msleep(100);
		if (i % 10 == 0)
			debug_parser(".");
		if (!uart_tstc())
			continue;
		int c = uart_getc();
		if (c == 'k' || c == 'K') {
			debug_parser("\n");
			cli_simple_loop();
		}
	}*/
	debug_parser("\n");
#endif

	/* TO run later, we must check APP last */
	int bak_check = flash_image_check(FLASH_BAK_ADDR, (uint64_t *)_boot,
					  FLASH_NEXT_SIZE);
	int app_check = flash_image_check(FLASH_APP_ADDR, (uint64_t *)_boot,
					  FLASH_NEXT_SIZE);

	if (flash_image_compare(FLASH_APP_ADDR, FLASH_BAK_ADDR) != 0) {
		printk("WARNING: Different image found!\n");
		if (app_check == 0) {
			printk("## Copy from app 0x%08X to bak 0x%08X:\n",
			       FLASH_APP_ADDR, FLASH_BAK_ADDR);
			flash_image_backup(FLASH_APP_ADDR, FLASH_BAK_ADDR,
					   (uint8_t *)_boot);
		} else if (bak_check == 0) {
			printk("## Copy from bak 0x%08X to app 0x%08X:\n",
			       FLASH_BAK_ADDR, FLASH_APP_ADDR);
			flash_image_backup(FLASH_BAK_ADDR, FLASH_APP_ADDR,
					   (uint8_t *)_boot);
		} else {
			printk("\nFailed to boot: Image check failed!\n");
			goto FAILED;
		}
	} else if (app_check != 0) {
		printk("\nFailed to boot: image check failed!\n");
		goto FAILED;
	}

	go_boot();

FAILED:
	/* Exit due to user code is end. This function can't return */
	printk("ERROR: Boot failed?! System exit...\n");
	sys_exit(EXIT_REASON_ABNORMAL);
}
