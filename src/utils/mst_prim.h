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

// 
// Class MSTPrim:
// DESCRIPTION:
//   Exercises Prim's min spanning tree algorithm given a graph
//   and creates a min spanning tree as output
//
// EXAMPLE USAGE:
//   mst = MinSpanningTreePrim(g)

#ifndef _MST_PRIM_H_
#define _MST_PRIM_H_

#include <iostream>     // std::cout
#include <vector>       // std::vector
#include <utility>      // std::pair
#include <string>       // std::string
#include <queue>        // std::priority_queue
#include <functional>   // std::less

#include <cassert>      // assert

#include "utils/graph.h"
#include "utils/tree.h"

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------
class MSTPrim {
 public:
  using MSTElem = Tree::TreeElem;

  // Contructors
  //     Creates a MSTPrim class that runs Prim's   
  //     algorithm on Graph g and creates a tree.
  MSTPrim(const Graph &g);

  // Destructor
  ~MSTPrim() {}

  // METHODS:
  // return number of vertices in the tree
  inline uint32_t get_num_vertices() const { return _mst.get_num_vertices(); }

  //   Dumps the state of the tree in file_name
  void output_to_file(std::string file_name);

  // helper function to allow chained cout cmds: example
  // cout << "The tree: " << endl << mst << endl << "---------" << endl;
  friend std::ostream& operator << (std::ostream& os, const MSTPrim &mst);

  // ITERATORS: 
  //   We simply use delegation to Tree class
  using const_iterator = typename Tree::const_iterator;
  inline const_iterator cbegin() const { return _mst.cbegin(); }
  inline const_iterator cend() const { return _mst.cend(); }

  // Set method
  using size_type = typename Tree::size_type;

  using const_reference = typename Tree::const_reference;
  inline const_reference at(size_type n) const { return _mst.at(n); }

 protected:
 private:
  const Graph& _g;
  Tree _mst;
};

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _MST_PRIM_H_
