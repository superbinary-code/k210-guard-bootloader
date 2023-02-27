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

#ifndef __INCLUDE_AES_H_
#define __INCLUDE_AES_H_

#include <stdint.h>
#include "encoding.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      AES
 *

 */
struct aes_t {
	uint32_t aes_key[4];
	uint32_t encrpt_sel;
	/* 0: encrption ; 1: dencrption*/
	uint32_t cipher_mode;
	/*
	 * [1:0],set the first bit and second bit 00:ecb; 01:cbc;
	 * 10,11：aes_gcm
	 */
	uint32_t aes_iv[4];
	uint32_t aes_endian;
	/*aes interrupt enable*/
	uint32_t aes_finish;
	/*aes interrupt flag*/
	uint32_t dma_sel;
	/*gcm add data begin address*/
	uint32_t gb_aad_end_adr;
	/*gcm add data end address*/
	uint32_t gb_pc_ini_adr;
	/*gcm plantext/ciphter text data begin address*/
	uint32_t gb_pc_end_adr;
	/*gcm plantext/ciphter text data end  address*/
	uint32_t aes_text_data;
	/*gcm plantext/ciphter text data*/
	uint32_t aes_aad_data;
	/*AAD data*/
	uint32_t tag_chk;
	/*
	 * [1:0],00:check not finish; 01: check fail; 10: check success;11:
	 * reversed
	 */
	uint32_t data_in_flag;
	/*data can input flag 1: data can input; 0 : data cannot input*/
	uint32_t gcm_in_tag[4];
	/*gcm input tag for compare with the calculate tag*/
	uint32_t aes_out_data;
	/*gcm plantext/ciphter text data*/
	uint32_t gb_aes_en;
	uint32_t data_out_flag;
	/*data can output flag 1: data ready 0: data not ready*/
	uint32_t tag_in_flag;
	/*allow tag input when use GCM*/
	uint32_t tag_clear;
	uint32_t gcm_out_tag[4];
} __attribute__((packed, aligned(4)));

enum aes_cipher_mod {
	AES_ECB = 0,
	AES_CBC = 1,
	AES_GCM = 3,
};

enum aes_encrpt_sel {
	AES_ENCRPTION = 0,
	AES_DENCRPTION = 1,
};

int aes_init(uint8_t *key_addr, uint8_t key_length, uint8_t *aes_iv,
	     uint8_t iv_length, uint8_t *aes_aad,
	     enum aes_cipher_mod cipher_mod, enum aes_encrpt_sel encrpt_sel,
	     uint32_t add_size, uint32_t data_size);
/*write data*/
int aes_write_aad(uint32_t aad_data);
int aes_write_text(uint32_t text_data);
int aes_write_tag(uint32_t *tag);
/*get tag*/
int get_data_in_flag(void);
int get_data_out_flag(void);
int get_tag_in_flag(void);

uint32_t read_out_data(void);
int aes_check_tag(void);
int aes_get_tag(uint8_t *l_tag);
int aes_clear_chk_tag(void);
int aes_process(uint8_t *aes_in_data, uint8_t *aes_out_data, uint32_t data_size,
		enum aes_cipher_mod cipher_mod);
int check_tag(uint32_t *aes_gcm_tag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_AES_H_ */
