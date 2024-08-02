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

#include <wxcodepay_ca_api.h>

static uint8_t buffer[512];
static uint32_t len;

static void usage(void)
{
    printf("usage:\n"
           "\tca_wxcodepay_test check item_num\n"
           "\tca_wxcodepay_test delete item_num\n"
           "\tca_wxcodepay_test read item_num\n"
           "\tca_wxcodepay_test write item_num item_content\n"
           "\tmax item_num is 10, example: ca_wxcodepay_test write 6 test_content\n");
}

int main(int argc, FAR char* argv[])
{
    if (argc != 3 && argc != 4) {
        printf("Invalid argument number\n");
        usage();
        return -1;
    }

    int item = atoi(argv[2]);
    if (item > 10) {
        printf("Invalid item number: %d\n", item);
        usage();
        return -1;
    }

    if (argc == 3 && strcmp(argv[1], "check") == 0) {
        if (is_wxcodepay_tee_data_exited(item) == true) {
            printf("item is exited.\n");
        } else {
            printf("item is not exited.\n");
        }
    } else if (argc == 3 && strcmp(argv[1], "delete") == 0) {
        if (wxcodepay_tee_data_delete(item) == 0) {
            printf("item del successfully.\n");
        } else {
            printf("item del fail.\n");
        }
    } else if (argc == 3 && strcmp(argv[1], "read") == 0) {
        len = 512;
        memset(buffer, 0, 512);

        if (wxcodepay_tee_data_read(item, buffer, &len) == 0) {
            printf("item read successfully. len = %" PRId32 "\n", len);
            printf("item:%s\n", buffer);
        } else {
            printf("item read fail.\n");
        }
    } else if (argc == 4 && strcmp(argv[1], "write") == 0) {
        if (wxcodepay_tee_data_write(item, (const uint8_t*)argv[3], strlen(argv[3])) == 0) {
            printf("item write successfully.\n");
        } else {
            printf("item write fail.\n");
        }
    } else {
        printf("Unrecognized option: %s\n", argv[1]);
        usage();
        return -1;
    }
    return 0;
}
