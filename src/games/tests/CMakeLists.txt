# Copyright 2014 asarcar Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Author: Arijit Sarcar <sarcar_a@yahoo.com>

add_executable(hex_test hex_test.cc)
target_link_libraries(hex_test games)
setup_unit_test_program(hex_test)

add_executable(hex_ctest hex_test.cc)
target_link_libraries(hex_ctest games)
register_test_profile(hex_ctest)

add_executable(mc_hex_test mc_hex_test.cc)
target_link_libraries(mc_hex_test games)
setup_unit_test_program(mc_hex_test)

add_executable(mc_hex_ctest mc_hex_test.cc)
target_link_libraries(mc_hex_ctest games)
register_test(mc_hex_ctest)

