#
# Copyright (C) 2023 Xiaomi Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#

if(CONFIG_TA_PIN)
  set(WLDFLAGS
      -Wl,--export=wasm_TA_CreateEntryPoint
      -Wl,--export=wasm_TA_DestroyEntryPoint
      -Wl,--export=wasm_TA_OpenSessionEntryPoint
      -Wl,--export=wasm_TA_CloseSessionEntryPoint
      -Wl,--export=wasm_TA_InvokeCommandEntryPoint)

  list(APPEND CFLAGS -DCFG_NUM_THREADS=1 -DUSER_TA_WASM)

  if(CONFIG_DEBUG_INFO)
    list(APPEND CFLAGS -DTRACE_LEVEL=3)
  elseif(CONFIG_DEBUG_WARN)
    list(APPEND CFLAGS -DTRACE_LEVEL=2)
  elseif(CONFIG_DEBUG_ERROR)
    list(APPEND CFLAGS -DTRACE_LEVEL=1)
  else()
    list(APPEND CFLAGS -DTRACE_LEVEL=1)
  endif()

  list(
    APPEND
    INCDIR
    ${APPDIR}/frameworks/security/include
    ${APPDIR}/frameworks/security/optee_vela/include
    ${APPDIR}/interpreters/wamr/wamr/core/iwasm/include
    ${APPDIR}/external/optee/optee_os/optee_os/core/include
    ${APPDIR}/external/optee/optee_os/optee_os/lib/libutee/include
    ${APPDIR}/external/optee/optee_os/optee_os/lib/libutils/ext/include)

  if(CONFIG_ARCH_SIM)
    wasm_add_application(
      NAME
      ta_pin
      SRCS
      pin_ta.c
      STACK_SIZE
      4096
      WINCLUDES
      ${INCDIR}
      WAMR_MODE
      WASM
      WLDFLAGS
      ${WLDFLAGS}
      WCFLAGS
      ${CFLAGS}
      INSTALL_NAME
      etc/ta/EE571882BE621355ADAB1B7F23DEFAA6)
  else()
    wasm_add_application(
      NAME
      ta_pin
      SRCS
      pin_ta.c
      STACK_SIZE
      4096
      WINCLUDES
      ${INCDIR}
      WAMR_MODE
      XIP
      WLDFLAGS
      ${WLDFLAGS}
      WCFLAGS
      ${CFLAGS}
      INSTALL_NAME
      etc/ta/EE571882BE621355ADAB1B7F23DEFAA6)
  endif()

  wasm_add_library(
    NAME
    ta_frameworks
    SRCS
    ${APPDIR}/frameworks/security/optee_vela/wasm/wasm_ta_framework.c
    WINCLUDES
    ${INCDIR}
    WCFLAGS
    ${CFLAGS})

endif()
