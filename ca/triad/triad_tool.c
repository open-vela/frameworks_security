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

#define TRIAD_DID_SIZE 8
#define TRIAD_KEY_SIZE 16
#define TRIAD_HMAC_SIZE 32

static void usage(void)
{
    printf("usage:\n"
           "\tca_triad_tool set did <did_string> \n"
           "\tca_triad_tool get did \n"
           "\tca_triad_tool set key <key_string> \n"
           "\tca_triad_tool get key \n"
           "\tExample: ca_triad_tool set did 12345678\n"
           "\tExample: ca_triad_tool set key abcdefgh12345678\n");
}

int main(int argc, char* const argv[])
{
    int ret = -1;

    if (argc == 3 && !strcmp("get", argv[1])) {
        if (!strcmp("did", argv[2])) {
            uint8_t did[TRIAD_DID_SIZE] = { 0 };
            ret = triad_load_did(did, TRIAD_DID_SIZE);
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
            uint8_t key[TRIAD_KEY_SIZE] = { 0 };

            ret = triad_load_key(key, TRIAD_KEY_SIZE);
            if (!ret) {
                printf("optee: get key data: %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
                    key[0], key[1], key[2], key[3],
                    key[4], key[5], key[6], key[7],
                    key[8], key[9], key[10], key[11],
                    key[12], key[13], key[14], key[15]);
            } else {
                printf("optee: get key failed !\n");
            }
        }
    } else if (argc == 4 && !strcmp("set", argv[1])) {
        if (!strcmp("did", argv[2])) {
            if (strlen(argv[3]) != TRIAD_DID_SIZE) {
                printf("did length must be %d\n", TRIAD_DID_SIZE);
                return -1;
            }
            char* end;
            uint64_t did_num = strtoull(argv[3], &end, 10);
            uint8_t* did_group = (uint8_t*)&did_num;
            printf("optee: did string:%s, did number:%" PRIu64 " \n", argv[3], did_num);
            printf("optee: did byte:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x \n",
                did_group[0], did_group[1], did_group[2], did_group[3],
                did_group[4], did_group[5], did_group[6], did_group[7]);
            ret = triad_store_did(did_group, TRIAD_DID_SIZE);
            if (!ret) {
                printf("optee: set did key success!\n");
            } else {
                printf("optee: set did key failed!\n");
            }
        } else if (!strcmp("key", argv[2])) {
            if (strlen(argv[3]) != TRIAD_KEY_SIZE) {
                printf("key length must be %d\n", TRIAD_KEY_SIZE);
                return -1;
            }
            uint8_t key[TRIAD_KEY_SIZE] = { 0 };
            for (uint8_t i = 0; i < TRIAD_KEY_SIZE; i++) {
                key[i] = argv[3][i];
            }
            ret = triad_store_key(key, TRIAD_KEY_SIZE);
            if (!ret) {
                printf("optee: set key success!\n");
            } else {
                printf("optee: set key failed !\n");
            }
        }
    } else {
        printf("Unrecognized option: %s\n", argv[1]);
        usage();
        return -1;
    }

    return 0;
}