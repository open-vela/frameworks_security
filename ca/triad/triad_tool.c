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

#include <triad_ca_api.h>

static void usage(void)
{
    printf(" \tset did <did_string> \n");
    printf(" \tget did \n");
    printf(" \tset key <key_string> \n");
    printf(" \tget key \n");
}

int main(int argc, char* const argv[])
{
    int ret = -1;

    if (argc == 3 && !strcmp("get", argv[1])) {
        if (!strcmp("did", argv[2])) {
            uint8_t did[8] = { 0 };
            ret = triad_load_did(did, 8);
            if (!ret) {
                char result[32];
                printf("optee:did byte %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x \n",
                    did[0], did[1], did[2], did[3], did[4], did[5], did[6], did[7]);
                snprintf(result, sizeof(result), "%" PRIu64, *(uint64_t*)did);
                printf("optee: get did key data: %s \n", result);
            } else {
                printf("optee: get did failed !\n");
            }
        } else if (!strcmp("key", argv[2])) {
            uint8_t key[16] = { 0 };
            char ret_key[16] = { 0 };

            ret = triad_load_key(key, 16);
            if (!ret) {
                for (int i = 0; i < 16; i++) {
                    ret_key[i] = (char)key[i];
                }

                printf("optee: get key data: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
                    ret_key[0], ret_key[1], ret_key[2], ret_key[3], ret_key[4],
                    ret_key[5], ret_key[6], ret_key[7], ret_key[8], ret_key[9],
                    ret_key[10], ret_key[11], ret_key[12], ret_key[13], ret_key[14],
                    ret_key[15]);
            } else {
                printf("optee: get key failed !\n");
            }
        }
    } else if (argc == 4 && !strcmp("set", argv[1])) {
        if (!strcmp("did", argv[2])) {
            char* end;
            uint64_t did_num = strtoull(argv[3], &end, 10);
            uint8_t* did_group = (uint8_t*)&did_num;
            printf("optee: did string:%s, did number:%" PRIu64 " \n", argv[3], did_num);
            printf("optee: did byte:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x \n",
                did_group[0], did_group[1], did_group[2], did_group[3],
                did_group[4], did_group[5], did_group[6], did_group[7]);
            ret = triad_store_did((uint8_t*)&did_num, 8);
            if (!ret) {
                printf("optee: set did key success!\n");
            } else {
                printf("optee: set did key failed!\n");
            }
        } else if (!strcmp("key", argv[2])) {
            uint8_t key[16] = { 0 };
            for (uint8_t i = 0; i < 16; i++) {
                key[i] = (uint8_t)argv[3][i];
            }
            ret = triad_store_key(key, 16);
            if (!ret) {
                printf("optee: set key success!\n");
            } else {
                printf("optee: set key failed !\n");
            }
        }
    } else {
        printf("wrong argc\n");
        usage();
        return -1;
    }

    return 0;
}