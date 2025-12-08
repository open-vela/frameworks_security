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

#define CRYPT_AES_128 1
#define CRYPT_AES_256 2
#define CRYPT_AES_128_GCM 3

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief store the did to the secure storage
 *
 * @param[in] did the did to store to the secure storage
 * @param[in] len the length of the did to store, the length must be 8
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
int triad_store_did(uint8_t* did, uint16_t len);

/**
 * @brief load the did from secure storage
 *
 * @param[out] did buffer to store the did that fetched from secure storage
 * @param[in]  len the length of the did to load, the length must be 8
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
int triad_load_did(uint8_t* did, uint16_t len);

/**
 * @brief store the key to the secure storage
 *
 * @param[in] key the key to store to the secure storage
 * @param[in] len the length of the key to store, the length must be 16
 *
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
int triad_store_key(uint8_t* key, uint16_t len);

/**
 * @brief load the key from secure storage
 *
 * @param[out] key the buffer to store the key that fetched from secure
 *                 storage
 * @param[in]  len the length of the key to load, the length must be 16
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
int triad_load_key(uint8_t* key, uint16_t len);

/**
 * @brief calculate the hmac of the given "input" data, the key of the hmac
 *        is the key that we stored with triad_store_key() previously
 *
 * @param[in]  input  the data that need to calculate the hmac
 * @param[in]  inlen  the length of the data that need to calculate the hmac
 * @param[out] output the buffer using to store the hmac content that calculated
 * @param[in]  outlen the length of the result hmac, the length is fixed at 32
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
int triad_get_hmac(uint8_t* input, uint16_t inlen,
    uint8_t* output, uint16_t outlen);

/**
 * @brief Encrypt data using triad key in GCM mode
 *
 * @param[in]  iv       Initialization Vector (IV)
 * @param[in]  iv_len   Length of the initialization vector
 * @param[in]  aad      Additional Authenticated Data (AAD)
 * @param[in]  aad_len  Length of the additional authenticated data
 * @param[in]  input    Plaintext data to be encrypted
 * @param[in]  length   Length of the data to be encrypted
 * @param[out] tag      Output buffer for the authentication tag
 * @param[in]  tag_len  Length of the authentication tag
 * @param[out] output   Output buffer for ciphertext (should be same size as input)
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure
 */
int triad_gcm_encrypt(const unsigned char* iv, size_t iv_len,
    const unsigned char* aad, size_t aad_len,
    const unsigned char* input, size_t length,
    unsigned char* tag, size_t tag_len,
    unsigned char* output);

/**
 * @brief Decrypt and authenticate data encrypted with triad key in GCM mode
 *
 * @param[in]  iv       Initialization Vector (IV)
 * @param[in]  iv_len   Length of the initialization vector
 * @param[in]  aad      Additional Authenticated Data (AAD)
 * @param[in]  aad_len  Length of the additional authenticated data
 * @param[in]  tag      Authentication tag to verify
 * @param[in]  tag_len  Length of the authentication tag
 * @param[in]  input    Ciphertext data to be decrypted
 * @param[in]  length   Length of the data to be decrypted
 * @param[out] output   Output buffer for plaintext (should be same size as input)
 * @return TEEC_SUCCESS on success, TEEC_ERROR_* value on failure (including authentication failure)
 */
int triad_gcm_decrypt(const unsigned char* iv, size_t iv_len,
    const unsigned char* aad, size_t aad_len,
    const unsigned char* tag, size_t tag_len,
    const unsigned char* input, size_t length,
    unsigned char* output);

#ifdef __cplusplus
}
#endif

#endif