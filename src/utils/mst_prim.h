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
// Forward Declarations
template <typename GCost>
class MSTPrim;

template <typename GCost>
std::ostream& operator <<(std::ostream&, const MSTPrim<GCost>&);
// End of Forward Declarations

template <typename GCost>
class MSTPrim {
 public:
  // Contructors
  //     Creates a MSTPrim class that runs Prim's   
  //     algorithm on Graph g and creates a tree.
  MSTPrim(const Graph<GCost> &g);

  // Destructor
  ~MSTPrim() {}

  // METHODS:
  // return number of vertices in the tree
  inline uint32_t get_num_vertices() const { return _mst.get_num_vertices(); }

  //   Dumps the state of the tree in file_name
  void output_to_file(std::string file_name);

  // helper function to allow chained cout cmds: example
  // cout << "The tree: " << endl << mst << endl << "---------" << endl;
  friend std::ostream& operator << <>(std::ostream& os, const MSTPrim<GCost> &mst);

  // ITERATORS: 
  //   We simply use delegation to Tree class
  using MConstIterator = typename Tree<GCost>::TConstIterator;
  inline MConstIterator cbegin() const { return _mst.cbegin(); }
  inline MConstIterator cend() const { return _mst.cend(); }

  // Set method
  using size_type = typename Tree<GCost>::size_type;

  using MConstReference = typename Tree<GCost>::TConstReference;
  inline MConstReference at(size_type n) const { return _mst.at(n); }

 protected:
 private:
  const Graph<GCost>& _g;
  Tree<GCost> _mst;
};

// Suppress implicit instantiation
extern template std::ostream& 
operator << <uint32_t>(std::ostream& os, const MSTPrim<uint32_t> &t);
extern template class MSTPrim<uint32_t>;

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _MST_PRIM_H_
