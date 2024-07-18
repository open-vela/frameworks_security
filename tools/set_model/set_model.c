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

#define PSM_PATH CONFIG_SC_SET_MODEL_MIIO_PSM_PATH
#define PRODUCT_NAME CONFIG_SC_SET_MODEL_MIJIA_PRODUCT_NAME
#define PRODUCT_ID CONFIG_SC_SET_MODEL_MIJIA_PRODUCT_ID
#define PRODUCT_MODEL CONFIG_SC_SET_MODEL_MIJIA_PRODUCT_MODEL
#define PRODUCT_XIAOAI_APP_ID CONFIG_SC_SET_MODEL_PRODUCT_XIAOAI_APP_ID
#define PRODUCT_HARDWARE CONFIG_SC_SET_MODEL_PRODUCT_HARDWARE
#define DEVICE_INFO_PATH "/data/etc/device.info"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint8_t kvs_crc8(char* buf, uint16_t len)
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
        if (ret) {
            syslog(LOG_ERR, "can not create dir /data/etc, %d", ret);
            return -1;
        } else {
            syslog(LOG_INFO, "create /data/etc successfully!");
            return 0;
        }
    } else {
        syslog(LOG_INFO, "/data/etc is exist");
        return 0;
    }
    return -1;
}

static int device_info_set(int argc, char* argv[])
{
    char buf[128];
    FILE* p = NULL;
    int size;
    if (argc != 4) {
        return -1;
    }
    if (check_dir_status() == -1) {
        return -1;
    }
    p = fopen(DEVICE_INFO_PATH, "a+");
    if (!p) {
        syslog(LOG_ERR, "could not open %s", DEVICE_INFO_PATH);
        return -1;
    }
    /*
        argv[0] -> path
        argv[1] -> set
        argv[2] -> key
        argv[3] -> value
    */
    size = snprintf(buf, 128, "\"%s\" = \"%s\"\n", argv[2], argv[3]);
    fwrite(buf, size, 1, p);
    syslog(LOG_INFO, "`%s` is written\n", buf);
    fclose(p);
    return 0;
}

static int device_info_clear(void)
{
    FILE* p = NULL;
    if (check_dir_status() == -1) {
        syslog(LOG_ERR, "reset failed\n");
        return -1;
    }
    p = fopen(DEVICE_INFO_PATH, "w");
    if (!p) {
        syslog(LOG_ERR, "could not open %s\n", DEVICE_INFO_PATH);
        return -1;
    }
    char buf = '\0';
    fwrite(&buf, 1, 1, p);
    fclose(p);
    syslog(LOG_INFO, "%s is clear", DEVICE_INFO_PATH);
    return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char* argv[])
{
    if (argc >= 2) {
        if (strcmp(argv[1], "set") == 0) {
            return device_info_set(argc, argv);
        } else if (strcmp(argv[1], "clear") == 0) {
            return device_info_clear();
        }
    }

    int fd = -1;
    char m_model[32] = PRODUCT_MODEL;
    char psm_path[64] = { 0 };
    uint8_t crc = 0;
    int ret = 0;

    memset(psm_path, 0, sizeof(psm_path));
    if (access(PSM_PATH, F_OK)) {
        if (!mkdir(PSM_PATH, 0666)) {
            snprintf(psm_path, 64, "%s/ot_config.psm_model", PSM_PATH);
        }
    } else {
        snprintf(psm_path, 64, "%s/ot_config.psm_model", PSM_PATH);
    }

    if (strlen(psm_path)) {
        fd = open(psm_path, O_CREAT | O_RDWR | O_TRUNC, 0666);
        if (fd < 0) {
            printf("create ot_config.psm_model file error!\n");
        }
    }

    printf("mico hardware: %s, mijia model: %s, name: %s, pid: %s, xiaoai app_id: %s\n",
        PRODUCT_HARDWARE, m_model, PRODUCT_NAME,
        PRODUCT_ID, PRODUCT_XIAOAI_APP_ID);

    property_set("model", PRODUCT_HARDWARE);
    property_set("persist.bt.name", PRODUCT_NAME);
    property_set("persist.bt.pid", PRODUCT_ID);
    property_set("persist.miio.model", m_model);
    property_set("persist.sys.appid", PRODUCT_XIAOAI_APP_ID);

    if (fd >= 0) {
        ret = write(fd, m_model, strlen(m_model));
        if (ret != strlen(m_model)) {
            printf("write model failed!\n");
            goto out;
        }

        crc = kvs_crc8(m_model, strlen(m_model));
        ret = write(fd, &crc, sizeof(uint8_t));
        if (ret != sizeof(uint8_t)) {
            printf("write crc failed!\n");
        }
    out:
        close(fd);
    }

    return 0;
}
