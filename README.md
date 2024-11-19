# Security

[[English](./README.md) | [中文](./README_zh-cn.md)]

## Project Overview

The current project mainly includes the implementation of `TA`, `CA` and `set_model` tools in `openvela`.

Among them, `CA/TA` is implemented based on the standard [GP API](https://globalplatform.org/specs-library/tee-internal-core-api-specification). If our current device supports `TEE`, then we call `openvela` running in `TEE` `Vela TEE`, and `openvela` running in a normal environment `Vela AP`.

Among them, `CA` runs in `Vela AP`, and `TA` runs in `Vela TEE`.
The overall communication process between `CA` and `TA` in `openvela` is as follows:

```log
+-------------+ +---------------+
|[Vela AP] | |[Vela TEE] |
| | | |
| CA | | TA |
| | | | /|\ |
| \|/ | | | |
| LIB_TEEC | | TA MANAGER |
| | | | /|\ |
| \|/ | rpmsg socket | | |
| /dev/tee0 <----------------> opteed server |
|_____________| |_______________|
```

## Project Description

### CA

### 1 CA

1. comsst CA

    `comsst CA` is a `CA` program for communicating with `comsst TA`, which includes the input, read, verify and delete operations of `comsst`.

    `comsst CA` itself is a complete `CA` program, but users can also choose to define their own logic based on the [API](include/comsst_ca_api.h) provided by `comsst CA` for secondary development.

2. pin CA

    `pin CA` is a `CA` program for communicating with `pin TA`, which includes the acquisition, storage, deletion and verification operations of `pin`.

    `pin CA` itself is a complete `CA` program, but users can also choose to define their own logic based on the [API](include/pin_ca_api.h) provided by `pin CA` for secondary development.

3. triad CA

    `triad CA` is a `CA` program used to communicate with `triad TA`, which includes the acquisition, deletion and update operations of the device `key`, `did` and `did hmac`.
    `triad CA` itself is a complete `CA` program, but users can also define their own logic based on the [API](include/triad_ca_api.h) provided by `triad CA` for secondary development.

### 2 TA

1. comsst TA

    `comsst TA` is mainly used to call the underlying `TEE API` to implement the input, read, verify and delete operations of `comsst`.

2. pin TA

    `pin TA` is mainly used to call the underlying `TEE API` to implement the input, read, update, delete and verify operations of `pin`.

3. triad TA

    `triad TA` is mainly used to call the underlying `TEE API` to implement the read, delete and write operations of the system `key` and `did`.

### 3 tools

`tools` mainly includes a `set_model` tool.

`set_model` tool is mainly used to store some key information of the device, such as the device's `sn` code, `wifi mac` address, `bluetooth mac` address, and the device's unique identifier `did` and other information.

The internal implementation principle of `set_model` is to save these key information through `kvdb`.

The specific location where these data are saved can be specified by passing the specified parameters to the `set_model` tool to specify the specific storage path.

## Usage Guide

### 1 CA

1. comsst CA

    First, turn on the `CONFIG_CA_COMSST_API` option in `openvela AP`.
    Then, in the current project, a test program [comsst api demo](ca/comsst/comsst_test.c) that fully uses the `comsst CA API` is provided.

2. pin CA

    First, turn on the `CONFIG_CA_PIN_API` option in `openvela AP`.
    Then, in the current project, a test program [pin api demo](ca/pin/pin_test.c) that fully uses the `pin CA API` is provided.

3. triad CA

    First, turn on the `CONFIG_CA_TRIAD_API` option in `Vela AP`.
    Then, in the current project, a test program [triad api demo](ca/triad/triad_test.c) that fully uses the `triad CA API` is provided.

### 2 TA

If we need to use the `TA` program in `openvela`, we need to enable the following configuration options in `openvela TEE`:

```cpp
CONFIG_INTERPRETERS_WAMR=y
CONFIG_INTERPRETERS_WAMR_AOT=y
CONFIG_INTERPRETERS_WAMR_BUILD_MODULES_FOR_NUTTX=y
CONFIG_INTERPRETERS_WAMR_LIBC_BUILTIN=y
CONFIG_TA_TRIAD=y # If you use triad TA, you need to open this option
CONFIG_TA_COMSST=y # If you use comsst TA, you need to open this option
CONFIG_TA_PIN=y # If you use pin TA, you need to open this option
```

### 3 set_model

When using the `set_model` tool, we first need to turn on the `CONFIG_SC_SET_MODEL` option.

Since the `set_model` tool itself has many sub-functions, we need to turn on the corresponding options when using the corresponding functions.

The `set_model` tool is a command-line tool that can be run directly in `nsh`.

Below are the commands, parameters, and corresponding configuration options for running the `set_model` tool in `nsh`:

| Command | Expected Result | Corresponding Configuration Options |
| --  | --      | --           |
| set_model set sn 55119/F3YN00102 | [  INFO] [ap] Set sn=55119/F3YN00102 success | SC_SET_MODEL_PRODUCT_ID |
| set_model set mac_wifi CC:D8:43:20:C4:22 | [  INFO] [ap] Set mac_wifi=CC:D8:43:20:C4:22 success | SC_SET_MODEL_PRODUCT_HARDWARE |
| set_model set mac_bt CC:D8:43:20:C4:22 | [  INFO] [ap] Set mac_bt=CC:D8:43:20:C4:22 success | SC_SET_MODEL_PRODUCT_HARDWARE |
| set_model set miio_did 771897593 | [  INFO] [ap] Set miio_did=771897593 success | SC_SET_MODEL_PRODUCT_APP_ID |
| set_model set miio_key 0000000000000001 | [  INFO] [ap] Set miio_key=0000000000000001 success | SC_SET_MODEL_PRODUCT_ID |
| set_model set color_id 0 | [  INFO] [ap] Set color_id=0 success | SC_SET_MODEL_PRIORITY |
| set_model set color_desc 000000000000000 | [  INFO] [ap] Set color_desc=000000000000000 success | SC_SET_MODEL_PRIORITY |
| set_model get | [  INFO] [ap] get /data/etc/device.info success | SC_SET_MODEL_PRIORITY |
| set_model setpsm | [  INFO] [ap] Writing psm path and set property success | SC_SET_MODEL_MIIO_PSM_PATH |
| set_model reset | [  INFO] [ap] Reset /data/etc/device.info success | CONFIG_SC_SET_MODEL |