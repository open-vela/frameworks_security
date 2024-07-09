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

#endif