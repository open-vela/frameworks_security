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
#include <stdio.h>
#include <string.h>

#include <pin_ca_api.h>

int main(int argc, FAR char* argv[])
{
    /*
     * argv[1] : store/verify/change/hash
     * argv[2] : 0(undeletable) 1(deletable)
     * argv[3] : input PIN when store/verify
     *           old PIN when change
     * argv[4] : new PIN when change
     */

    bool is_deletable;

    if (argc != 3 && argc != 4 && argc != 5) {
        return -1;
    }

    if (atoi(argv[2]) == 0) {
        is_deletable = false;
    } else {
        is_deletable = true;
    }

    if (strcmp(argv[1], "store") == 0 && argc == 4) {
        char* buff = argv[3];
        if (pin_store(is_deletable, (uint8_t*)buff, strlen((const char*)buff)) == 0) {
            printf("store successfully.\n");
        } else {
            printf("store failed.\n");
        }
    }

    if (strcmp(argv[1], "verify") == 0 && argc == 4) {
        char* buff = argv[3];
        if (pin_verify(is_deletable, (uint8_t*)buff, strlen((const char*)buff)) == 0) {
            printf("verify successfully.\n");
        } else {
            printf("verify failed.\n");
        }
    }

    if (strcmp(argv[1], "change") == 0 && argc == 5) {
        char* old = argv[3];
        char* new = argv[4];
        if (pin_change(is_deletable, (uint8_t*)old, strlen((const char*)old),
                (uint8_t*)new, strlen((const char*)new))
            == 0) {
            printf("change successfully.\n");
        } else {
            printf("change failed.\n");
        }
    }

    if (strcmp(argv[1], "hash") == 0 && argc == 3) {
        uint8_t sha256[32];
        if (pin_getsha256(is_deletable, sha256, 32) == 0) {
            printf("hash successfully.\n");
            for (int i = 0; i < 32; i++) {
                printf("%02x ", sha256[i]);
            }
            printf("\n");
        } else {
            printf("hash failed.\n");
        }
    }

    if (strcmp(argv[1], "check") == 0 && argc == 3) {
        if (pin_is_exist(is_deletable) == true) {
            printf("pin is existed.\n");
        } else {
            printf("pin is not existed.\n");
        }
    }

    if (strcmp(argv[1], "delete") == 0 && argc == 3) {
        if (pin_delete(is_deletable) == 0) {
            printf("delete successfully.\n");
        } else {
            printf("delete failed.\n");
        }
    }

    return 0;
}