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

#include <nuttx/config.h>
#include <pin_ta.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <teec_trace.h>

uint32_t pin_store(bool is_deletable, uint8_t* buff, uint32_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_PIN_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08lx\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08lx\n", res);
        goto exit_finalize;
    }

    memcpy(io_shm.buffer, buff, len);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_WHOLE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = is_deletable ? 1 : 0;
    op.params[1].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_PIN_CMD_STORE, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_close_session;
    }

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_free_mem:
    DMSG("TEEC_ReleaseSharedMemory...\n");
    TEEC_ReleaseSharedMemory(&io_shm);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    return res;
}

uint32_t pin_verify(bool is_deletable, uint8_t* buff, uint32_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_PIN_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08lx\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08lx\n", res);
        goto exit_finalize;
    }

    memcpy(io_shm.buffer, buff, len);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_WHOLE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = is_deletable ? 1 : 0;
    op.params[1].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_PIN_CMD_VERIFY, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_close_session;
    }

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_free_mem:
    DMSG("TEEC_ReleaseSharedMemory...\n");
    TEEC_ReleaseSharedMemory(&io_shm);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    return res;
}

uint32_t pin_change(bool is_deletable, uint8_t* old, uint32_t oldlen,
    uint8_t* new, uint32_t newlen)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_PIN_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08lx\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = oldlen + newlen;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08lx\n", res);
        goto exit_finalize;
    }

    memcpy(io_shm.buffer, old, oldlen);
    memcpy(io_shm.buffer + oldlen, new, newlen);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_WHOLE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = is_deletable ? 1 : 0;
    op.params[0].value.b = oldlen;
    op.params[1].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_PIN_CMD_CHANGE, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_close_session;
    }

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_free_mem:
    DMSG("TEEC_ReleaseSharedMemory...\n");
    TEEC_ReleaseSharedMemory(&io_shm);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    return res;
}

uint32_t pin_getsha256(bool is_deletable, uint8_t* buff, uint32_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_PIN_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    if (len != 32) {
        return (uint32_t)-1;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08lx\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = 32;
    io_shm.flags = TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08lx\n", res);
        goto exit_finalize;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_WHOLE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = is_deletable ? 1 : 0;
    op.params[1].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_PIN_CMD_GETSHA256, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_close_session;
    }

    memcpy(buff, io_shm.buffer, 32);

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_free_mem:
    DMSG("TEEC_ReleaseSharedMemory...\n");
    TEEC_ReleaseSharedMemory(&io_shm);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    return res;
}

bool pin_is_exist(bool is_deletable)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_PIN_UUID;
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08lx\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_finalize;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = is_deletable ? 1 : 0;

    res = TEEC_InvokeCommand(&sess, TA_PIN_CMD_CHK, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_close_session;
    }

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    if (res != TEEC_SUCCESS) {
        return false;
    } else {
        return true;
    }
}

uint32_t pin_delete(bool is_deletable)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_PIN_UUID;
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08lx\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_finalize;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);
    op.params[0].value.a = is_deletable ? 1 : 0;

    res = TEEC_InvokeCommand(&sess, TA_PIN_CMD_DEL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_close_session;
    }

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    return res;
}