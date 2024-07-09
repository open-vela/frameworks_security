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

#ifndef _TRIAD_CA_API_H_
#define _TRIAD_CA_API_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int triad_store_did(uint8_t* did, uint16_t len);
int triad_load_did(uint8_t* did, uint16_t len);
int triad_store_key(uint8_t* key, uint16_t len);
int triad_load_key(uint8_t* key, uint16_t len);
int triad_get_hmac(uint8_t* input, uint16_t inlen,
    uint8_t* output, uint16_t outlen);

#ifdef __cplusplus
}
#endif

#endif