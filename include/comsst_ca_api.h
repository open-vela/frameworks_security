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

#ifndef _COMSST_CA_API_H_
#define _COMSST_CA_API_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t comsst_data_read(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t* out_len);
uint32_t comsst_data_write(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t len);
uint32_t comsst_data_delete(uint8_t* scope, uint8_t* name, bool is_deletable);
uint32_t is_comsst_data_exited(uint8_t* scope, uint8_t* name,
    bool is_deletable);
uint32_t comsst_data_verify(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif