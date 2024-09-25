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

#include <kernel/user_ta.h>
#include <string.h>
#include <tee_internal_api.h>
#include <trace.h>
#include <triad_ta.h>

#define TA_OBJECT_NAME_KEY "triad_key"
#define TA_OBJECT_NAME_DID "triad_did"

extern int find_hash(const char* name);
extern int hmac_memory(int hash,
    const unsigned char* key, uint32_t keylen,
    const unsigned char* in, uint32_t inlen,
    unsigned char* out, uint32_t* outlen);

static TEE_Result TA_Load_DID(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result TA_Load_Key(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result TA_Store_DID(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result TA_Store_Key(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result TA_Get_HMAC(uint32_t param_types __unused,
    TEE_Param params[4] __unused);

/* | Object Type           | Possible Key Sizes                            |
 * +-----------------------+-----------------------------------------------+
 * | TEE_TYPE_HMAC_SHA256  | Between 192 and 1024 bits, multiple of 8 bits |
 */
#define MAX_KEY_SIZE 128
#define MIN_KEY_SIZE 24

static TEE_Result hmac_sha256(uint8_t* key, uint32_t keylen,
    uint8_t* in, uint32_t inlen,
    uint8_t* out, size_t* outlen)
{
    TEE_Attribute attr = { 0 };
    TEE_ObjectHandle key_handle = TEE_HANDLE_NULL;
    TEE_OperationHandle op_handle = TEE_HANDLE_NULL;
    TEE_Result res = TEE_SUCCESS;

    if (keylen < MIN_KEY_SIZE || keylen > MAX_KEY_SIZE) {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    res = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA256, keylen * 8,
        &key_handle);
    if (res != TEE_SUCCESS) {
        EMSG("e8bc7c84:0x%08" PRIx32 "\n", res);
        goto exit;
    }

    TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, keylen);

    res = TEE_PopulateTransientObject(key_handle, &attr, 1);
    if (res != TEE_SUCCESS) {
        EMSG("ea67b44c:0x%08" PRIx32 "\n", res);
        goto exit;
    }

    res = TEE_AllocateOperation(&op_handle, TEE_ALG_HMAC_SHA256, TEE_MODE_MAC,
        keylen * 8);
    if (res != TEE_SUCCESS) {
        EMSG("9476476a:0x%08" PRIx32 "\n", res);
        goto exit;
    }

    res = TEE_SetOperationKey(op_handle, key_handle);
    if (res != TEE_SUCCESS) {
        EMSG("2c47d055:0x%08" PRIx32 "\n", res);
        goto exit;
    }

    TEE_MACInit(op_handle, NULL, 0);
    TEE_MACUpdate(op_handle, in, inlen);
    res = TEE_MACComputeFinal(op_handle, NULL, 0, out, outlen);

exit:
    if (op_handle != TEE_HANDLE_NULL) {
        TEE_FreeOperation(op_handle);
    }

    TEE_FreeTransientObject(key_handle);

    return res;
}

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TRIAD_TA_CreateEntryPoint(void)
{
    DMSG("has been called\n");

    return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TRIAD_TA_DestroyEntryPoint(void)
{
    DMSG("has been called\n");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TRIAD_TA_OpenSessionEntryPoint(uint32_t param_types,
    TEE_Param __maybe_unused params[4],
    void __maybe_unused** sess_ctx)
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    DMSG("has been called\n");
    if (param_types != exp_param_types) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    /*
     * The DMSG() macro is non-standard, TEE Internal API doesn't
     * specify any means to logging from a TA.
     */
    DMSG("TRIAD TA!\n");

    /* If return value != TEE_SUCCESS the session will not be created. */
    return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TRIAD_TA_CloseSessionEntryPoint(void __maybe_unused* sess_ctx)
{
    (void)&sess_ctx; /* Unused parameter */
    DMSG("Goodbye!\n");
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TRIAD_TA_InvokeCommandEntryPoint(void __maybe_unused* sess_ctx,
    uint32_t cmd_id,
    uint32_t param_types, TEE_Param params[4])
{
    (void)&sess_ctx; /* Unused parameter */

    DMSG("cmd: 0x%08" PRIx32 "\n", cmd_id);
    switch (cmd_id) {
    case TA_TRIAD_CMD_STORE_KEY:
        return TA_Store_Key(param_types, params);
    case TA_TRIAD_CMD_LOAD_KEY:
        return TA_Load_Key(param_types, params);
    case TA_TRIAD_CMD_STORE_DID:
        return TA_Store_DID(param_types, params);
    case TA_TRIAD_CMD_LOAD_DID:
        return TA_Load_DID(param_types, params);
    case TA_TRIAD_CMD_GET_HMAC:
        return TA_Get_HMAC(param_types, params);
    default:
        EMSG("ee962c07: 0x%08" PRIx32 "\n", cmd_id);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

static TEE_Result TA_Store_Key(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;
    uint8_t name[] = TA_OBJECT_NAME_KEY;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types || params[0].memref.size != 16) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_CreatePersistentObject(%s)...\n", name);
    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
        name, sizeof(name),
        TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
        NULL, 0,
        &obj);
    if (res != TEE_SUCCESS) {
        EMSG("e23a89fe:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_WriteObjectData()...\n");

    res = TEE_WriteObjectData(obj, params[0].memref.buffer,
        params[0].memref.size);

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);

    return res;
}

static TEE_Result TA_Load_Key(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;
    uint8_t name[] = TA_OBJECT_NAME_KEY;
    size_t read_len;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types || params[0].memref.size != 16) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject()...\n");
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, name, sizeof(name),
        TEE_DATA_FLAG_ACCESS_READ, &obj);
    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_ReadObjectData()...\n");
    res = TEE_ReadObjectData(obj, params[0].memref.buffer, 16, &read_len);
    if ((res != TEE_SUCCESS) || (read_len != 16)) {
        EMSG("8d4785a7:0x%08" PRIx32 ",%zu\n",
            res, read_len);
    }

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

static TEE_Result TA_Store_DID(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;
    uint8_t name[] = TA_OBJECT_NAME_DID;
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types || params[0].memref.size != 8) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_CreatePersistentObject(%s)...\n", name);
    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
        name, sizeof(name),
        TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
        NULL, 0,
        &obj);
    if (res != TEE_SUCCESS) {
        EMSG("e23a89fe:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_WriteObjectData()...\n");

    res = TEE_WriteObjectData(obj, params[0].memref.buffer,
        params[0].memref.size);

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);

    return res;
}

static TEE_Result TA_Load_DID(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;
    uint8_t name[] = TA_OBJECT_NAME_DID;
    size_t read_len;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types || params[0].memref.size != 8) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject()...\n");
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, name, sizeof(name),
        TEE_DATA_FLAG_ACCESS_READ, &obj);
    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_ReadObjectData()...\n");
    res = TEE_ReadObjectData(obj, params[0].memref.buffer, 8, &read_len);
    if ((res != TEE_SUCCESS) || (read_len != 8)) {
        EMSG("8d4785a7:0x%08" PRIx32 ",%zu\n",
            res, read_len);
    }

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

static TEE_Result TA_Get_HMAC(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;
    uint8_t name[] = TA_OBJECT_NAME_KEY;
    size_t read_len;
    uint8_t key[32];
    uint8_t hmac[32];
    int ret;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
        TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types || params[0].memref.size < 32
        || params[1].value.a == 0) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject()...\n");
    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, name, sizeof(name),
        TEE_DATA_FLAG_ACCESS_READ, &obj);
    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_ReadObjectData()...\n");

    memset(key, 0, sizeof(key));
    res = TEE_ReadObjectData(obj, key, 16, &read_len);
    if ((res != TEE_SUCCESS) || (read_len != 16)) {
        EMSG("8d4785a7:0x%08" PRIx32 ",%zu\n",
            res, read_len);
        if (res != TEE_SUCCESS) {
            return TEE_SUCCESS;
        } else {
            goto exit;
        }
    }

    read_len = 32;
    ret = hmac_sha256(key, 32,
        (uint8_t*)params[0].memref.buffer,
        params[1].value.a, hmac, &read_len);

    if (ret != 0 || read_len != 32) {
        EMSG("f2e10a84:%d\n", ret);
        res = TEE_ERROR_GENERIC;
    } else {
        memcpy(params[0].memref.buffer, hmac, 32);
    }

exit:
    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

struct user_ta_head triad_user_ta_head = {
    .uuid = TA_TRIAD_UUID,
    .name = "TRIAD",
    .flags = TA_FLAG_USER_MODE,
    .create_entry_point = TRIAD_TA_CreateEntryPoint,
    .destroy_entry_point = TRIAD_TA_DestroyEntryPoint,
    .open_session_entry_point = TRIAD_TA_OpenSessionEntryPoint,
    .close_session_entry_point = TRIAD_TA_CloseSessionEntryPoint,
    .invoke_command_entry_point = TRIAD_TA_InvokeCommandEntryPoint
};

struct user_ta_head* user_ta = &triad_user_ta_head;
