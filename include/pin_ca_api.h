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

#ifndef _PIN_CA_API_H_
#define _PIN_CA_API_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t pin_store(bool is_deletable, uint8_t* buff, uint32_t len);
bool pin_is_exist(bool is_deletable);
uint32_t pin_delete(bool is_deletable);
uint32_t pin_verify(bool is_deletable, uint8_t* buff, uint32_t len);
uint32_t pin_change(bool is_deletable, uint8_t* old, uint32_t oldlen,
    uint8_t* new, uint32_t newlen);
uint32_t pin_getsha256(bool is_deletable, uint8_t* buff, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif