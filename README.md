# Security

[[English](./README.md) | [中文](./README_zh-cn.md)]

## Project Overview

The current project mainly includes the implementation of TA, CA and set_model tools in openvela. CA/TA is implemented based on the standard [GP API](https://globalplatform.org/specs-library/tee-internal-core-api-specification). If our current device supports TEE, then we call openvela running in TEE Vela TEE, and openvela running in a normal environment Vela AP. CA runs in Vela AP, and TA runs in Vela TEE.
The overall communication process between CA and TA in openvela is as follows:

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

1.comsst CA

comsst CA is a CA program used to communicate with comsst TA, which contains comsst input, read, verify and delete operations.
Comsst CA itself is a complete CA program, but users can also choose to define their own logic based on the [API] (include/comsst_ca_api.h) provided by Comsst CA for secondary development.

2.Pin CA

Pin CA is a CA program used to communicate with Pin TA, which includes the operations of obtaining, storing, deleting and verifying Pin.

Pin CA itself is a complete CA program, but users can also choose to customize their own logic based on the [API] (include/pin_ca_api.h) provided by Pin CA for secondary development.

3.Triad CA

Triad CA is a CA program used to communicate with Triad TA, which includes the operations of obtaining, deleting and updating device key, did and did hmac.
Triad CA itself is a complete CA program, but users can also define their own logic based on the [API] (include/triad_ca_api.h) provided by Triad CA for secondary development.

### TA

1.comsst TA

Comsst TA is mainly used to call the underlying TEE API to implement comsst's input, read, verify and delete operations.

2.pin TA

Pin TA is mainly used to call the underlying TEE API to implement pin's input, read, update, delete and verify operations.

3.triad TA

Triad TA is mainly used to call the underlying TEE API to implement system key and did read, delete and write operations.

### tools

The tools mainly include a set_model tool.

The set_model tool is mainly used to store some key information of the device, such as the device's sn code, wifi mac address, bluetooth mac address, and the device's unique identification did and other information.

The internal implementation principle of set_model is to save these key information through kvdb.

The specific location where these data are saved can be specified by passing the specified parameters to the set_model tool to specify the specific storage path.

## User Guide

### CA

1.comsst CA

First, turn on the CONFIG_CA_COMSST_API option in Vela AP.

Then in the current project, a complete test program using the comsst CA API is provided [comsst api demo](ca/comsst/comsst_test.c)

2.pin CA

First, turn on the CONFIG_CA_PIN_API option in Vela AP.

Then in the current project, a complete test program using the pin CA API is provided [pin api demo](ca/pin/pin_test.c)

3.triad CA

First, turn on the CONFIG_CA_TRIAD_API option in Vela AP.
Then in the current project, a complete test program using triad CA API is provided [triad api demo](ca/triad/triad_test.c)

### TA

If we need to use TA program in Vela, we need to open the following configuration options in Vela TEE:
```cpp
CONFIG_INTERPRETERS_WAMR=y
CONFIG_INTERPRETERS_WAMR_AOT=y
CONFIG_INTERPRETERS_WAMR_BUILD_MODULES_FOR_NUTTX=y
CONFIG_INTERPRETERS_WAMR_LIBC_BUILTIN=y
CONFIG_TA_TRIAD=y # If you use triad TA, you need to open this option
CONFIG_TA_COMSST=y # If you use comsst TA, you need to open this option
CONFIG_TA_PIN=y # If you use pin TA, you need to open this option
```

### set_model

When using the `set_model` tool, we first need to turn on the `CONFIG_SC_SET_MODEL` option.

Since the `set_model` tool itself has many sub-functions, we need to turn on the corresponding options when using the corresponding functions.

The `set_model` tool is a command line tool that can be run directly in `nsh`.
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