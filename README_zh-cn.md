# Security

[[English](./README.md) | [中文](./README_zh-cn.md)]

## 项目概揽

当前项目当中主要包含了`Vela`当中的`TA`,`CA`以及`set_model`工具的实现.
其中`CA/TA`是基于标准的[GP API](https://globalplatform.org/specs-library/tee-internal-core-api-specification)来实现的.
如果我们当前的设备支持TEE的话,那么我们把运行在`TEE`当中的`Vela`称为`Vela TEE`,运行在普通环境当中的`Vela`称为`Vela AP`.
其中`CA`是运行在`Vela AP`当中, `TA`是运行在`Vela TEE`当中的.
在Vela当中CA和TA之间的整体通信过程如下所示:

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

### 1. CA

1. `comsst CA`

`comsst CA`是用于同`comsst TA`之间通信的`CA`程序,里面包含了`comsst`的录入,读取,验证,删除操作.
`comsst CA`本身是一个完整的`CA`程序,但是用户也可以选择基于`comsst CA`对外提供的[API](include/comsst_ca_api.h)来定义自己的逻辑, 进行二次开发.

2. `pin CA`

`pin CA`是用于同`pin TA`之间通信的`CA`程序,里面包含了`pin`的获取,存储,删除,验证操作.
`pin CA`本身是一个完整的`CA`程序,但是用户也可以选择基于`pin CA`对外提供的[API](include/pin_ca_api.h)来自定义自己的逻辑, 进行二次开发.

3. `triad CA`

`triad CA`是用于同`triad TA`之间通信的`CA`程序,里面包含了设备`key`,`did`以及`did hmac`的获取,删除,更新操作.
`triad CA`本身是一个完整的`CA`程序,但是用户也可以基于`triad CA`对外提供的[API](include/triad_ca_api.h)来定义自己的逻辑, 进行二次开发, 

### 2. TA

1. comsst TA

`comsst TA`内部主要是用于调用底层的TEE API来实现`comsst`的录入,读取,验证,删除操作.

2. pin TA

`pin TA`内部主要是用于调用底层的TEE API来实现`pin`的录入,读取,更新,删除,验证操作.

3. triad TA

`triad TA`内部主要是用于调用底层的TEE API来实现系统key和did的读取,删除,写入操作.

### 3. tools

`tools`当中主要包含了一个`set_model`工具.

1. `set_model`

`set_model`工具主要是用于存储设备的一些关键信息,例如设备的`sn`码, `wifi mac`地址, `bluetooth mac`地址,以及设备的唯一标识`did`等信息.
`set_model`的内部实现原理是通过`kvdb`来保存这些关键信息的.
这些数据保存的具体位置可以通过给`set_model`工具传输指定的参数来指定具体的存储路径.

## 使用指南

### 1. CA

1. comsst CA

首先在Vela AP当中打开`CONFIG_CA_COMSST_API`选项.
然后在当前工程当中,提供了完整的使用`comsst CA API`的测试程序:[comsst api demo](ca/comsst/comsst_test.c)

2. pin CA

首先在Vela AP当中打开`CONFIG_CA_PIN_API`选项.
然后在当前工程当中,提供了完整的使用`pin CA API`的测试程序:[pin api demo](ca/pin/pin_test.c)

3. triad CA

首先在Vela AP当中打开`CONFIG_CA_TRIAD_API`选项.
然后在当前工程当中,提供了完整的使用`triad CA API`的测试程序:[triad api demo](ca/triad/triad_test.c)

### 2. TA

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

### 3. `set_model`

在使用`set_model`工具时,我们首先需要打开`CONFIG_SC_SET_MODEL`选项.
由于`set_model`工具本身具有很多的子功能,因此在使用相对应的功能时,我们需要打开相应的选项.

`set_model`工具是直接基于命令行的工具,可以直接在`nsh`当中运行.
下面是在`nsh`当中运行`set_model`工具的命令, 参数,以及对应的配置选项:

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
