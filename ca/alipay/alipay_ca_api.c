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

#include <alipay_ta.h>
#include <nuttx/config.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <teec_trace.h>

#define ALIPAY_PRIVATE_KEY_KEY "alipay_private_key"
#define ALIPAY_SHARE_KEY_KEY "alipay_share_key"
#define ALIPAY_SEED_KEY "alipay_seed"
#define ALIPAY_TIMEDIFF_KEY "alipay_timediff"
#define ALIPAY_NICK_NAME_KEY "alipay_nick_name"
#define ALIPAY_LOGON_ID_KEY "alipay_logon_id"
#define ALIPAY_BIND_FLAG_KEY "alipay_bind_flag"

static int get_item(const char* item_name)
{
    if (strcmp(item_name, ALIPAY_PRIVATE_KEY_KEY) == 0) {
        return 0;
    }
    if (strcmp(item_name, ALIPAY_SHARE_KEY_KEY) == 0) {
        return 1;
    }
    if (strcmp(item_name, ALIPAY_SEED_KEY) == 0) {
        return 2;
    }
    if (strcmp(item_name, ALIPAY_TIMEDIFF_KEY) == 0) {
        return 3;
    }
    if (strcmp(item_name, ALIPAY_NICK_NAME_KEY) == 0) {
        return 4;
    }
    if (strcmp(item_name, ALIPAY_LOGON_ID_KEY) == 0) {
        return 5;
    }
    if (strcmp(item_name, ALIPAY_BIND_FLAG_KEY) == 0) {
        return 6;
    }
    return -1;
}

uint32_t alipay_tee_data_read(const char* item_name, uint8_t* buff,
    uint32_t* out_len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_ALIPAY_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;
    int item;

    item = get_item(item_name);

    if (item < 0) {
        goto exit;
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

    io_shm.size = *out_len;
    io_shm.flags = TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08lx\n", res);
        goto exit_finalize;
    }
    memset(io_shm.buffer, 0, io_shm.size);

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

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_MEMREF_WHOLE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = item;
    op.params[1].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_ALIPAY_CMD_RD, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n",
            res, err_origin);
        goto exit_close_session;
    }

    *out_len = op.params[0].value.b;

    memcpy(buff, io_shm.buffer, *out_len);

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

uint32_t alipay_tee_data_write(const char* item_name, const uint8_t* buf,
    uint32_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_ALIPAY_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;
    int item;

    item = get_item(item_name);

    if (item < 0) {
        goto exit;
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

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08lx\n", res);
        goto exit_finalize;
    }
    memcpy(io_shm.buffer, buf, io_shm.size);

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
    op.params[0].value.a = item;
    op.params[1].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_ALIPAY_CMD_WR, &op, &err_origin);
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

uint32_t alipay_tee_data_delete(const char* item_name)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_ALIPAY_UUID;
    uint32_t err_origin;
    int item;

    item = get_item(item_name);

    if (item < 0) {
        goto exit;
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
    op.params[0].value.a = item;

    res = TEEC_InvokeCommand(&sess, TA_ALIPAY_CMD_DEL, &op, &err_origin);
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

bool is_alipay_tee_data_exited(const char* item_name)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_ALIPAY_UUID;
    uint32_t err_origin;
    int item;

    item = get_item(item_name);

    if (item < 0) {
        goto exit;
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
    op.params[0].value.a = item;

    res = TEEC_InvokeCommand(&sess, TA_ALIPAY_CMD_CHK, &op, &err_origin);
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
    return res != TEEC_SUCCESS;
}
