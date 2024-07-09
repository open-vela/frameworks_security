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

#ifndef _ALIPAY_CA_API_H_
#define _ALIPAY_CA_API_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t alipay_tee_data_read(const char* item_name, uint8_t* buff,
    uint32_t* out_len);
uint32_t alipay_tee_data_write(const char* item_name, const uint8_t* buf,
    uint32_t len);
uint32_t alipay_tee_data_delete(const char* item_name);
bool is_alipay_tee_data_exited(const char* item_name);

#ifdef __cplusplus
}
#endif

#endif