# Security

[[English](./README.md) | [中文](./README_zh-cn.md)]

## Project Overview

The current project mainly includes the implementation of `TA`, `CA`, and the `set_model` tool in `Vela`.
Among them, `CA/TA` is implemented based on the standard [GP API](https://globalplatform.org/specs-library/tee-internal-core-api-specification).
If our current device supports TEE, then we call the `Vela` running in `TEE` as `Vela TEE`, and the `Vela` running in the normal environment as `Vela AP`.
Among them, `CA` runs in `Vela AP`, and `TA` runs in `Vela TEE`.
The overall communication process between CA and TA in Vela is as follows:

```log
+-------------+               +---------------+
|[Vela AP]    |               |[Vela TEE]     |
|             |               |               |
|    CA       |               |     TA        |
|    |        |               |     /|\       |
|   \|/       |               |      |        |
| LIB_TEEC    |               |  TA MANAGER   |
|    |        |               |     /|\       |
|   \|/       | rpmsg socket  |      |        |
|  /dev/tee0 <----------------> opteed server |
|_____________|               |_______________|
```

## Project Description

### 1. CA

1. `comsst CA`

`comsst CA` is a `CA` program used for communication with `comsst TA`. It contains operations such as input, reading, verification, and deletion of `comsst`.
`comsst CA` itself is a complete `CA` program, but users can also choose to define their own logic and conduct secondary development based on the [API](include/comsst_ca_api.h) provided by `comsst CA`.

2. `pin CA`

`pin CA` is a `CA` program used for communication with `pin TA`. It contains operations such as acquisition, storage, deletion, and verification of `pin`.
`pin CA` itself is a complete `CA` program, but users can also choose to define their own logic and conduct secondary development based on the [API](include/pin_ca_api.h) provided by `pin CA`.

3. `triad CA`

`triad CA` is a `CA` program used for communication with `triad TA`. It contains operations such as acquisition, deletion, and update of device `key`, `did`, and `did hmac`.
`triad CA` itself is a complete `CA` program, but users can also define their own logic and conduct secondary development based on the [API](include/triad_ca_api.h) provided by `triad CA`.

### 2. TA

1. comsst TA

The inside of `comsst TA` is mainly used to call the underlying TEE API to implement operations such as input, reading, verification, and deletion of `comsst`.

2. pin TA

The inside of `pin TA` is mainly used to call the underlying TEE API to implement operations such as input, reading, update, deletion, and verification of `pin`.

3. triad TA

The inside of `triad TA` is mainly used to call the underlying TEE API to implement read, delete, and write operations of system keys and dids.

### 3. tools

The `tools` mainly contains a `set_model` tool.

1. `set_model`

The `set_model` tool is mainly used to store some key information of the device, such as the device's `sn` code, `wifi mac` address, `bluetooth mac` address, and the device's unique identifier `did` and other information.
The internal implementation principle of `set_model` is to save these key information through `kvdb`.
The specific storage location of these data can be specified by passing specific parameters to the `set_model` tool to specify the specific storage path.

## User Guide

### 1. CA

1. comsst CA

First, turn on the `CONFIG_CA_COMSST_API` option in Vela AP.
Then in the current project, a complete test program for using the `comsst CA API` is provided: [comsst api demo](ca/comsst/comsst_test.c).

2. pin CA

First, turn on the `CONFIG_CA_PIN_API` option in Vela AP.
Then in the current project, a complete test program for using the `pin CA API` is provided: [pin api demo](ca/pin/pin_test.c).

3. triad CA

First, turn on the `CONFIG_CA_TRIAD_API` option in Vela AP.
Then in the current project, a complete test program for using the `triad CA API` is provided: [triad api demo](ca/triad/triad_test.c).

### 2. TA

If we need to use the TA program in Vela, we need to turn on the following configuration options in Vela TEE:
```cpp
CONFIG_INTERPRETERS_WAMR=y
CONFIG_INTERPRETERS_WAMR_AOT=y
CONFIG_INTERPRETERS_WAMR_BUILD_MODULES_FOR_NUTTX=y
CONFIG_INTERPRETERS_WAMR_LIBC_BUILTIN=y
CONFIG_TA_TRIAD=y # If using triad TA, this option needs to be turned on.
CONFIG_TA_COMSST=y # If using comsst TA, this option needs to be turned on.
CONFIG_TA_PIN=y # If using pin TA, this option needs to be turned on.
```

### 3. `set_model`

When using the `set_model` tool, we first need to turn on the `CONFIG_SC_SET_MODEL` option.
Since the `set_model` tool itself has many sub-functions, when using the corresponding function, we need to turn on the corresponding option.

The `set_model` tool is a command-line-based tool and can be run directly in `nsh`.
The following is the command, parameters, and corresponding configuration options for running the `set_model` tool in `nsh`:

| Command | Expected Result | Corresponding Configuration Option |
| -- | -- | -- |
| set_model set sn 55119/F3YN00102 | [ INFO] [ap] Set sn=55119/F3YN00102 success | SC_SET_MODEL_PRODUCT_ID |
| set_model set mac_wifi CC:D8:43:20:C4:22 | [ INFO] [ap] Set mac_wifi=CC:D8:43:20:C4:22 success | SC_SET_MODEL_PRODUCT_HARDWARE |
| set_model set mac_bt CC:D8:43:20:C4:22 | [ INFO] [ap] Set mac_bt=CC:D8:43:20:C4:22 success | SC_SET_MODEL_PRODUCT_HARDWARE |
| set_model set miio_did 771897593 | [ INFO] [ap] Set miio_did=771897593 success | SC_SET_MODEL_PRODUCT_APP_ID |
| set_model set miio_key 0000000000000001 | [ INFO] [ap] Set miio_key=0000000000000001 success | SC_SET_MODEL_PRODUCT_ID |
| set_model set color_id 0 | [ INFO] [ap] Set color_id=0 success | SC_SET_MODEL_PRIORITY |
| set_model set color_desc 000000000000000 | [ INFO] [ap] Set color_desc=000000000000000 success | SC_SET_MODEL_PRIORITY |
| set_model get | [ INFO] [ap] get /data/etc/device.info success | SC_SET_MODEL_PRIORITY |
| set_model setpsm | [ INFO] [ap] Writing psm path and set property success | SC_SET_MODEL_MIIO_PSM_PATH |
| set_model reset | [ INFO] [ap] Reset /data/etc/device.info success | CONFIG_SC_SET_MODEL |
