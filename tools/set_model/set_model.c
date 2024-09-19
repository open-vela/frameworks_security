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

#define PRODUCT_MARKET_NAME CONFIG_SC_SET_MODEL_PRODUCT_MARKET_NAME
#define PSM_PATH CONFIG_SC_SET_MODEL_MIIO_PSM_PATH
#define PRODUCT_NAME CONFIG_SC_SET_MODEL_PRODUCT_NAME
#define PRODUCT_ID CONFIG_SC_SET_MODEL_PRODUCT_ID
#define PRODUCT_MODEL CONFIG_SC_SET_MODEL_PRODUCT_MODEL
#define PRODUCT_APP_ID CONFIG_SC_SET_MODEL_PRODUCT_APP_ID
#define PRODUCT_HARDWARE CONFIG_SC_SET_MODEL_PRODUCT_HARDWARE
#define DEVICE_INFO_PATH "/data/etc/device.info"

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
                     "\tset_model set <property> <value>\n"
                     "\tAvailable properties and their values:\n"
                     "\t\tsn: <serial_number> (length: 15 characters max)\n"
                     "\t\tmac_wifi: <MAC_address> (length: 17 characters)\n"
                     "\t\tmac_bt: <MAC_address> (length: 17 characters)\n"
                     "\t\tmiio_did: <device_id> (length: 9 characters)\n"
                     "\t\tmiio_key: <device_key> (length: 16 characters)\n"
                     "\t\tcolor_id: <color_id> (length: 1 characters)\n"
                     "\t\tcolor_desc: <color_desc> (length: 15 characters)\n"
                     "\t\tExample:\n"
                     "\t\tset_model set sn 55119/F3YN00102\n"
                     "\t\tset_model set mac_wifi CC:D8:43:20:C4:22\n"
                     "\t\tset_model set mac_bt CC:D8:43:20:C4:22\n"
                     "\t\tset_model set miio_did 771897593\n"
                     "\t\tset_model set miio_key 0000000000000001\n"
                     "\t\tset_model set color_id 0\n"
                     "\t\tset_model set color_desc 000000000000000\n"
                     "\tTo set the PSM model and properties, use:\n"
                     "\tset_model setpsm\n"
                     "\tTo reset the set model properties, use:\n"
                     "\tset_model reset\n"
                     "\tTo get the all model properties and values, use:\n"
                     "\tset_model get\n");
}

static uint8_t kvs_crc8(const char* buf, uint16_t len)
{
    uint8_t crc = 0x00;
    uint8_t i = 0;

    while (len--) {
        crc ^= *buf++;
        for (i = 8; i > 0; i--) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }

    return crc;
}

static int check_dir_status(void)
{
    if (access("/data/etc", F_OK)) {
        int ret = mkdir("/data/etc", 0644);
        if (ret != 0) {
            syslog(LOG_ERR, "can not create dir /data/etc\n");
            return -1;
        }
        syslog(LOG_INFO, "create /data/etc successfully!");
    }
    syslog(LOG_INFO, "/data/etc is exist\n");

    return 0;
}

static int device_info_get(void)
{
    FILE* p = fopen(DEVICE_INFO_PATH, "r");
    if (!p) {
        syslog(LOG_ERR, "Could not open %s for reading\n", DEVICE_INFO_PATH);
        return -1;
    }

    char buf[128];
    syslog(LOG_INFO, "Here getting content from %s:\n", DEVICE_INFO_PATH);
    while (fgets(buf, sizeof(buf), p) != NULL) {
        syslog(LOG_INFO, "%s", buf);
    }

    fclose(p);
    return 0;
}

static bool is_key_exist(const char* path, const char* key)
{
    FILE* file = fopen(path, "r");
    if (!file) {
        syslog(LOG_ERR, "Could not open %s\n", path);
        return false;
    }

    char line[32];
    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, "=");
        if (token && strcmp(token, key) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

static int device_info_set(char* argv[])
{
    if (is_key_exist(DEVICE_INFO_PATH, argv[2])) {
        syslog(LOG_INFO, "Key %s already exists in %s\n", argv[2], DEVICE_INFO_PATH);
        return 0;
    }

    FILE* p = fopen(DEVICE_INFO_PATH, "a+");
    if (!p) {
        syslog(LOG_ERR, "Could not open %s\n", DEVICE_INFO_PATH);
        return -1;
    }

    if (fprintf(p, "%s=%s\n", argv[2], argv[3]) < 0) {
        syslog(LOG_ERR, "Failed to write %s=%s to %s\n", argv[2], argv[3], DEVICE_INFO_PATH);
        fclose(p);
        return -1;
    }

    fclose(p);
    return 0;
}

static int device_info_clear(void)
{
    int fd = open(DEVICE_INFO_PATH, O_WRONLY);
    if (fd == -1) {
        syslog(LOG_ERR, "Could not open %s for writing\n", DEVICE_INFO_PATH);
        return -1;
    }

    if (ftruncate(fd, 0) == -1) {
        syslog(LOG_ERR, "failed to truncate %s\n", DEVICE_INFO_PATH);
        close(fd);
        return -1;
    }

    close(fd);
    syslog(LOG_INFO, "%s is clear\n", DEVICE_INFO_PATH);
    return 0;
}

static int psm_info_write(void)
{
    int fd = -1;
    const char m_model[32] = PRODUCT_MODEL;
    char psm_path[64] = { 0 };
    uint8_t crc = 0;
    int ret = 0;

    memset(psm_path, 0, sizeof(psm_path));
    if (access(PSM_PATH, F_OK)) {
        if (mkdir(PSM_PATH, 0666) != 0) {
            syslog(LOG_ERR, "Failed to create directory %s\n", PSM_PATH);
            return -1;
        }
    }
    snprintf(psm_path, sizeof(psm_path), "%s/ot_config.psm_model", PSM_PATH);

    fd = open(psm_path, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) {
        syslog(LOG_ERR, "Failed to create %s file\n", psm_path);
        return -1;
    }

    ret = write(fd, m_model, strlen(m_model));
    if (ret != strlen(m_model)) {
        syslog(LOG_ERR, "Write %s into %s failed\n", m_model, psm_path);
        close(fd);
        return -1;
    }

    crc = kvs_crc8(m_model, strlen(m_model));
    ret = write(fd, &crc, sizeof(uint8_t));
    if (ret != sizeof(uint8_t)) {
        syslog(LOG_ERR, "Write crc failed\n");
        close(fd);
        return -1;
    }
    syslog(LOG_INFO, "write %s into %s success\n", m_model, psm_path);

    property_set("market_name", PRODUCT_MARKET_NAME);
    property_set("model", PRODUCT_HARDWARE);
    property_set("persist.bt.name", PRODUCT_NAME);
    property_set("persist.bt.pid", PRODUCT_ID);
    property_set("persist.miio.model", PRODUCT_MODEL);
    property_set("persist.sys.appid", PRODUCT_APP_ID);
    syslog(LOG_INFO, "Set property as follows success :\n"
                     "\tmarket_name:%s\n"
                     "\tmodel: %s\n"
                     "\tpersist.miio.model: %s\n"
                     "\tpersist.bt.name: %s\n"
                     "\tpersist.bt.pid: %s\n"
                     "\tpersist.sys.appid: %s\n",
        PRODUCT_MARKET_NAME, PRODUCT_HARDWARE, m_model, PRODUCT_NAME,
        PRODUCT_ID, PRODUCT_APP_ID);

    return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char* argv[])
{
    if (argc != 1 && argc != 2 && argc != 4) {
        syslog(LOG_ERR, "Invalid argument number\n");
        usage();
        return -1;
    }

    if (check_dir_status() != 0) {
        syslog(LOG_ERR, "Check /data/etc status fail\n");
        return -1;
    }
    syslog(LOG_INFO, "check directory success\n");

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
        if (device_info_set(argv) != 0) {
            syslog(LOG_ERR, "Set %s fail\n", argv[2]);
            return -1;
        }
        syslog(LOG_INFO, "Set %s=%s success\n", argv[2], argv[3]);
    } else if (argc == 2 && strcmp(argv[1], "reset") == 0) {
        if (device_info_clear() != 0) {
            syslog(LOG_ERR, "Reset /data/etc/device.info fail\n");
            return -1;
        }
        syslog(LOG_INFO, "Reset /data/etc/device.info success\n");
    } else if (argc == 2 && strcmp(argv[1], "get") == 0) {
        if (device_info_get() != 0) {
            syslog(LOG_ERR, "Getting /data/etc/device.info fail\n");
            return -1;
        }
        syslog(LOG_INFO, "get /data/etc/device.info success\n");
    } else if (argc == 2 && strcmp(argv[1], "setpsm") == 0) {
        if (psm_info_write() != 0) {
            syslog(LOG_ERR, "Writing psm path or set property fail\n");
            return -1;
        }
        syslog(LOG_INFO, "Writing psm path and set property success\n");
    } else {
        syslog(LOG_ERR, "Unrecognized option: %s\n", argv[1]);
        usage();
        return -1;
    }

    return 0;
}
