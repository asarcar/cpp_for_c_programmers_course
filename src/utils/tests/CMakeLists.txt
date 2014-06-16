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

add_executable(spt_test spt_test.cc)
target_link_libraries(spt_test utils)
setup_unit_test_program(spt_test)

add_executable(spt_ctest spt_test.cc)
target_link_libraries(spt_ctest utils)
register_test(spt_ctest "--input_file=\"${CMAKE_DATA_DIR}/input.txt\"")

add_executable(mst_test mst_test.cc)
target_link_libraries(mst_test utils)
setup_unit_test_program(mst_test)

add_executable(bfs_dfs_test bfs_dfs_test.cc)
target_link_libraries(bfs_dfs_test utils)
setup_unit_test_program(bfs_dfs_test)

add_executable(find_merge_test find_merge_test.cc)
target_link_libraries(find_merge_test utils)
setup_unit_test_program(find_merge_test)

