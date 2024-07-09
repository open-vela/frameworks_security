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

#ifndef TA_ALIPAY_H
#define TA_ALIPAY_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */

#define TA_ALIPAY_UUID                                     \
    {                                                      \
        0x470649f1, 0x8c57, 0x5b56,                        \
        {                                                  \
            0xa2, 0xc3, 0x21, 0xfd, 0x51, 0xdc, 0x67, 0x91 \
        }                                                  \
    }

/* The function IDs implemented in this TA */
#define TA_ALIPAY_CMD_CHK 0
#define TA_ALIPAY_CMD_DEL 1
#define TA_ALIPAY_CMD_WR 2
#define TA_ALIPAY_CMD_RD 3

#endif /*TA_ALIPAY_H*/