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
#include <pin_ta.h>
#include <string.h>
#include <tee_internal_api.h>
#include <trace.h>

static char* pin_name = "PIN";

static TEE_Result Pin_Store(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result Pin_Verify(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result Pin_Change(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result Pin_GetSha256(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result Pin_Check(uint32_t param_types __unused,
    TEE_Param params[4] __unused);
static TEE_Result Pin_Delete(uint32_t param_types __unused,
    TEE_Param params[4] __unused);

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result PIN_TA_CreateEntryPoint(void)
{
    DMSG("has been called\n");

    return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void PIN_TA_DestroyEntryPoint(void)
{
    DMSG("has been called\n");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result PIN_TA_OpenSessionEntryPoint(uint32_t param_types,
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
    DMSG("PIN TA!\n");

    /* If return value != TEE_SUCCESS the session will not be created. */
    return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void PIN_TA_CloseSessionEntryPoint(void __maybe_unused* sess_ctx)
{
    (void)&sess_ctx; /* Unused parameter */
    DMSG("Goodbye!\n");
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result PIN_TA_InvokeCommandEntryPoint(void __maybe_unused* sess_ctx,
    uint32_t cmd_id,
    uint32_t param_types, TEE_Param params[4])
{
    (void)&sess_ctx; /* Unused parameter */

    DMSG("cmd: 0x%08" PRIx32 "\n", cmd_id);
    switch (cmd_id) {
    case TA_PIN_CMD_STORE:
        return Pin_Store(param_types, params);
    case TA_PIN_CMD_VERIFY:
        return Pin_Verify(param_types, params);
    case TA_PIN_CMD_CHANGE:
        return Pin_Change(param_types, params);
    case TA_PIN_CMD_GETSHA256:
        return Pin_GetSha256(param_types, params);
    case TA_PIN_CMD_CHK:
        return Pin_Check(param_types, params);
    case TA_PIN_CMD_DEL:
        return Pin_Delete(param_types, params);
    default:
        EMSG("ee962c07: 0x%08" PRIx32 "\n", cmd_id);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

static TEE_Result Pin_Store(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_CreatePersistentObject...\n");

    if (params[0].value.a == 0) {
        res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
            pin_name, strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
            NULL, 0, &obj);
    } else {
        res = TEE_CreatePersistentObject(TEE_STORAGE_USER,
            pin_name, strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
            NULL, 0, &obj);
    }

    if (res != TEE_SUCCESS) {
        EMSG("e23a89fe:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_WriteObjectData()...\n");

    res = TEE_WriteObjectData(obj, params[1].memref.buffer,
        params[1].memref.size);

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

static TEE_Result Pin_Verify(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    size_t read_len;
    uint8_t data[32];

    if (param_types != exp_param_types) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject...\n");

    if (params[0].value.a == 0) {
        res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    } else {
        res = TEE_OpenPersistentObject(TEE_STORAGE_USER,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    }

    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_ReadObjectData()...\n");

    res = TEE_ReadObjectData(obj, data, sizeof(data), &read_len);
    if (res != TEE_SUCCESS) {
        EMSG("8d4785a7:0x%08" PRIx32 ",%zu\n",
            res, read_len);
        goto exit;
    }

    if (read_len != params[1].memref.size) {
        res = TEE_ERROR_GENERIC;
        goto exit;
    }

    for (int i = 0; i < read_len; i++) {
        if (data[i] != ((uint8_t*)params[1].memref.buffer)[i]) {
            res = TEE_ERROR_GENERIC;
            goto exit;
        }
    }

exit:
    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

static TEE_Result Pin_Change(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    size_t read_len;
    uint8_t data[32];

    if (param_types != exp_param_types) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject...\n");

    if (params[0].value.a == 0) {
        res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    } else {
        res = TEE_OpenPersistentObject(TEE_STORAGE_USER,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    }

    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_ReadObjectData()...\n");

    res = TEE_ReadObjectData(obj, data, sizeof(data), &read_len);
    if (res != TEE_SUCCESS) {
        EMSG("8d4785a7:0x%08" PRIx32 ",%zu\n",
            res, read_len);
        goto exit;
    }

    if (read_len != params[0].value.b) {
        res = TEE_ERROR_GENERIC;
        goto exit;
    }

    for (int i = 0; i < read_len; i++) {
        if (data[i] != ((uint8_t*)params[1].memref.buffer)[i]) {
            res = TEE_ERROR_GENERIC;
            goto exit;
        }
    }

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);

    /* write new pin */

    if (params[0].value.a == 0) {
        res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
            pin_name, strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
            NULL, 0, &obj);
    } else {
        res = TEE_CreatePersistentObject(TEE_STORAGE_USER,
            pin_name, strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_OVERWRITE, NULL,
            NULL, 0, &obj);
    }

    if (res != TEE_SUCCESS) {
        EMSG("e23a89fe:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_WriteObjectData()...\n");

    res = TEE_WriteObjectData(obj,
        (uint8_t*)params[1].memref.buffer + params[0].value.b,
        params[1].memref.size - params[0].value.b);

exit:
    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

static TEE_Result Pin_GetSha256(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;
    TEE_OperationHandle operation = NULL;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    size_t read_len;
    uint8_t data[32];

    if (param_types != exp_param_types) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject...\n");

    if (params[0].value.a == 0) {
        res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    } else {
        res = TEE_OpenPersistentObject(TEE_STORAGE_USER,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    }

    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_ReadObjectData()...\n");

    res = TEE_ReadObjectData(obj, data, sizeof(data), &read_len);
    if (res != TEE_SUCCESS) {
        EMSG("8d4785a7:0x%08" PRIx32 ",%zu\n",
            res, read_len);
        goto exit;
    }

    if (read_len == 0) {
        res = TEE_ERROR_GENERIC;
        goto exit;
    }

    res = TEE_AllocateOperation(&operation, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0);
    if (res != TEE_SUCCESS) {
        EMSG("9476476a: 0x%08" PRIx32, res);
        goto hash_end;
    }

    TEE_DigestUpdate(operation, (void*)data, read_len);
    read_len = 32;
    res = TEE_DigestDoFinal(operation, NULL, 0, params[1].memref.buffer,
        &read_len);
    if (read_len != 32) {
        res = TEE_ERROR_GENERIC;
    }

    if (res != TEE_SUCCESS) {
        EMSG("83f15f75:0x%08" PRIx32, res);
        goto hash_end;
    }

hash_end:
    if (operation != NULL) {
        TEE_FreeOperation(operation);
    }

exit:
    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

static TEE_Result Pin_Check(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject...\n");

    if (params[0].value.a == 0) {
        res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    } else {
        res = TEE_OpenPersistentObject(TEE_STORAGE_USER,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_READ,
            &obj);
    }

    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_CloseObject()...\n");
    TEE_CloseObject(obj);
    return res;
}

static TEE_Result Pin_Delete(uint32_t param_types __unused,
    TEE_Param params[4] __unused)
{
    TEE_Result res = TEE_ERROR_GENERIC;
    TEE_ObjectHandle obj;

    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types) {
        EMSG("718fc92c\n");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    DMSG("TEE_OpenPersistentObject...\n");

    if (params[0].value.a == 0) {
        res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_WRITE_META,
            &obj);
    } else {
        res = TEE_OpenPersistentObject(TEE_STORAGE_USER,
            pin_name,
            strlen(pin_name),
            TEE_DATA_FLAG_ACCESS_WRITE_META,
            &obj);
    }

    if (res != TEE_SUCCESS) {
        EMSG("c173d631:0x%08" PRIx32 "\n", res);
        return res;
    }

    DMSG("TEE_CloseAndDeletePersistentObject1()...\n");
    res = TEE_CloseAndDeletePersistentObject1(obj);
    if (res != TEE_SUCCESS) {
        EMSG("69aa1fce:0x%08" PRIx32 "\n", res);
        return res;
    }
    return TEE_SUCCESS;
}

struct user_ta_head pin_user_ta_head = {
    .uuid = TA_PIN_UUID,
    .name = "PIN",
    .flags = TA_FLAG_USER_MODE,
    .create_entry_point = PIN_TA_CreateEntryPoint,
    .destroy_entry_point = PIN_TA_DestroyEntryPoint,
    .open_session_entry_point = PIN_TA_OpenSessionEntryPoint,
    .close_session_entry_point = PIN_TA_CloseSessionEntryPoint,
    .invoke_command_entry_point = PIN_TA_InvokeCommandEntryPoint
};

struct user_ta_head* user_ta = &pin_user_ta_head;