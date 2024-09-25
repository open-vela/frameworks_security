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

/**
 * @brief to read the comsst data from secure storage
 *
 * @param[in]  scope        the scope the comsst data to fetch
 * @param[in]  name         the name of comsst data to fetch
 *                          in underlying implementation, the comsst
 *                          name is constructed by scope and name,
 *                          and the max length of "scope + name" is 30
 * @param[in]  is_deletable to indicate the comsst to fetch is stored on
 *                          deleteable area or non-deletable area
 * @param[out] buff         the buffer to contain the comsst data that
 *                          is fetched from secure storage
 * @param[out] out_len      the length of the comsst data that is fetched
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t comsst_data_read(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t* out_len);

/**
 * @brief to write the comsst data to secure storage
 *
 * @param[in]  scope       the scope the comsst data to fetch
 * @param[in]  name        the name of comsst data to fetch
 *                         in underlying implementation, the comsst
 *                         name is constructed by scope and name,
 *                         and the max length of "scope + name" is 30
 * @param[in] is_deletable to indicate the comsst to write is stored on
 *                         deleteable area or non-deletable area
 * @param[in] buff         the buffer contains the comsst data to write
 * @param[in] len          the length of the comsst data to write
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t comsst_data_write(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t len);

/**
 * @brief to delete the comsst data from secure storage
 *
 * @param[in]  scope       the scope the comsst data to fetch
 * @param[in]  name        the name of comsst data to fetch
 *                         in underlying implementation, the comsst
 *                         name is constructed by scope and name,
 *                         and the max length of "scope + name" is 30
 * @param[in] is_deletable to indicate the comsst to delete is stored on
 *                         deleteable area or non-deletable area
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t comsst_data_delete(uint8_t* scope, uint8_t* name, bool is_deletable);

/**
 * @brief to detect whether the comsst data is existed in secure storage or not
 *
 * @param[in]  scope       the scope the comsst data to fetch
 * @param[in]  name        the name of comsst data to fetch
 *                         in underlying implementation, the comsst
 *                         name is constructed by scope and name,
 *                         and the max length of "scope + name" is 30
 * @param[in] is_deletable to indicate the comsst to check is stored on
 *                         deleteable area or non-deletable area
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t is_comsst_data_exited(uint8_t* scope, uint8_t* name,
    bool is_deletable);

/**
 * @brief to verify the content of given buffer is same with the
 *        comsst data stored in secure storage
 *
 * @param[in]  scope       the scope the comsst data to fetch
 * @param[in]  name        the name of comsst data to fetch
 *                         in underlying implementation, the comsst
 *                         name is constructed by scope and name,
 *                         and the max length of "scope + name" is 30
 * @param[in] is_deletable to indicate the comsst to verify is stored on
 *                         deleteable area or non-deletable area
 * @param[in] buff         the buffer contains the data to verify
 * @param[in] len          the length of the data to verify
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t comsst_data_verify(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
