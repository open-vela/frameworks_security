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

#ifndef SET_MODEL_API_H_
#define SET_MODEL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Writes product model information and CRC to a PSM (Product Specific Model) file.
 *
 * This function prepares a file path to store the product model information and its CRC.
 * It first checks if the directory specified by PSM_PATH exists, and creates it if not.
 * Then, it opens (or creates) a file named 'ot_config.psm_model' within this directory
 * for writing. The product model (PRODUCT_MODEL macro) is written to this file, followed
 * by its CRC (computed using kvs_crc8). Additionally, it sets several system properties
 * related to the product, including market name, model, Bluetooth name and PID, MiIO model,
 * and system app ID.
 *
 * @return 0 on success, -1 if an error occurs during directory creation, file opening,
 *         writing to file, or setting system properties.
 *
 * @note PRODUCT_MODEL, PSM_PATH, and other PRODUCT_* macros must be defined prior to
 *       calling this function.
 */
int psm_info_write(void);

/**
 * @brief Clears the content of the device information file.
 *
 * This function first checks the status of the directory where the device information file
 * resides (typically /data/etc). If the directory status check fails, an error message is
 * logged and the function returns with an error code.
 *
 * If the directory status check is successful, the function proceeds to open the device
 * information file (specified by DEVICE_INFO_PATH) in write-only mode. If the file cannot
 * be opened for writing, an error message is logged and the function returns with an error
 * code.
 *
 * Once the file is successfully opened, the content of the file is cleared by truncating
 * it to zero length. If truncation fails, an error message is logged, the file is closed,
 * and the function returns with an error code.
 *
 * Upon successful truncation, the file is closed, and a success message is logged.
 *
 * @return 0 on success, -1 on failure.
 *
 * @note This function assumes that DEVICE_INFO_PATH is defined and points to a valid
 *       path to the device information file.
 */
int device_info_clear(void);

/**
 * @brief Retrieves the value associated with a given key from the device information file.
 *
 * This function first checks the status of the directory where the device information
 * file resides. If the check fails, an error is logged and the function returns with
 * an error code.
 *
 * It then calls the is_key_exist() function to search for the given key in the device
 * information file. If the key is not found, an error message is logged and the
 * function returns with an error code.
 *
 * If the key is found, the function returns successfully without any further action
 * since the value (if needed) is retrieved and copied by the is_key_exist() function.
 *
 * @param key The key for which to retrieve the value.
 * @param value A pointer to a character array where the retrieved value will be stored.
 *              This parameter is ignored by this function but is used by is_key_exist().
 * @return 0 on success, -1 on failure.
 *
 * @note This function assumes that DEVICE_INFO_PATH is defined and points to a valid
 *       path to the device information file.
 */
int device_info_get(const char* key, char* value);

/**
 * @brief Sets the value for a given key in the device information file.
 *
 * This function first checks the status of the directory where the device information file
 * is located (usually /data/etc). If the directory status check fails, an error is logged
 * and the function returns with an error code.
 *
 * If the key already exists in the device information file, as determined by calling
 * is_key_exist(), an informational message is logged indicating that the key already
 * exists and the function returns successfully without making any changes.
 *
 * If the key does not exist, the function proceeds to open the device information file
 * (specified by DEVICE_INFO_PATH) in append mode with read/write capabilities ("a+").
 * If the file cannot be opened, an error is logged and the function returns with an error
 * code.
 *
 * Using fprintf(), the function attempts to write the key and its corresponding value
 * to the file in the format "key=\"value\"\n". If the write operation fails, an error
 * is logged, the file is closed, and the function returns with an error code.
 *
 * Upon successful write, the file is closed and the function returns successfully.
 *
 * @param key The key for which to set the value.
 * @param value The value to set for the given key.
 * @return 0 on success, -1 on failure.
 *
 * @note This function assumes that DEVICE_INFO_PATH is defined and points to a valid
 *       path to the device information file. It also relies on the implementation of
 *       check_dir_status() and is_key_exist() functions to perform their respective
 *       tasks correctly.
 */
int device_info_set(const char* key, const char* value);

/**
 * @brief Sets the default device information.
 *
 * This function iterates through a predefined list of device information key-value pairs
 * and sets each pair using the device_info_set function. The device information includes
 * serial number (sn), WiFi MAC address (mac_wifi), Bluetooth MAC address (mac_bt),
 * MiIO device ID (miio_did), MiIO device key (miio_key), color ID (color_id), and color
 * description (color_desc).
 *
 * @return 0 on success, -1 if any of the device information settings fail.
 *
 * @note The default values for each key are defined as macros (e.g., DEFAULT_SN_VALUE)
 *       and should be defined prior to calling this function.
 */
int default_device_info_set(void);

#ifdef __cplusplus
}
#endif

#endif
