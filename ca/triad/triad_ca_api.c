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

#include <fcntl.h>
#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

#if defined(CONFIG_LIB_TEEC)
#include <tee_client_api.h>
#include <teec_trace.h>
#endif

#include <triad_ta.h>

#if defined(CONFIG_LIB_TEEC)
int triad_store_did(uint8_t* did, uint16_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TRIAD_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    if (len != 8) {
        goto exit;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08"PRIx32"\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08"PRIx32"\n", res);
        goto exit_finalize;
    }
    memset(io_shm.buffer, 0, io_shm.size);
    memcpy(io_shm.buffer, did, len);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_STORE_DID, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
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

int triad_load_did(uint8_t* did, uint16_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TRIAD_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    if (len != 8) {
        goto exit;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08"PRIx32"\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08"PRIx32"\n", res);
        goto exit_finalize;
    }
    memset(io_shm.buffer, 0, io_shm.size);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    /*
     * Open a session to the "hello world" TA, the TA will print "hello
     * world!" in the log when the session is created.
     */

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_LOAD_DID, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_close_session;
    }

    memcpy(did, io_shm.buffer, len);

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

int triad_store_key(uint8_t* key, uint16_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TRIAD_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    if (len != 16) {
        goto exit;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08"PRIx32"\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08"PRIx32"\n", res);
        goto exit_finalize;
    }
    memset(io_shm.buffer, 0, io_shm.size);
    memcpy(io_shm.buffer, key, len);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_STORE_KEY, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
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

int triad_load_key(uint8_t* key, uint16_t len)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TRIAD_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    if (len != 16) {
        goto exit;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08"PRIx32"\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08"PRIx32"\n", res);
        goto exit_finalize;
    }
    memset(io_shm.buffer, 0, io_shm.size);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    /*
     * Open a session to the "hello world" TA, the TA will print "hello
     * world!" in the log when the session is created.
     */

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_LOAD_KEY, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_close_session;
    }

    memcpy(key, io_shm.buffer, len);

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

int triad_get_hmac(uint8_t* input, uint16_t inlen,
    uint8_t* output, uint16_t outlen)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TRIAD_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    if (inlen == 0 || outlen != 32) {
        goto exit;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);

    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08"PRIx32"\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    if (inlen < 32)
        io_shm.size = 32;
    else
        io_shm.size = inlen;

    io_shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08"PRIx32"\n", res);
        goto exit_finalize;
    }

    memset(io_shm.buffer, 0, io_shm.size);
    memcpy(io_shm.buffer, input, inlen);

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);

    /*
     * Open a session to the "hello world" TA, the TA will print "hello
     * world!" in the log when the session is created.
     */

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_VALUE_INPUT,
        TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &io_shm;
    op.params[1].value.a = inlen;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_GET_HMAC, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08"PRIx32" origin 0x%08"PRIx32"\n",
            res, err_origin);
        goto exit_close_session;
    }

    memcpy(output, io_shm.buffer, 32);

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
#else /* !CONFIG_LIB_TEEC */
static int load_data(const char* filename, uint8_t* data, uint16_t len)
{
    ssize_t bytes_read;
    int fd;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return -1;
    }

    memset(data, 0, len);
    bytes_read = read(fd, data, len);
    if (bytes_read == -1) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int store_data(const char* filename, uint8_t* data, uint16_t len)
{
    ssize_t bytes_write;
    int fd;

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        return -1;
    }

    bytes_write = write(fd, data, len);
    if (bytes_write < -1) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int triad_store_did(uint8_t* did, uint16_t len)
{
    const char* filename = "/data/triad_did.txt";
    return store_data(filename, did, len);
}

int triad_load_did(uint8_t* did, uint16_t len)
{
    const char* filename = "/data/triad_did.txt";
    return load_data(filename, did, len);
}

int triad_store_key(uint8_t* key, uint16_t len)
{
    const char* filename = "/data/triad_key.txt";
    return store_data(filename, key, len);
}

int triad_load_key(uint8_t* key, uint16_t len)
{
    const char* filename = "/data/triad_key.txt";
    return load_data(filename, key, len);
}
#endif
