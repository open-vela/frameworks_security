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

#ifndef TA_WXCODEPAY_H
#define TA_WXCODEPAY_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */

#define TA_WXCODEPAY_UUID                                  \
    {                                                      \
        0xa89eba4e, 0xf4ac, 0x5127,                        \
        {                                                  \
            0xad, 0x93, 0xa5, 0xa4, 0xf0, 0x3e, 0x1a, 0x67 \
        }                                                  \
    }

/* The function IDs implemented in this TA */
#define TA_WXCODEPAY_CMD_CHK 0
#define TA_WXCODEPAY_CMD_DEL 1
#define TA_WXCODEPAY_CMD_WR 2
#define TA_WXCODEPAY_CMD_RD 3

#endif /*TA_WXCODEPAY_H*/