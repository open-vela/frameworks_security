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
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, /* 1 */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, /* 10 */
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF /* 256 bytes */
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
