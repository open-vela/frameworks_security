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

#ifndef TA_COMSST_H
#define TA_COMSST_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */

#define TA_COMSST_UUID                                     \
    {                                                      \
        0xc9ac17f6, 0xe59a, 0x5940,                        \
        {                                                  \
            0x85, 0xd0, 0x50, 0x56, 0xdf, 0x6b, 0xf2, 0x8b \
        }                                                  \
    }

/* The function IDs implemented in this TA */
#define TA_COMSST_CMD_CHK 0
#define TA_COMSST_CMD_DEL 1
#define TA_COMSST_CMD_WR 2
#define TA_COMSST_CMD_RD 3
#define TA_COMSST_CMD_VR 4

#endif /*TA_COMSST_H*/