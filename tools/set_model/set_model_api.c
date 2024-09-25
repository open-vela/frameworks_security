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
#define DEVICE_INFO_PATH CONFIG_SC_SET_MODEL_DEVICE_INFO_PATH

#define DEFAULT_SN_VALUE CONFIG_SC_SET_MODEL_DEFAULT_SN_VALUE
#define DEFAULT_MAC_WIFI_VALUE CONFIG_SC_SET_MODEL_DEFAULT_MAC_WIFI_VALUE
#define DEFAULT_MAC_BT_VALUE CONFIG_SC_SET_MODEL_DEFAULT_MAC_BT_VALUE
#define DEFAULT_MIIO_DID_VALUE CONFIG_SC_SET_MODEL_DEFAULT_MIIO_DID_VALUE
#define DEFAULT_MIIO_KEY_VALUE CONFIG_SC_SET_MODEL_DEFAULT_MIIO_KEY_VALUE
#define DEFAULT_COLOR_ID_VALUE CONFIG_SC_SET_MODEL_DEFAULT_COLOR_ID_VALUE
#define DEFAULT_COLOR_DESC_VALUE CONFIG_SC_SET_MODEL_DEFAULT_COLOR_DESC_VALUE

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint8_t kvs_crc8(const char* line, uint16_t len)
{
    uint8_t crc = 0x00;
    uint8_t i = 0;

    while (len--) {
        crc ^= *line++;
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
        int ret = mkdir("/data/etc", 0755);
        if (ret != 0) {
            syslog(LOG_ERR, "can not create dir /data/etc\n");
            return -1;
        }
        syslog(LOG_INFO, "create /data/etc successfully!");
    }

    return 0;
}

static int is_key_exist(const char* path, const char* key, char* value, bool need_get_value, bool* found)
{
    FILE* file = fopen(path, "r");
    if (!file) {
        syslog(LOG_ERR, "Could not open %s\n", path);
        return -1;
    }
    char line[64];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char* read_key = strtok(line, "=");
        if (!read_key)
            continue;
        if (read_key && strcmp(read_key, key) == 0) {
            if (value) {
                char* read_value = strtok(NULL, "\n");
                if (!read_value)
                    continue;
                int read_length = strlen(read_value);
                if (read_length > 0 && read_value[0] == '"' && read_value[read_length - 1] == '"') {
                    read_value[read_length - 1] = '\0';
                    read_value++;
                }
                read_length = strlen(read_value);
                if (need_get_value) {
                    strncpy(value, read_value, read_length);
                }
            }
            *found = true;
            break;
        }
    }

    if (found == false && need_get_value == true) {
        value = NULL;
    }

    fclose(file);
    return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int device_info_get(const char* key, char* value)
{
    if (check_dir_status() != 0) {
        syslog(LOG_ERR, "Check /data/etc status fail\n");
        return -1;
    }

    bool found = false;
    bool need_get_value = true;
    if (is_key_exist(DEVICE_INFO_PATH, key, value, need_get_value, &found) == 0 && found == false) {
        syslog(LOG_ERR, "Key %s not found in %s\n", key, DEVICE_INFO_PATH);
        return -1;
    }

    return 0;
}

int device_info_set(const char* key, const char* value)
{
    if (check_dir_status() != 0) {
        syslog(LOG_ERR, "Check /data/etc status fail\n");
        return -1;
    }

    bool found = false;
    bool need_get_value = false;
    if (is_key_exist(DEVICE_INFO_PATH, key, (char*)value, need_get_value, &found) == 0 && found == true) {
        if (found == true) {
            syslog(LOG_INFO, "%s already exists in %s\n", key, DEVICE_INFO_PATH);
            return 0;
        }
    } else {
        FILE* p = fopen(DEVICE_INFO_PATH, "a+");
        if (!p) {
            syslog(LOG_ERR, "Could not open %s\n", DEVICE_INFO_PATH);
            return -1;
        }

        if (fprintf(p, "%s=\"%s\"\n", key, value) < 0) {
            syslog(LOG_ERR, "Failed to write %s=%s to %s\n", key, value, DEVICE_INFO_PATH);
            fclose(p);
            return -1;
        }
        fclose(p);
    }

    return 0;
}

int device_info_clear(void)
{
    if (check_dir_status() != 0) {
        syslog(LOG_ERR, "Check /data/etc status fail\n");
        return -1;
    }
    syslog(LOG_INFO, "Check /data/etc success\n");

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

int psm_info_write(void)
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

int default_device_info_set(void)
{
    struct {
        const char* key;
        const char* value;
    } default_device_info[] = {
        { "sn", DEFAULT_SN_VALUE },
        { "mac_wifi", DEFAULT_MAC_WIFI_VALUE },
        { "mac_bt", DEFAULT_MAC_BT_VALUE },
        { "miio_did", DEFAULT_MIIO_DID_VALUE },
        { "miio_key", DEFAULT_MIIO_KEY_VALUE },
        { "color_id", DEFAULT_COLOR_ID_VALUE },
        { "color_desc", DEFAULT_COLOR_DESC_VALUE },
    };

    for (int i = 0; i < sizeof(default_device_info) / sizeof(default_device_info[0]); i++) {
        if (device_info_set(default_device_info[i].key, default_device_info[i].value) != 0) {
            syslog(LOG_ERR, "Set %s=%s fail\n", default_device_info[i].key, default_device_info[i].value);
            return -1;
        }
    }

    return 0;
}
