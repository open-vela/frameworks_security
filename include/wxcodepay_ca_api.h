/*
 * Copyright (C) 2022-2024 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _WXCODEPAY_CA_API_H_
#define _WXCODEPAY_CA_API_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t wxcodepay_tee_data_read(int item, uint8_t* buff, uint32_t* out_len);
uint32_t wxcodepay_tee_data_write(int item, const uint8_t* buf, uint32_t len);
uint32_t wxcodepay_tee_data_delete(int item);
bool is_wxcodepay_tee_data_exited(int item);

#ifdef __cplusplus
}
#endif

#endif