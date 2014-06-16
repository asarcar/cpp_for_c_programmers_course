// Copyright 2014 asarcar Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Arijit Sarcar <sarcar_a@yahoo.com>

#ifndef _FIND_MERGE_H_
#define _FIND_MERGE_H_

// Standard C++ Headers
#include <iostream>
// Standard C Headers
// Google Headers
// Local Headers

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------

// FindMerge: implements find merge algorithm 
// Each node is uniquely identified by the idx
// Conceptually the class has a vector where given a 
// node (denoted by its idx) the vector points to the 
// parent node (by storing the idx of the parent node)
// The highest ancestor does not have any parent - denoted by a -ve number 
// The -ve number is the count of the number of nodes in its set
class FindMerge {
 public:
  constexpr static uint32_t MIN_NODES = 5;
  constexpr static uint32_t DEFAULT_NUM_NODES = 10;
  constexpr static int      DEFAULT_PARENT_NODE_IDX = -1;
  explicit FindMerge(std::size_t n=DEFAULT_NUM_NODES) : 
      _v(n, DEFAULT_PARENT_NODE_IDX) {};
  explicit FindMerge(std::string file_name);
  ~FindMerge() = default;

  // Prevent unintended bad usage: 
  // Disallow: copy ctor/assignable or move ctor/assignable (C++11)
  FindMerge(const FindMerge &) = delete;
  FindMerge(FindMerge &&) = delete; // C++11 only
  void operator=(const FindMerge &) = delete;
  void operator=(const FindMerge &&) = delete; // C++11 only

  // Traverse up the node hierarchy to get the highest ancestor
  // Note: The set count is the -ve of the number in the highest ancestor
  int find_set(uint32_t node_idx);

  // useful when invoked from functions expecting const FM
  int find_set(const uint32_t node_idx) const;

  // merge the two sets and return the identity of the merged set
  int merge_set(int node_idx1, int node_idx2);

  //   Dumps the state of the find_merge in file_name
  void output_to_file(std::string file_name);

  friend std::ostream & operator <<(std::ostream& os, 
                                    const FindMerge& fm);

 protected:
 private:
  std::vector<int> _v;
};

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _FIND_MERGE_H_
