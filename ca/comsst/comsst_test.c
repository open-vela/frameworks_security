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

#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <comsst_ca_api.h>

static uint8_t buffer[512];
static uint32_t len;

uint32_t comsst_data_read(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t* out_len);
uint32_t comsst_data_write(uint8_t* scope, uint8_t* name, bool is_deletable,
    uint8_t* buff, uint32_t len);
uint32_t comsst_data_delete(uint8_t* scope, uint8_t* name, bool is_deletable);
uint32_t is_comsst_data_exited(uint8_t* scope, uint8_t* name,
    bool is_deletable);

static void usage(void)
{
    printf("usage:\n"
           "\tca_comsst_test check scope name is_deletable\n"
           "\tca_comsst_test read scope name is_deletable\n"
           "\tca_comsst_test write scope name is_deletable data\n"
           "\tca_comsst_test delete scope name is_deletable\n"
           "\tca_comsst_test verify scope name is_deletable data\n"
           "\tExample: ca_comsst_test write scope name 0 test_data \n");
}

int main(int argc, FAR char* argv[])
{
    /*
     * argv[1] : check/read/write/delete/verify
     * argv[2] : scope
     * argv[3] : name
     * argv[4] : 0(undeletable) 1(deletable)
     * argv[5] : write data(when argv[1] is write)
     */

    if (argc != 5 && argc != 6) {
        printf("Invalid argument number\n");
        usage();
        return -1;
    }

    uint8_t* scope = (uint8_t*)argv[2];
    uint8_t* name = (uint8_t*)argv[3];
    bool is_deletable = atoi(argv[4]) == 1;

    uint32_t res;
    clock_t start = clock();

    if (argc == 5 && strcmp(argv[1], "check") == 0) {
        res = is_comsst_data_exited(scope, name, is_deletable);
        if (res == 0) {
            printf("item is notexisted.\n");
        } else {
            printf("item isexisted\n");
        }
    } else if (argc == 5 && strcmp(argv[1], "delete") == 0) {
        if (comsst_data_delete(scope, name, is_deletable) == 0) {
            printf("item del successfully.\n");
        } else {
            printf("item del fail.\n");
        }
    } else if (argc == 5 && strcmp(argv[1], "read") == 0) {
        len = 512;
        memset(buffer, 0, 512);
        if (comsst_data_read(scope, name, is_deletable, buffer, &len) == 0) {
            printf("item read successfully. len = %ld\n", len);
            printf("item:%s\n", buffer);
        } else {
            printf("item read failed.\n");
        }
    } else if (argc == 6 && strcmp(argv[1], "write") == 0) {
        if (comsst_data_write(scope, name, is_deletable, (uint8_t*)argv[5],
                strlen(argv[5]))
            == 0) {
            printf("item write successfully.\n");
        } else {
            printf("item write failed.\n");
        }
    } else if (argc == 6 && strcmp(argv[1], "verify") == 0) {
        res = comsst_data_verify(scope, name, is_deletable, (uint8_t*)argv[5],
            strlen(argv[5]));
        if (res == 0) {
            printf("item verify successfully.\n");
        } else if (res == 0xffff0008) {
            printf("item is notexisted.\n");
        } else {
            printf("item verify failed.\n");
        }
    } else {
        printf("Unrecognized option: %s\n", argv[1]);
        usage();
        return -1;
    }

    uint32_t elapsed = (uint32_t)TICK2MSEC(clock() - start);
    printf("The time taken of %s operation is %ld ms.\n", argv[1], elapsed);

    return 0;
}