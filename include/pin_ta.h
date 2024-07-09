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

#ifndef TA_PIN_H
#define TA_PIN_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */

#define TA_PIN_UUID                                        \
    {                                                      \
        0x821857ee, 0x62be, 0x5513,                        \
        {                                                  \
            0xad, 0xab, 0x1b, 0x7f, 0x23, 0xde, 0xfa, 0xa6 \
        }                                                  \
    }

/* The function IDs implemented in this TA */
#define TA_PIN_CMD_STORE 0
#define TA_PIN_CMD_VERIFY 1
#define TA_PIN_CMD_CHANGE 2
#define TA_PIN_CMD_GETSHA256 3
#define TA_PIN_CMD_CHK 4
#define TA_PIN_CMD_DEL 5

#endif /*TA_PIN_H*/