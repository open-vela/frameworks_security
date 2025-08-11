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

if(CONFIG_TA_HELLO_WORLD)
    ta_add_application(
      NAME
      ta_hello_world
      SRCS
      hello_world_ta.c
      STACKSIZE
      4096
      INSTALL_NAME
      etc/ta/00F2AA8A5024E411ABE20002A5D5C51B)
endif()
