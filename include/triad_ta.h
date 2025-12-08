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

#ifndef TRIAD_TA_H_
#define TRIAD_TA_H_

#include <stdint.h>

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */

#define TA_TRIAD_UUID                                      \
    {                                                      \
        0xc955641c, 0xdd67, 0x5125,                        \
        {                                                  \
            0x94, 0xbe, 0x7f, 0x9d, 0xcd, 0x55, 0x75, 0x95 \
        }                                                  \
    }

/* The function IDs implemented in this TA */
#define TA_TRIAD_CMD_STORE_DID 0
#define TA_TRIAD_CMD_LOAD_DID 1
#define TA_TRIAD_CMD_STORE_KEY 2
#define TA_TRIAD_CMD_LOAD_KEY 3
#define TA_TRIAD_CMD_GET_HMAC 4
#define TA_TRIAD_CMD_GCM_ENCRYPT 5
#define TA_TRIAD_CMD_GCM_DECRYPT 6

#define CRYPT_AES_128 1
#define CRYPT_AES_256 2
#define CRYPT_AES_128_GCM 3
#define CRYPT_AES_256_GCM 4

#define IV_SIZE 16
#define TAG_SIZE 16
#define MAX_BUF_SIZE 512

#define TRIAD_KEY_SIZE 16

struct tk_auth_hdr {
    uint8_t iv_len;
    uint8_t tag_len;
    uint16_t aad_len;
    uint16_t data_len;
};

#endif