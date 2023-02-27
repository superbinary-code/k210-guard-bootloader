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

#ifndef __INCLUDE_SHA256_H_
#define __INCLUDE_SHA256_H_

#include <stdint.h>
#include "encoding.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DISABLE_SHA_DMA 0
#define ENABLE_SHA_DMA 1

#define DISABLE_DOUBLE_SHA 0
#define ENABLE_DOUBLE_SHA 1
/**
 * @brief      SHA256
 *
 */
struct sha256_t {
	uint32_t sha_result[8];
	uint32_t sha_data_in1;
	uint32_t sha_data_in2;
	uint32_t sha_data_num; //1 unit represents 64 bytes
	uint32_t sha_status;
	uint32_t double_sha;
	uint32_t sha_input_ctrl;
} __attribute__((packed, aligned(4)));

#define SHA256_HASH_SIZE 32

/* Hash size in 32-bit words */
#define SHA256_HASH_WORDS 8

struct _SHA256Context {
	uint64_t totalLength;
	uint32_t hash[SHA256_HASH_WORDS];
	uint32_t bufferLength;
	union {
		uint32_t words[16];
		uint8_t bytes[64];
	} buffer;
#ifdef RUNTIME_ENDIAN
	int littleEndian;
#endif /* RUNTIME_ENDIAN */
};

typedef struct _SHA256Context SHA256Context;

int sha256_init(uint8_t dma_en, uint8_t double_sha_en, uint32_t input_size,
		SHA256Context *sc);
void sha256_update(SHA256Context *sc, const void *data, uint32_t len);
void sha256_final(SHA256Context *sc, uint8_t hash[SHA256_HASH_SIZE]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_SHA256_H_ */
