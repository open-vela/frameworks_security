# ##############################################################################
# frameworks/security/ca/triad/CMakeLists.txt
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(CONFIG_CA_TRIAD_API)

  # vela CMake build process
  nuttx_add_prebuilt(
    BIN_TARGETS
    triad_ca
    APP_TARGETS
    ${CONFIG_CA_TRIAD_TEST_PROGNAME}
    ${CONFIG_CA_TRIAD_TOOL_PROGNAME}
    EXPORT_FILES
    ../../include/triad_ca_api.h)

  nuttx_library_ifndef(SKIP_PREBUILT triad_ca)

  # ############################################################################
  # Source files
  # ############################################################################

  nuttx_sources(triad_ca_api.c)

  # ############################################################################
  # Flags always set
  # ############################################################################

  if(CONFIG_DEBUG_INFO)
    list(APPEND CFLAGS -DDEBUGLEVEL=3)
  elseif(CONFIG_DEBUG_WARN)
    list(APPEND CFLAGS -DDEBUGLEVEL=2)
  elseif(CONFIG_DEBUG_ERROR)
    list(APPEND CFLAGS -DDEBUGLEVEL=1)
  else()
    list(APPEND CFLAGS -DDEBUGLEVEL=1)
  endif()
  list(APPEND CFLAGS -DBINARY_PREFIX="ca_triad")

  nuttx_compile_options(${CFLAGS})

  # ############################################################################
  # Application add
  # ############################################################################

  if(CONFIG_CA_TRIAD_TEST)
    nuttx_add_application(
      NAME
      ${CONFIG_CA_TRIAD_TEST_PROGNAME}
      SRCS
      triad_test.c
      STACKSIZE
      ${CONFIG_CA_TRIAD_TEST_STACKSIZE}
      PRIORITY
      ${CONFIG_CA_TRIAD_TEST_PRIORITY}
      COMPILE_FLAGS
      ${CFLAGS})
  endif(CONFIG_CA_TRIAD_TEST)

  if(CONFIG_CA_TRIAD_TOOL)
    nuttx_add_application(
      NAME
      ${CONFIG_CA_TRIAD_TOOL_PROGNAME}
      SRCS
      triad_tool.c
      STACKSIZE
      ${CONFIG_CA_TRIAD_TOOL_STACKSIZE}
      PRIORITY
      ${CONFIG_CA_TRIAD_TOOL_PRIORITY}
      COMPILE_FLAGS
      ${CFLAGS})
  endif(CONFIG_CA_TRIAD_TOOL)

endif()
