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

static const uint8_t test_key[16] = {
    0x46, 0x78, 0x5A, 0x43, 0x30, 0x76, 0x55, 0x78,
    0x63, 0x56, 0x7A, 0x61, 0x76, 0x4C, 0x74, 0x51
};

static const uint8_t txt[80] = {
    0x37, 0x29, 0xE6, 0x08, 0x9D, 0x3A, 0x36, 0x50,
    0x2A, 0x57, 0x9B, 0x56, 0x2D, 0xAB, 0xA0, 0x61,
    0x6B, 0xDC, 0x30, 0xC4, 0x4C, 0xDA, 0xC3, 0x94,
    0x3B, 0xD6, 0xF2, 0xCB, 0x7C, 0x57, 0x99, 0x88,
    0x61, 0x5B, 0xCC, 0x10, 0x44, 0xFC, 0x2C, 0x4A,
    0xC5, 0x6D, 0xA9, 0xF0, 0x16, 0x6F, 0xCF, 0xA2,
    0x9D, 0x04, 0x34, 0xA4, 0x44, 0xF0, 0xC4, 0xC5,
    0xF1, 0xE9, 0xDC, 0x40, 0xA2, 0xA5, 0x01, 0xF1,
    0xA8, 0x34, 0xAF, 0xA4, 0xE4, 0x59, 0x6B, 0xD3,
    0x1A, 0xF3, 0x3C, 0xBD, 0xF8, 0x0F, 0x74, 0x0C
};

static uint8_t hmac[TRIAD_HMAC_SIZE];
static uint8_t key[TRIAD_KEY_SIZE];
static uint8_t did[TRIAD_DID_SIZE];

static void usage(void)
{
    printf("usage:\n"
           "\tca_triad_test \n"
           "\tca_triad_test store\n");
}

int main(int argc, FAR char* argv[])
{
    /*
     * argv[1] : store (overwrite triad when reading a triple fails)
     */

    if (argc != 1 && argc != 2) {
        printf("Invalid argument number\n");
        usage();
        return -1;
    }

    bool store = false;
    if (argc == 2) {
        if (strcmp(argv[1], "store") == 0) {
            store = true;
        } else {
            printf("Unrecognized option: %s\n", argv[1]);
            usage();
            return -1;
        }
    }

    int res = 0;

    if (triad_load_did(did, TRIAD_DID_SIZE) == 0) {
        printf("load did ok, %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",
            did[0], did[1], did[2], did[3], did[4], did[5], did[6], did[7]);
    } else if (store) {
        memset(did, 0x12, TRIAD_DID_SIZE);
        printf("store did...\n");
        if (triad_store_did(did, TRIAD_DID_SIZE) == 0) {
            printf("store did ok\n");
        } else {
            printf("store did fail\n");
        }
    } else {
        printf("triad load did fail (nostore)\n");
        res = -1;
    }

    if (triad_load_key(key, TRIAD_KEY_SIZE) == 0) {
        printf("load key ok\n");
    } else if (store) {
        memcpy(key, test_key, TRIAD_KEY_SIZE);
        printf("store key...\n");
        if (triad_store_key(key, TRIAD_KEY_SIZE) == 0) {
            printf("store key ok\n");
        } else {
            printf("store did fail\n");
        }
    } else {
        printf("triad load key fail (nostore)\n");
        res = -1;
    }

    if (triad_get_hmac((uint8_t*)txt, sizeof(txt), hmac, TRIAD_HMAC_SIZE) == 0) {
        /*
         * hamc should be
         * 356daddc139eb72a4df6623f171312c39d3a994eed12b71503b42ee1af4c761f
         */

        printf("get hmac ok, ");
        for (int i = 0; i < TRIAD_HMAC_SIZE; i++) {
            printf("%02x,", hmac[i]);
        }
        printf("\n");
    } else {
        printf("triad get hmac fail\n");
        res = -1;
    }
    printf("end main\n");

    return res;
}