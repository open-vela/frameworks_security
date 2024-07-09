############################################################################
#
# Copyright (C) 2022-2024 Xiaomi Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

include $(APPDIR)/Make.defs

ifeq ($(CONFIG_CA_TRIAD_TEST),y)
PROGNAME = $(CONFIG_CA_TRIAD_TEST_PROGNAME)
PRIORITY = $(CONFIG_CA_TRIAD_TEST_PRIORITY)
STACKSIZE = $(CONFIG_CA_TRIAD_TEST_STACKSIZE)
MODULE = $(CONFIG_CA_TRIAD_TEST)
MAINSRC = triad_test.c
endif

ifeq ($(CONFIG_CA_TRIAD_TOOL),y)
PROGNAME = $(CONFIG_CA_TRIAD_TOOL_PROGNAME)
PRIORITY = $(CONFIG_CA_TRIAD_TOOL_PRIORITY)
STACKSIZE = $(CONFIG_CA_TRIAD_TOOL_STACKSIZE)
MODULE = $(CONFIG_CA_TRIAD_TOOL)
MAINSRC = triad_tool.c
endif

CSRCS += triad_ca_api.c

ifneq ($(CONFIG_DEBUG_INFO),)
CFLAGS += -DDEBUGLEVEL=3
else ifneq ($(CONFIG_DEBUG_WARN),)
CFLAGS += -DDEBUGLEVEL=2
else ifneq ($(CONFIG_DEBUG_ERROR),)
CFLAGS += -DDEBUGLEVEL=1
else
# the default DEBUGLEVEL are 1(with error level)
CFLAGS += -DDEBUGLEVEL=1
endif

CFLAGS += -DBINARY_PREFIX='"ca_triad"'

EXPORT_FILES := $(APPDIR)/frameworks/security/include/triad_ca_api.h

include $(APPDIR)/Application.mk