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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <fcntl.h>
#include <set_model_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>

#include <kvdb.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* These length is inherited from previous projects.*/
#define PRODUCT_SN_LEN 15
#define PRODUCT_MAC_WIFI_LEN 17
#define PRODUCT_MAC_BT_LEN 17
#define PRODUCT_MIIO_DID_LEN 9
#define PRODUCT_MIIO_KEY_LEN 16
#define PRODUCT_COLOR_ID_LEN 1
#define PRODUCT_COLOR_DESC_LEN 15

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static void usage(void)
{
    syslog(LOG_INFO, "set_model usage:\n"
                     "\tTo set model properties, use the following syntax:\n"
                     "\t\tset_model set <property> <value>\n"
                     "\tAvailable properties and their values:\n"
                     "\t\tsn: <serial_number> (length: 15 characters max)\n"
                     "\t\tmac_wifi: <MAC_address> (length: 17 characters)\n"
                     "\t\tmac_bt: <MAC_address> (length: 17 characters)\n"
                     "\t\tmiio_did: <device_id> (length: 9 characters)\n"
                     "\t\tmiio_key: <device_key> (length: 16 characters)\n"
                     "\t\tcolor_id: <color_id> (length: 1 characters)\n"
                     "\t\tcolor_desc: <color_desc> (length: 15 characters)\n"
                     "\tExample:\n"
                     "\t\tset_model set sn 55119/F3YN00102\n"
                     "\t\tset_model set mac_wifi CC:D8:43:20:C4:22\n"
                     "\t\tset_model set mac_bt CC:D8:43:20:C4:22\n"
                     "\t\tset_model set miio_did 771897593\n"
                     "\t\tset_model set miio_key 0000000000000001\n"
                     "\t\tset_model set color_id 0\n"
                     "\t\tset_model set color_desc 000000000000000\n"
                     "\tTo set the PSM model and properties, use:\n"
                     "\t\tset_model setpsm\n"
                     "\tTo reset the set model properties, use:\n"
                     "\t\tset_model reset\n"
                     "\tTo get the all model properties and values, use:\n"
                     "\t\tset_model get sn\n"
                     "\t\tset_model get mac_bt\n"
                     "\t\tset_model get miio_did\n"
                     "\t\tset_model get miio_key\n"
                     "\t\tset_model get color_id\n"
                     "\t\tset_model get color_desc\n");
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char* argv[])
{
    if (argc != 1 && argc != 2 && argc != 3 && argc != 4) {
        syslog(LOG_ERR, "Invalid argument number\n");
        usage();
        return -1;
    }

    if (argc == 4 && strcmp(argv[1], "set") == 0) {
        if (strcmp(argv[2], "sn") == 0 && strlen(argv[3]) == PRODUCT_SN_LEN) {
        } else if (strcmp(argv[2], "mac_wifi") == 0 && strlen(argv[3]) == PRODUCT_MAC_WIFI_LEN) {
        } else if (strcmp(argv[2], "mac_bt") == 0 && strlen(argv[3]) == PRODUCT_MAC_BT_LEN) {
        } else if (strcmp(argv[2], "miio_did") == 0 && strlen(argv[3]) == PRODUCT_MIIO_DID_LEN) {
        } else if (strcmp(argv[2], "miio_key") == 0 && strlen(argv[3]) == PRODUCT_MIIO_KEY_LEN) {
        } else if (strcmp(argv[2], "color_id") == 0 && strlen(argv[3]) == PRODUCT_COLOR_ID_LEN) {
        } else if (strcmp(argv[2], "color_desc") == 0 && strlen(argv[3]) == PRODUCT_COLOR_DESC_LEN) {
        } else {
            syslog(LOG_ERR, "Unrecognized property or wrong value length\n");
            usage();
            return -1;
        }
        if (device_info_set(argv[2], argv[3]) != 0) {
            syslog(LOG_ERR, "Set %s=%s fail\n", argv[2], argv[3]);
            return -1;
        }
        syslog(LOG_INFO, "Set %s=%s success\n", argv[2], argv[3]);
    } else if (argc == 2 && strcmp(argv[1], "reset") == 0) {
        if (device_info_clear() != 0) {
            syslog(LOG_ERR, "Reset /data/etc/device.info fail\n");
            return -1;
        }
        syslog(LOG_INFO, "Reset /data/etc/device.info success\n");
    } else if (argc == 3 && strcmp(argv[1], "get") == 0) {
        char value[24];
        memset(value, 0, sizeof(value));
        if (device_info_get(argv[2], value) != 0) {
            syslog(LOG_ERR, "Getting /data/etc/device.info fail\n");
            return -1;
        }
        syslog(LOG_INFO, "get %s = %s from /data/etc/device.info success\n", argv[2], value);
    } else if (argc == 2 && strcmp(argv[1], "setpsm") == 0) {
        if (psm_info_write() != 0) {
            syslog(LOG_ERR, "Writing psm path or set property fail\n");
            return -1;
        }
        syslog(LOG_INFO, "Writing psm path and set property success\n");
    } else if (argc == 2 && strcmp(argv[1], "setdefault") == 0) {
        if (default_device_info_set() != 0) {
            syslog(LOG_ERR, "Set default device info fail\n");
            return -1;
        }
    } else {
        syslog(LOG_ERR, "Unrecognized option: %s\n", argv[1]);
        usage();
        return -1;
    }

    return 0;
}
