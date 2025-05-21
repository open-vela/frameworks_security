/*
 * Copyright (C) 2022-2025 Xiaomi Corporation
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unqlite.h>
#ifdef CONFIG_PERM_RECORD
#include <sys/queue.h>
#endif

#include "perm_manager.h"

#define PERM_PATH_PREFIX "/data/permission/"
#define RECORD_KEEP_TIME(days) (days * 24 * 60 * 60)

static char g_permission_name[][PERM_NAME_LEN] = {
    "vela.permission.LOCATION",
    "vela.permission.STEP_COUNTER",
    "vela.permission.DEVICE_INFO",
    "vela.permission.RECORD",
    "vela.permission.RECORD_AUDIO",
    "vela.permission.BLUETOOTH",
    "vela.permission.READ_PHONE_STATE",
    "vela.permission.SEND_SMS",
    "vela.permission.RECEIVE_SMS",
    "vela.permission.INTERNET",
    "vela.permission.WRITE_CONTACTS",
    "vela.permission.READ_CONTACTS",
    "vela.permission.WRITE_CALL_LOG",
    "vela.permission.READ_CALL_LOG",
    "vela.permission.READ_SMS",
    "vela.permission.NFC",
    "vela.permission.CALL_PHONE",
    "vela.permission.READ_MEDIA_DATA",
    "vela.permission.WRITE_CALENDAR",
    "vela.permission.READ_CALENDAR",
    "vela.permission.READ_HEALTH_DATA",
    "vela.permission.ACCELEROMETER",
    "vela.permission.GYROSCOPE",
    "vela.permission.ACTIVITY_MOTION",
    "vela.permission.GESTURE_DETECTION",
};

#ifdef CONFIG_PERM_NOTIFY
static perm_notify_callback g_perm_notify_cb;

void perm_notify_registry(perm_notify_callback notify_cb)
{
    g_perm_notify_cb = notify_cb;
}
#endif

static int check_permission_dir(void)
{
    if (mkdir(PERM_PATH_PREFIX, 0755) == -1 && errno != EEXIST) {
        printf("create %s path failed\n", PERM_PATH_PREFIX);
        return PERM_ERROR_GENERIC_ERROR;
    }

    return PERM_ERROR_NOERROR;
}

#ifdef CONFIG_PERM_RECORD
struct permlog {
    TAILQ_ENTRY(permlog)
    next;
    permission_record record;
};

struct permlog_queue {
    TAILQ_HEAD(permloglist, permlog)
    queue;
    int logn;
    pthread_mutex_t mutex;
};

static struct permlog_queue g_permlog = {
    .queue = TAILQ_HEAD_INITIALIZER(g_permlog.queue),
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .logn = 0,
};

int save_perm_record(void)
{
    char temp_filename[64];
    char perm_filename[64];
    char message[CONFIG_PERM_RECORD_LOG_LEN];
    struct permlog* log;

    snprintf(temp_filename, sizeof(temp_filename), "%s%s", PERM_PATH_PREFIX, "tmp.log");
    FILE* file = fopen(temp_filename, "a");
    if (file == NULL) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    TAILQ_FOREACH(log, &g_permlog.queue, next)
    {
        memset(message, 0, CONFIG_PERM_RECORD_LOG_LEN);
        snprintf(message, CONFIG_PERM_RECORD_LOG_LEN, "%lld-%.*s-%.*s\n",
            log->record.recordtime, PKG_NAME_LEN, log->record.pkgname,
            PERM_NAME_LEN, log->record.permname);
        fputs(message, file);
    }

    fclose(file);

    snprintf(perm_filename, sizeof(perm_filename), "%s%s", PERM_PATH_PREFIX, CONFIG_PERM_RECORD_FILE);
    rename(temp_filename, perm_filename);
    return PERM_ERROR_NOERROR;
}

int load_perm_record(void)
{
    char perm_filename[64];
    char* line;
    struct permlog* log;
    int ret = PERM_ERROR_NOERROR;

    if (check_permission_dir()) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    if (g_permlog.logn != 0) {
        return PERM_ERROR_NOERROR;
    }

    snprintf(perm_filename, sizeof(perm_filename), "%s%s", PERM_PATH_PREFIX, CONFIG_PERM_RECORD_FILE);
    FILE* file = fopen(perm_filename, "r");
    if (file == NULL) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    line = (char*)malloc(CONFIG_PERM_RECORD_LOG_LEN);
    if (line == NULL) {
        fclose(file);
        return PERM_ERROR_GENERIC_ERROR;
    }

    pthread_mutex_lock(&g_permlog.mutex);
    while (fgets(line, CONFIG_PERM_RECORD_LOG_LEN, file)) {
        log = malloc(sizeof(struct permlog));
        if (log == NULL) {
            ret = PERM_ERROR_GENERIC_ERROR;
            goto exit;
        }

        size_t rest;
        memset(log, 0, sizeof(struct permlog));
        if (sscanf(line, "%llu-%[^-]%n", &log->record.recordtime, log->record.pkgname, &rest) == 2) {
            strcpy(log->record.permname, line + rest + 1);
            log->record.permname[strlen(log->record.permname) - 1] = '\0';
            TAILQ_INSERT_TAIL(&g_permlog.queue, log, next);
            g_permlog.logn++;
        } else {
            free(log);
            break;
        }
    }

exit:
    pthread_mutex_unlock(&g_permlog.mutex);
    free(line);
    fclose(file);
    return ret;
}

int get_perm_record(permission_record* record, int* num)
{
    struct permlog* log;
    int i = 0;

    if (record == NULL) {
        *num = g_permlog.logn;
        return PERM_ERROR_NOERROR;
    }

    pthread_mutex_lock(&g_permlog.mutex);
    TAILQ_FOREACH_REVERSE(log, &g_permlog.queue, permloglist, next)
    {
        record[i].recordtime = log->record.recordtime;
        strncpy(record[i].pkgname, log->record.pkgname, PKG_NAME_LEN);
        strncpy(record[i].permname, log->record.permname, PERM_NAME_LEN);
        i++;
    }

    pthread_mutex_unlock(&g_permlog.mutex);
    return PERM_ERROR_NOERROR;
}

static int add_perm_record(struct permlog* log)
{
    pthread_mutex_lock(&g_permlog.mutex);
    TAILQ_INSERT_TAIL(&g_permlog.queue, log, next);
    g_permlog.logn++;

    if (g_permlog.logn > CONFIG_PERM_RECORD_MAX_COUNT) {
        struct permlog* head = TAILQ_FIRST(&g_permlog.queue);
        TAILQ_REMOVE(&g_permlog.queue, head, next);
        free(head);
        g_permlog.logn--;
    }

    pthread_mutex_unlock(&g_permlog.mutex);
    return PERM_ERROR_NOERROR;
}

static void retotal_perm_record(int days)
{
    struct permlog* log;
    time_t curtime;
    time(&curtime);

    pthread_mutex_lock(&g_permlog.mutex);
    while ((log = TAILQ_FIRST(&g_permlog.queue)) != NULL) {
        if (difftime(curtime, log->record.recordtime) <= RECORD_KEEP_TIME(days)) {
            break;
        }

        TAILQ_REMOVE(&g_permlog.queue, log, next);
        free(log);
        g_permlog.logn--;
    }

    pthread_mutex_unlock(&g_permlog.mutex);
}

int record_permission_event(const char* package_name, const char* perm_name)
{
    time_t rawtime;
    struct permlog* log;

    retotal_perm_record(CONFIG_PERM_RECORD_KEEP_DAYS);

    log = malloc(sizeof(struct permlog));
    if (log == NULL) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    memset(log, 0, sizeof(struct permlog));
    time(&rawtime);

    log->record.recordtime = rawtime;
    strncpy(log->record.pkgname, package_name, PKG_NAME_LEN);
    strncpy(log->record.permname, perm_name, PERM_NAME_LEN);

    add_perm_record(log);
    return PERM_ERROR_NOERROR;
}
#endif

static int data_consumer_callback(const void* pData, unsigned int nDatalen, void* pUserData)
{
    int* state = (int*)pUserData;
    char* data = (char*)zalloc(nDatalen + 1);
    if (data == NULL) {
        return UNQLITE_NOMEM;
    }

    memcpy(data, pData, nDatalen);
    *state = atoi(data);
    free(data);
    return UNQLITE_OK;
}

static int get_path(const char* package_name, char* path)
{
    if (check_permission_dir()) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    memset(path, 0, PATH_MAX);
    strcpy(path, PERM_PATH_PREFIX);
    strcat(path, package_name);
    return PERM_ERROR_NOERROR;
}

static int check_permission_valid(const char* perm_name)
{
    int perm_num = sizeof(g_permission_name) / PERM_NAME_LEN;
    int i;

    for (i = 0; i < perm_num; i++) {
        if (strcmp(perm_name, g_permission_name[i]) == 0) {
            return PERM_ERROR_NOERROR;
        }
    }

    return PERM_ERROR_INVALID_PERMNAME;
}

int query_permission(const char* package_name, permission_info* info,
    int* permissionsNum)
{
    if (package_name == NULL) {
        return PERM_ERROR_BAD_INPUT_DATA;
    }

    unqlite* pDb;
    unqlite_kv_cursor* pCur;
    char database[PATH_MAX];
    int ret = PERM_ERROR_NOERROR;

    if (get_path(package_name, database)) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    if (unqlite_open(&pDb, database, UNQLITE_OPEN_READONLY) != UNQLITE_OK) {
        return PERM_ERROR_OPEN_DATABASE_FAIL;
    }

    if (unqlite_kv_cursor_init(pDb, &pCur) != UNQLITE_OK) {
        unqlite_close(pDb);
        return PERM_ERROR_GENERIC_ERROR;
    }

    unqlite_kv_cursor_first_entry(pCur);
    if (info == NULL) {
        *permissionsNum = 0;
        while (unqlite_kv_cursor_valid_entry(pCur)) {
            (*permissionsNum)++;
            unqlite_kv_cursor_next_entry(pCur);
        }
    } else {
        int cnt = 0;
        while (unqlite_kv_cursor_valid_entry(pCur) && cnt < (*permissionsNum)) {
            int keylen = PERM_NAME_LEN;
            int data;
            unqlite_kv_cursor_key(pCur, info[cnt].name, &keylen);
            unqlite_kv_cursor_data_callback(pCur, data_consumer_callback, &data);
            info[cnt].state = data;
            unqlite_kv_cursor_next_entry(pCur);
            cnt++;
        }
    }

    unqlite_kv_cursor_release(pDb, pCur);
    unqlite_close(pDb);
    return ret;
}

int update_permission(const char* package_name, const permission_info* info)
{
    if (package_name == NULL || info == NULL) {
        return PERM_ERROR_BAD_INPUT_DATA;
    }

    unqlite* pDb;
    unqlite_kv_cursor* pCur;
    char database[PATH_MAX];
    int ret = PERM_ERROR_NOERROR;

    if (check_permission_valid(info->name) != PERM_ERROR_NOERROR) {
        return PERM_ERROR_INVALID_PERMNAME;
    }

    if (get_path(package_name, database)) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    if (unqlite_open(&pDb, database, UNQLITE_OPEN_READWRITE) != UNQLITE_OK) {
        return PERM_ERROR_OPEN_DATABASE_FAIL;
    }

    if (unqlite_kv_cursor_init(pDb, &pCur) != UNQLITE_OK) {
        unqlite_close(pDb);
        return PERM_ERROR_GENERIC_ERROR;
    }

    unqlite_kv_cursor_first_entry(pCur);
    while (unqlite_kv_cursor_valid_entry(pCur)) {
        int keylen = PERM_NAME_LEN;
        char key[PERM_NAME_LEN];

        unqlite_kv_cursor_key(pCur, key, &keylen);
        if (memcmp(key, info->name, keylen) == 0) {
            if (unqlite_kv_store_fmt(pDb, info->name, -1, "%d", info->state) != UNQLITE_OK) {
                ret = PERM_ERROR_WRITE_DATABASE_FAIL;
            }
            break;
        }

        unqlite_kv_cursor_next_entry(pCur);
    }

    unqlite_kv_cursor_release(pDb, pCur);
    unqlite_close(pDb);
    return ret;
}

int check_permission(const char* package_name, permission_info* info, int uid)
{
    if (package_name == NULL || info == NULL) {
        return PERM_ERROR_BAD_INPUT_DATA;
    }

    unqlite* pDb;
    unqlite_kv_cursor* pCur;
    char database[PATH_MAX];
    int ret = PERM_ERROR_NOERROR;

#ifdef CONFIG_PERM_NOTIFY
    if (g_perm_notify_cb && uid > PERM_SYS_APP_UID_MAX && strcmp(info->name, "vela.permission.INTERNET") == 0) {
        info->state = GRANTED;
        g_perm_notify_cb(package_name, info, getpid());
        return PERM_ERROR_NOERROR;
    }
#endif

    if (check_permission_valid(info->name) != PERM_ERROR_NOERROR) {
        return PERM_ERROR_INVALID_PERMNAME;
    }

#ifdef CONFIG_PERM_NOTIFY
    if (g_perm_notify_cb) {
        g_perm_notify_cb(package_name, info, getpid());
    }
#endif

    if (uid <= PERM_SYS_APP_UID_MAX) {
        info->state = GRANTED;
        return PERM_ERROR_NOERROR;
    }

    if (get_path(package_name, database)) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    if (unqlite_open(&pDb, database, UNQLITE_OPEN_READONLY) != UNQLITE_OK) {
        return PERM_ERROR_OPEN_DATABASE_FAIL;
    }

    if (unqlite_kv_cursor_init(pDb, &pCur) != UNQLITE_OK) {
        unqlite_close(pDb);
        return PERM_ERROR_GENERIC_ERROR;
    }

    unqlite_kv_cursor_first_entry(pCur);
    while (unqlite_kv_cursor_valid_entry(pCur)) {
        int keylen = PERM_NAME_LEN;
        char key[PERM_NAME_LEN];
        int data;

        unqlite_kv_cursor_key(pCur, key, &keylen);
        if (memcmp(key, info->name, keylen) == 0) {
            unqlite_kv_cursor_data_callback(pCur, data_consumer_callback, &data);
            info->state = data;
            break;
        }

        unqlite_kv_cursor_next_entry(pCur);
    }

    if (!unqlite_kv_cursor_valid_entry(pCur)) {
        ret = PERM_ERROR_INVALID_PERMNAME;
    }

    unqlite_kv_cursor_release(pDb, pCur);
    unqlite_close(pDb);
    return ret;
}

int save_permissions(const char* package_name, const permission_info* info,
    int perm_num)
{
    if (package_name == NULL || info == NULL || perm_num <= 0) {
        return PERM_ERROR_BAD_INPUT_DATA;
    }

    unqlite* pDb;
    char database[PATH_MAX];
    int i;
    int ret = PERM_ERROR_NOERROR;

    if (get_path(package_name, database)) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    if (unqlite_open(&pDb, database, UNQLITE_OPEN_CREATE | UNQLITE_OPEN_READWRITE) != UNQLITE_OK) {
        return PERM_ERROR_OPEN_DATABASE_FAIL;
    }

    for (i = 0; i < perm_num; i++) {
        if (check_permission_valid(info[i].name) != PERM_ERROR_INVALID_PERMNAME) {
            if (unqlite_kv_store_fmt(pDb, info[i].name, -1, "%d", info[i].state) != UNQLITE_OK) {
                ret = PERM_ERROR_WRITE_DATABASE_FAIL;
                break;
            }
        }
    }

    unqlite_close(pDb);
    return ret;
}

int delete_permissions(const char* package_name)
{
    char database[PATH_MAX];

    if (get_path(package_name, database)) {
        return PERM_ERROR_GENERIC_ERROR;
    }

    return remove(database) == 0 ? PERM_ERROR_NOERROR : PERM_ERROR_UNLINK_ERROR;
}
