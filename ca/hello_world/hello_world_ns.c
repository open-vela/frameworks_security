/*
 * Copyright (c) 2016, Linaro Limited
 * Copyright (C) 2022-2024 Xiaomi Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

#include <hello_world_ta.h>
#include <tee_client_api.h>
#include <teec_trace.h>

int main(int argc, FAR char* argv[])
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
    TEEC_SharedMemory io_shm;
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */
    DMSG("TEEC_InitializeContext...\n");
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InitializeContext failed with code 0x%08lx\n", res);
        return 0;
    }

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));
#if 1
    io_shm.size = 20;
    io_shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
    DMSG("TEEC_AllocateSharedMemory...\n");
    res = TEEC_AllocateSharedMemory(&ctx, &io_shm);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_AllocateSharedMemory failed with code 0x%08lx\n", res);
        goto exit;
    }
    memset(io_shm.buffer, 0, io_shm.size);
    memcpy(io_shm.buffer, "hi ta!", sizeof("hi ta!"));

    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_WHOLE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    op.params[0].memref.parent = &io_shm;
#else
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);
#endif
    /*
     * Open a session to the "hello world" TA, the TA will print "hello
     * world!" in the log when the session is created.
     */
    DMSG("TEEC_OpenSession...\n");
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
        TEEC_LOGIN_PUBLIC, NULL, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_Opensession failed with code 0x%08lx origin 0x%08lx\n", res, err_origin);
        goto clean_exit;
    }
    if (op.params[0].memref.size > 0) {
        DMSG("received msg from ta: %s\n", (char*)io_shm.buffer);
    }

    /*
     * Execute a function in the TA by invoking it, in this case
     * we're incrementing a number.
     *
     * The value of command ID part and how the parameters are
     * interpreted is part of the interface provided by the TA.
     */

    /*
     * Prepare the argument. Pass a value in the first parameter,
     * the remaining three parameters are unused.
     */
    memcpy(io_shm.buffer, "hi ta!", sizeof("hi ta!"));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_MEMREF_WHOLE,
        TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = 42;
    op.params[1].memref.parent = &io_shm;

    /*
     * TA_HELLO_WORLD_CMD_INC_VALUE is the actual function in the TA to be
     * called.
     */
    DMSG("Invoking TA to increment 0x%08" PRIx32 "\n", op.params[0].value.a);
    res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_INC_VALUE, &op,
        &err_origin);
    if (res != TEEC_SUCCESS) {
        EMSG("TEEC_InvokeCommand failed with code 0x%08lx origin 0x%08lx\n", res, err_origin);
        goto clean_exit;
    }
    DMSG("TA incremented value to 0x%08" PRIx32 "\n", op.params[0].value.a);

    /*
     * We're done with the TA, close the session and
     * destroy the context.
     *
     * The TA will print "Goodbye!" in the log when the
     * session is closed.
     */

clean_exit:
    DMSG("TEEC_ReleaseSharedMemory...\n");
    TEEC_ReleaseSharedMemory(&io_shm);
exit:
    DMSG("TEEC_CloseSession...\n");
    TEEC_CloseSession(&sess);

    DMSG("TEEC_FinalizeContext...\n");
    TEEC_FinalizeContext(&ctx);

    DMSG("exit(0)\n");
    return 0;
}
