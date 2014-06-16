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

set(HDR_LIST basictypes.h find_merge.h graph.h graph_iter.h init.h mst_prim.h spt_dijkstra.h tree.h)
setup_custom_headers("${HDR_LIST}")

add_library(utils find_merge.cc graph.cc graph_iter.cc init.cc mst_prim.cc spt_dijkstra.cc tree.cc)
target_link_libraries(utils gflags glog profiler tcmalloc)
setup_custom_target(utils)

if (CMAKE_UNIT_TESTS)
  add_subdirectory(tests)
endif (CMAKE_UNIT_TESTS)
