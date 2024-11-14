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

#include "hello_world_ta.h"
#include <kernel/user_ta.h>
#include <string.h>
#include <tee_internal_api.h>
#include <trace.h>

#include <stdio.h>
#include <stdlib.h>

TEE_Result test_sha256(void)
{
    uint8_t* msg = (uint8_t*)"hello world";
    uint32_t msg_len = 11;
    uint8_t hash[32];
    size_t hash_len = 32;
    TEE_OperationHandle operation = NULL;
    TEE_Result ret = TEE_SUCCESS;

    ret = TEE_AllocateOperation(&operation, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0);
    if (ret != TEE_SUCCESS) {
        EMSG("TEE_AllocateOperation failed: 0x%08" PRIx32, ret);
        goto hash_fail;
    }

    TEE_DigestUpdate(operation, (void*)msg, msg_len);
    ret = TEE_DigestDoFinal(operation, NULL, 0, hash, &hash_len);
    if (ret != TEE_SUCCESS) {
        EMSG("TEE_DigestDoFinal failed: 0x%08" PRIx32, ret);
        goto hash_fail;
    }

hash_fail:
    if (operation != NULL) {
        TEE_FreeOperation(operation);
    }

    if (!ret) {
        DMSG("msg: %s\n", msg);
    }
    return ret;
}

/* tomcrypt_hash.h
 * tomcrypt_mac.h
 */
extern int find_hash(const char* name);
extern int hmac_memory(int hash,
    const unsigned char* key, unsigned long keylen,
    const unsigned char* in, unsigned long inlen,
    unsigned char* out, unsigned long* outlen);

/* | Object Type		| Possible Key Sizes				|
 * +----------------------------+-----------------------------------------------+
 * | TEE_TYPE_HMAC_SHA256	| Between 192 and 1024 bits, multiple of 8 bits	|
 */
#define MAX_KEY_SIZE 128
#define MIN_KEY_SIZE 24

static TEE_Result hmac_sha256(uint8_t* key, uint32_t keylen,
    uint8_t* in, uint32_t inlen,
    uint8_t* out, unsigned long* outlen)
{
    TEE_Attribute attr = { 0 };
    TEE_ObjectHandle key_handle = TEE_HANDLE_NULL;
    TEE_OperationHandle op_handle = TEE_HANDLE_NULL;
    TEE_Result res = TEE_SUCCESS;
    size_t m_outlen = *outlen;

    if (keylen < MIN_KEY_SIZE || keylen > MAX_KEY_SIZE) {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    res = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA256, keylen * 8, &key_handle);
    if (res != TEE_SUCCESS) {
        EMSG("call TEE_AllocateTransientObject failed with 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, keylen);

    res = TEE_PopulateTransientObject(key_handle, &attr, 1);
    if (res != TEE_SUCCESS) {
        EMSG("call TEE_PopulateTransientObject failed with 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    res = TEE_AllocateOperation(&op_handle, TEE_ALG_HMAC_SHA256, TEE_MODE_MAC, keylen * 8);
    if (res != TEE_SUCCESS) {
        EMSG("call TEE_AllocateOperation failed with 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    res = TEE_SetOperationKey(op_handle, key_handle);
    if (res != TEE_SUCCESS) {
        EMSG("call TEE_SetOperationKey with 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    TEE_MACInit(op_handle, NULL, 0);
    TEE_MACUpdate(op_handle, in, inlen);
    res = TEE_MACComputeFinal(op_handle, NULL, 0, out, &m_outlen);

exit:
    if (op_handle != TEE_HANDLE_NULL) {
        TEE_FreeOperation(op_handle);
    }

    TEE_FreeTransientObject(key_handle);
    if (!res) {
        *outlen = (unsigned long)m_outlen;
    }
    return res;
}

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result Hello_World_TA_CreateEntryPoint(void)
{
    DMSG("has been called\n");
    return TEE_SUCCESS;
}
/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void Hello_World_TA_DestroyEntryPoint(void)
{
    DMSG("has been called\n");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result Hello_World_TA_OpenSessionEntryPoint(uint32_t param_types,
    TEE_Param params[4],
    void** sess_ctx)
{
#ifndef TRACE_PERF
    TEE_ObjectHandle obj;
    uint8_t name1[] = { 't', 'e', 's', 't', '1', '.', 't', 'x', 't', '\0' }; // object id: test1.txt
    uint8_t name2[] = { 't', 'e', 's', 't', '2', '.', 't', 'x', 't', '\0' }; // object id: test2.txt

#endif
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    DMSG("has been called\n");
    if (param_types != exp_param_types) {
        EMSG("-> bad parameter\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

#ifndef TRACE_PERF
    DMSG("from ca(%zu): %s\n", params[0].memref.size, (char*)params[0].memref.buffer);
    TEE_MemMove(params[0].memref.buffer, "hi ca!", 6);
#endif

    /* Unused parameters */
    (void)&params;
    (void)&sess_ctx;

    /*
     * The DMSG() macro is non-standard, TEE Internal API doesn't
     * specify any means to logging from a TA.
     */
    IMSG("Hello World!\n");

#ifndef TRACE_PERF
    test_sha256();

    int index = 0;
    TEE_Result res;
    uint8_t key[32] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xa0,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xa0,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xa0,
        0x01, 0x02 };
    uint32_t keylen = 32;
    uint8_t in[10] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xa0 };
    uint32_t inlen = 10;
    uint8_t out[32] = { 0x00 };
    unsigned long outlen = 32;

    index = find_hash("sha256");
    DMSG("hash index: %d\n", index);

    /* fixed CID 209911, CHECKED_RETURN */
    if (hmac_memory(index, key, keylen, in, inlen, out, &outlen)) {
        EMSG("hmac memory failed");
        return TEE_ERROR_GENERIC;
    }

    memset((void*)out, 0, 32);
    outlen = 32;

    res = hmac_sha256(key, keylen, in, inlen, out, &outlen);
    if (res == TEE_SUCCESS) {
        EMSG("call hmac_sha256 success with len:%ld\n", outlen);
    } else {
        EMSG("call hmac_sha256 failed with 0x%08" PRIx32 "\n", res);
    }

    DMSG("TEE_CreatePersistentObject(%s) in TEE_STORAGE_PRIVATE(0x%08" PRIx32 ")...\n", name1, (uint32_t)TEE_STORAGE_PRIVATE);
    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
        name2, sizeof(name2),
        TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
        NULL, 0,
        &obj);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_CreatePersistentObject() returns 0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);

    DMSG("TEE_CreatePersistentObject(%s) in TEE_STORAGE_USER(0x%08" PRIx32 ")...\n", name2, (uint32_t)TEE_STORAGE_USER);
    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
        name1, sizeof(name1),
        TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
        NULL, 0,
        &obj);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_CreatePersistentObject() returns 0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
#endif

    /* If return value != TEE_SUCCESS the session will not be created. */
    return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void Hello_World_TA_CloseSessionEntryPoint(void* sess_ctx)
{
    (void)&sess_ctx; /* Unused parameter */
    IMSG("Goodbye!\n");
}

static uint8_t initial_data[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static TEE_Result PersistentObjectTest(uint32_t param_types,
    TEE_Param params[4])
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;
    uint8_t name[] = { 't', 'e', 's', 't', '1', '.', 't', 'x', 't', '\0' }; // object id: test1.txt
    uint8_t read_buf[256] = { 0 };
    size_t read_len;
    TEE_ObjectInfo obj_info;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
        TEE_PARAM_TYPE_MEMREF_INOUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types) {
        EMSG("bad parameters\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_CreatePersistentObject(%s)...\n", name);
    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
        name, sizeof(name),
        TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
        NULL, 0,
        &obj);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_CreatePersistentObject() returns 0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_WriteObjectData()...\n");
    res = TEE_WriteObjectData(obj, initial_data, sizeof(initial_data));
    if (res != TEE_SUCCESS) {
        EMSG("TEE_WriteObjectData() returns 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    DMSG("TEE_SeekObjectData()...\n");
    res = TEE_SeekObjectData(obj, 0, TEE_DATA_SEEK_SET);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_SeekObjectData() returns 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    DMSG("TEE_ReadObjectData()...\n");
    res = TEE_ReadObjectData(obj, read_buf, sizeof(read_buf), &read_len);
    if ((res != TEE_SUCCESS) || (read_len != sizeof(read_buf))) {
        EMSG("TEE_ReadObjectData() returns 0x%08" PRIx32 ", read size = %zu\n", res, read_len);
        goto exit;
    }

    DMSG("TEE_MemCompare()...\n");
    if (TEE_MemCompare(read_buf, initial_data, sizeof(read_buf)) != 0) {
        EMSG("TEE_MemCompare() failed!\n");
        res = TEE_ERROR_GENERIC;
        goto exit;
    }

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);

    DMSG("TEE_OpenPersistentObject()...\n");
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, name, sizeof(name), TEE_DATA_FLAG_ACCESS_WRITE_META, &obj);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_SeekObjectData() returns 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    DMSG("TEE_GetObjectInfo1()...\n");
    res = TEE_GetObjectInfo1(obj, &obj_info);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_SeekObjectData() returns 0x%08" PRIx32 "\n", res);
        goto exit;
    }
    DMSG("data size: %zu\n", obj_info.dataSize);

    DMSG("TEE_CloseAndDeletePersistentObject1()...\n");
    res = TEE_CloseAndDeletePersistentObject1(obj);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_SeekObjectData() returns 0x%08" PRIx32 "\n", res);
        return res;
    }
    return TEE_SUCCESS;

exit:
    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);

    return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result Hello_World_TA_InvokeCommandEntryPoint(void* sess_ctx,
    uint32_t cmd_id,
    uint32_t param_types, TEE_Param params[4])
{
    (void)&sess_ctx; /* Unused parameter */

    DMSG("cmd: 0x%08" PRIx32 "\n", cmd_id);
    switch (cmd_id) {
    case TA_HELLO_WORLD_CMD_INC_VALUE:
#ifdef TRACE_PERF
        DMSG("trace perf(0)\n");
        return TEE_SUCCESS;
#else
        return PersistentObjectTest(param_types, params);
        // return inc_value(param_types, params);
#endif
    case TA_HELLO_WORLD_CMD_DEC_VALUE:
        // return dec_value(param_types, params);
        return TEE_SUCCESS;
    default:
        EMSG("unknown cmd: 0x%08" PRIx32 "\n", cmd_id);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

struct user_ta_head hello_world_user_ta_head = {
    .uuid = TA_HELLO_WORLD_UUID,
    .name = "HelloWorld",
    .flags = TA_FLAG_USER_MODE,
    .create_entry_point = Hello_World_TA_CreateEntryPoint,
    .destroy_entry_point = Hello_World_TA_DestroyEntryPoint,
    .open_session_entry_point = Hello_World_TA_OpenSessionEntryPoint,
    .close_session_entry_point = Hello_World_TA_CloseSessionEntryPoint,
    .invoke_command_entry_point = Hello_World_TA_InvokeCommandEntryPoint
};

struct user_ta_head* user_ta = &hello_world_user_ta_head;
