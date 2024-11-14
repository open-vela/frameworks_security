# Security

[[English](./README.md) | [中文](./README_zh-cn.md)]

## 项目概览

当前项目当中主要包含了openvela当中的TA，CA以及set_model工具的实现，其中CA/TA是基于标准的[GP API](https://globalplatform.org/specs-library/tee-internal-core-api-specification)来实现的，如果我们当前的设备支持TEE的话，那么我们把运行在TEE当中的openvela称为Vela TEE，运行在普通环境当中的openvela称为Vela AP。其中CA是运行在Vela AP当中, TA是运行在Vela TEE当中的。
在openvela当中CA和TA之间的整体通信过程如下所示:

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

## 项目描述

### CA

1.comsst CA

comsst CA是用于同comsst TA之间通信的CA程序，里面包含了comsst的录入读取验证删除操作。
comsst CA本身是一个完整的CA程序，但是用户也可以选择基于comsst CA对外提供的[API](include/comsst_ca_api.h)来定义自己的逻辑, 进行二次开发。

2.pin CA

pin CA是用于同pin TA之间通信的CA程序,里面包含了pin的获取存储删除验证操作。
pin CA本身是一个完整的CA程序，但是用户也可以选择基于pin CA对外提供的[API](include/pin_ca_api.h)来自定义自己的逻辑, 进行二次开发。

3.triad CA

triad CA是用于同triad TA之间通信的CA程序，里面包含了设备key，did以及did hmac的获取删除和更新操作。
triad CA本身是一个完整的CA程序，但是用户也可以基于triad CA对外提供的[API](include/triad_ca_api.h)来定义自己的逻辑, 进行二次开发。

### TA

1.comsst TA

comsst TA内部主要是用于调用底层的TEE API来实现comsst的录入读取验证删除操作。

2.pin TA

pin TA内部主要是用于调用底层的TEE API来实现pin的录入读取更新删除验证操作。

3.triad TA

triad TA内部主要是用于调用底层的TEE API来实现系统key和did的读取删除和写入操作。

### tools

tools当中主要包含了一个set_model工具。

set_model工具主要是用于存储设备的一些关键信息，例如设备的sn码, wifi mac地址, bluetooth mac地址，以及设备的唯一标识did等信息。
set_model的内部实现原理是通过kvdb来保存这些关键信息的。
这些数据保存的具体位置可以通过给set_model工具传输指定的参数来指定具体的存储路径。

## 使用指南

### CA

1.comsst CA

首先在Vela AP当中打开`CONFIG_CA_COMSST_API`选项。
然后在当前工程当中,提供了完整的使用`comsst CA API`的测试程序[comsst api demo](ca/comsst/comsst_test.c)

2.pin CA

首先在Vela AP当中打开`CONFIG_CA_PIN_API`选项。
然后在当前工程当中,提供了完整的使用`pin CA API`的测试程序[pin api demo](ca/pin/pin_test.c)

3.triad CA

首先在Vela AP当中打开`CONFIG_CA_TRIAD_API`选项。
然后在当前工程当中,提供了完整的使用`triad CA API`的测试程序[triad api demo](ca/triad/triad_test.c)

### TA

如果我们需要在Vela当中使用TA程序的话,需要在Vela TEE当中打开下面的配置选项:
```cpp
CONFIG_INTERPRETERS_WAMR=y
CONFIG_INTERPRETERS_WAMR_AOT=y
CONFIG_INTERPRETERS_WAMR_BUILD_MODULES_FOR_NUTTX=y
CONFIG_INTERPRETERS_WAMR_LIBC_BUILTIN=y
CONFIG_TA_TRIAD=y # 如果是使用triad TA的话,需要打开这个选项
CONFIG_TA_COMSST=y # 如果是使用comsst TA的话,需要打开这个选项
CONFIG_TA_PIN=y # 如果是使用pin TA的话,需要打开这个选项
```

### set_model

在使用`set_model`工具时,我们首先需要打开CONFIG_SC_SET_MODEL选项。
由于`set_model`工具本身具有很多的子功能，因此在使用相对应的功能时，我们需要打开相应的选项。

`set_model`工具是直接基于命令行的工具,可以直接在nsh当中运行。
下面是在nsh当中运行set_model工具的命令, 参数，以及对应的配置选项：

| 命令 | 预期结果 | 对应的配置选项 |
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
