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

/**
 * @brief to store the pin to secure storage
 *
 * @param[in] is_deletable whether the pin store to secure storage is
 *                         deletable or not, the secure storage is based
 *                         on TEE, and in TEE the storage space is split
 *                         to two parts, one is for deletable area, one is
 *                         for non-deletable area, if the pin is deletable,
 *                         the pin will be stored to deletable area, otherwise
 *                         the pin will be stored to non-deletable area.
 * @param[in] buff         the buffer contains the pin to store
 * @param[in] len          the length of the pin to store
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t pin_store(bool is_deletable, uint8_t* buff, uint32_t len);

/**
 * @brief to detect wheter the pin exist or not
 *
 * @param[in] is_deletable to indicate the pin to detect is stored on
 *                         deleteable area or non-deletable area
 * @return true on pin exist, false on pin not exist
 */
bool pin_is_exist(bool is_deletable);

/**
 * @brief delete the pin from secure storage
 *
 * @param[in] is_deletable to indicate the pin to delete is stored on
 *                         deleteable area or non-deletable area
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t pin_delete(bool is_deletable);

/**
 * @brief to verify the pin is correct or not
 *
 * @param[in] is_deletable the pin to verify is stored on deleteable area
 *                         or non-deletable area
 * @param[in] buff         the buffer contains the pin to verify
 * @param[in] len          the length of the pin to verify
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t pin_verify(bool is_deletable, uint8_t* buff, uint32_t len);

/**
 * @brief change the old pin to new pin
 *
 * @param[in] is_deletable to indicate the pin to change is stored on
 *                         deleteable area or non-deletable area
 * @param[in] old          the old pin to change
 * @param[in] oldlen       the length of the old pin
 * @param[in] new          the new pin to change
 * @param[in] newlen       the length of the new pin
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t pin_change(bool is_deletable, uint8_t* old, uint32_t oldlen,
    uint8_t* new, uint32_t newlen);

/**
 * @brief get the sha256 of the pin
 *
 * @param[in]  is_deletable to indicate the pin to fetch is stored on
 *                          deleteable area or non-deletable area
 * @param[out] buff         the buffer to store the sha256 of the pin
 * @param[in]  len          the length of the sha256 of the pin
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
uint32_t pin_getsha256(bool is_deletable, uint8_t* buff, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif