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

#include <tee_client_api.h>
#include <teec_trace.h>

#include <triad_ta.h>

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
        EMSG("TEEC_InitializeContext failed with code 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
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
        EMSG("TEEC_Opensession failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_STORE_DID, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
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
        EMSG("TEEC_InitializeContext failed with code 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
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
        EMSG("TEEC_Opensession failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_LOAD_DID, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
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
        EMSG("TEEC_InitializeContext failed with code 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
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
        EMSG("TEEC_Opensession failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE, TEEC_NONE,
        TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_STORE_KEY, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
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
        EMSG("TEEC_InitializeContext failed with code 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));

    io_shm.size = len;
    io_shm.flags = TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
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
        EMSG("TEEC_Opensession failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &io_shm;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_LOAD_KEY, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
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
        EMSG("TEEC_InitializeContext failed with code 0x%08" PRIx32 "\n", res);
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
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
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
        EMSG("TEEC_Opensession failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_VALUE_INPUT,
        TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &io_shm;
    op.params[1].value.a = inlen;

    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_GET_HMAC, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
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

int triad_gcm_encrypt(const unsigned char* iv, size_t iv_len,
    const unsigned char* aad, size_t aad_len,
    const unsigned char* input, size_t length,
    unsigned char* tag, size_t tag_len,
    unsigned char* output)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TRIAD_UUID;
    TEEC_SharedMemory input_shm;
    TEEC_SharedMemory output_shm;
    struct tk_auth_hdr msg;
    uint32_t msg_len = 0;
    uint32_t err_origin;

    if (sizeof(msg) + iv_len + aad_len + length > CONFIG_CA_TRIAD_GCM_MAX_LEN) {
        EMSG("GCM encrypt input message is too long\n");
        goto exit;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));
    input_shm.size = sizeof(struct tk_auth_hdr) + iv_len + aad_len + length;
    input_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &input_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
        goto exit_finalize;
    }

    memset(input_shm.buffer, 0, input_shm.size);

    /* padding input message |header|iv|aad|input| */

    msg.iv_len = iv_len;
    msg.aad_len = aad_len;
    msg.tag_len = tag_len;
    msg.data_len = length;
    memcpy(input_shm.buffer, &msg, sizeof(msg));
    msg_len += sizeof(msg);
    memcpy(input_shm.buffer + msg_len, iv, iv_len);
    msg_len += iv_len;
    memcpy(input_shm.buffer + msg_len, aad, aad_len);
    msg_len += aad_len;
    memcpy(input_shm.buffer + msg_len, input, length);

    /* allocate output space */

    output_shm.size = length + tag_len;
    output_shm.flags = TEEC_MEM_OUTPUT;
    res = TEEC_AllocateSharedMemory(&ctx, &output_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
        goto exit_finalize;
    }

    memset(output_shm.buffer, 0, output_shm.size);
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
        TEEC_NONE,
        TEEC_NONE,
        TEEC_NONE);

    /*
     * Open a session to the "hello world" TA, the TA will print "hello
     * world!" in the log when the session is created.
     */

    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC,
        NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE,
        TEEC_MEMREF_WHOLE,
        TEEC_NONE,
        TEEC_NONE);
    op.params[0].memref.parent = &input_shm;
    op.params[1].memref.parent = &output_shm;
    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_GCM_ENCRYPT,
        &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_close_session;
    }

    memcpy(output, output_shm.buffer, length);
    memcpy(tag, output_shm.buffer + length, tag_len);

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_free_mem:
    DMSG("TEEC_ReleaseSharedMemory...\n");
    TEEC_ReleaseSharedMemory(&input_shm);
    TEEC_ReleaseSharedMemory(&output_shm);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    return res;
}

int triad_gcm_decrypt(const unsigned char* iv, size_t iv_len,
    const unsigned char* aad, size_t aad_len,
    const unsigned char* tag, size_t tag_len,
    const unsigned char* input, size_t length,
    unsigned char* output)
{
    TEEC_Result res = TEEC_ERROR_GENERIC;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_TRIAD_UUID;
    TEEC_SharedMemory input_shm;
    TEEC_SharedMemory output_shm;
    struct tk_auth_hdr msg;
    uint32_t msg_len = 0;
    uint32_t err_origin;

    if (sizeof(msg) + iv_len + aad_len + tag_len + length > CONFIG_CA_TRIAD_GCM_MAX_LEN) {
        EMSG("GCM decrypt input message is too long\n");
        goto exit;
    }

    /* Initialize a context connecting us to the TEE */

    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08" PRIx32 "\n", res);
        goto exit;
    }

    /* Clear the TEEC_Operation struct */

    memset(&op, 0, sizeof(op));
    input_shm.size = sizeof(struct tk_auth_hdr) + iv_len + aad_len + tag_len + length;
    input_shm.flags = TEEC_MEM_INPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &input_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
        goto exit_finalize;
    }

    memset(input_shm.buffer, 0, input_shm.size);

    /* padding input message |header|iv|aad|tag|input| */

    msg.iv_len = iv_len;
    msg.aad_len = aad_len;
    msg.tag_len = tag_len;
    msg.data_len = length;
    memcpy(input_shm.buffer, &msg, sizeof(msg));
    msg_len += sizeof(msg);
    memcpy(input_shm.buffer + msg_len, iv, iv_len);
    msg_len += iv_len;
    memcpy(input_shm.buffer + msg_len, aad, aad_len);
    msg_len += aad_len;
    memcpy(input_shm.buffer + msg_len, tag, tag_len);
    msg_len += tag_len;
    memcpy(input_shm.buffer + msg_len, input, length);

    /* the initial output_shm.size are set as same as inlen, since the size of
     * decrypted content must be smaller than the size of encrypted content.
     */

    output_shm.size = length;
    output_shm.flags = TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &output_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08" PRIx32 "\n", res);
        goto exit_finalize;
    }

    memset(output_shm.buffer, 0, output_shm.size);
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
        TEEC_NONE,
        TEEC_NONE,
        TEEC_NONE);
    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC,
        NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_free_mem;
    }

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_MEMREF_WHOLE,
        TEEC_VALUE_OUTPUT, TEEC_NONE);
    op.params[0].memref.parent = &input_shm;
    op.params[1].memref.parent = &output_shm;
    res = TEEC_InvokeCommand(&sess, TA_TRIAD_CMD_GCM_DECRYPT, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08" PRIx32 " origin 0x%08" PRIx32 "\n",
            res, err_origin);
        goto exit_close_session;
    }

    memcpy(output, output_shm.buffer, length);

exit_close_session:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);
exit_free_mem:
    DMSG("TEEC_ReleaseSharedMemory...\n");
    TEEC_ReleaseSharedMemory(&input_shm);
    TEEC_ReleaseSharedMemory(&output_shm);
exit_finalize:
    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);
exit:
    return res;
}
