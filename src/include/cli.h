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

#ifndef __INCLUDE_CLI_H_
#define __INCLUDE_CLI_H_

#include <sys/types.h>
#include <stdint.h>
#include "printf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#define CONFIG_SYS_CBSIZE	64
#define CONFIG_SYS_MAXARGS	8
#define CONFIG_SYS_PROMPT	"=> "
/* clang-format on */

void cli_simple_loop(void);
void cmd_init(void);
int cmd_process(int argc, char *const argv[]);
uint16_t crc_16(const unsigned char *input_str, size_t num_bytes);
ulong load_serial_bin(ulong offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_CLI_H_ */
