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

#ifndef PERMISSION_MANAGER_H
#define PERMISSION_MANAGER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_PERM_NAME_LEN
#define PERM_NAME_LEN CONFIG_PERM_NAME_LEN
#else
#define PERM_NAME_LEN 36
#endif

#ifdef CONFIG_PERM_DESC_LEN
#define PERM_DESC_LEN CONFIG_PERM_DESC_LEN
#else
#define PERM_DESC_LEN 32
#endif

#define PKG_NAME_LEN 64

#define PERM_SYS_APP_UID_MAX 999

#define PERM_ERROR_NOERROR 0
#define PERM_ERROR_GENERIC_ERROR -0x0001
#define PERM_ERROR_BAD_INPUT_DATA -0x0002
#define PERM_ERROR_INVALID_PERMNAME -0x0003
#define PERM_ERROR_INVALID_PATH -0x0004
#define PERM_ERROR_OPEN_DATABASE_FAIL -0x0005
#define PERM_ERROR_READ_DATABASE_FAIL -0x0006
#define PERM_ERROR_WRITE_DATABASE_FAIL -0x0007
#define PERM_ERROR_UNLINK_ERROR -0x0008
#define PERM_ERROR_RECORD_ERROR -0x0008

enum perm_stat {
    NOGRANTED = 0,
    GRANTED = 1,
};

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

typedef struct permission_info {
    char name[PERM_NAME_LEN];
    char desc[PERM_DESC_LEN];
    enum perm_stat state;
} permission_info;

#ifdef CONFIG_PERM_RECORD
typedef struct permission_record {
    char pkgname[PKG_NAME_LEN];
    char permname[PERM_NAME_LEN];
    uint64_t recordtime;
} permission_record;
#endif

#ifdef CONFIG_PERM_NOTIFY
typedef int (*perm_notify_callback)(const char* package_name,
    permission_info* permissionInfo,
    pid_t pid);
#endif

/****************************************************************************
 * Public Functions Definitions
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int query_permission(const char* package_name, permission_info* info,
    int* permissionsNum);

int update_permission(const char* package_name, const permission_info* info);

int check_permission(const char* package_name, permission_info* info, int uid);

int save_permissions(const char* package_name, const permission_info* info,
    int perm_num);

int delete_permissions(const char* package_name);

#ifdef CONFIG_PERM_RECORD
int save_perm_record(void);

int load_perm_record(void);

int get_perm_record(permission_record* record, int* num);

int record_permission_event(const char* package_name, const char* perm_name);
#endif

#ifdef CONFIG_PERM_NOTIFY
void perm_notify_registry(perm_notify_callback notify_cb);
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* PERMISSION_MANAGER_H */
