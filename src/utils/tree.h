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
// Class Tree:
// DESCRIPTION:
//   Container for a tree (without any loop) based on a graph reference.
//
// EXAMPLE USAGE:
//   The tree is typically created as a result of running another
//   algorithm on the graph: e.g. Dijkstra based on a source vertex,
//   Prim's Minimum Spanning Tree etc.
//   tree.output("outout_file") OR std::cout << tree << std::endl;

#ifndef _TREE_H_
#define _TREE_H_

// Standing C++ Headers
#include <fstream>      // std::ifstream & std::ofstream
#include <iostream>     // std::cout
#include <string>       // std::string
#include <sstream>      // std::stringstream
#include <utility>      // std::pair
#include <vector>       // std::vector
// Standard C Headers
#include <cassert>      // assert

#include "utils/graph.h"

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------
// Forward Declarations
template <typename GCost>
class Tree;

// Adding a typedef/using to refer to each entry in the tree container
template <typename GCost>
using TreeElem = typename std::pair<GVertexId, GCost>;

template <typename GCost>
std::ostream& operator <<(std::ostream&, const Tree<GCost>&);
// End of Forward Declarations

template <typename GCost>
class Tree {
 public:
  // Contructors
  //     Creates a Tree class referenced on Graph g
  Tree(const Graph<GCost> &g): 
      _g(g), _v{g.get_num_vertices()} {}

  // Destructor
  ~Tree() {}

  // METHODS:
  // return number of vertices in the tree
  inline uint32_t get_num_vertices() const { return _g.get_num_vertices(); }

  //   Dumps the state of the tree in file_name
  void output_to_file(std::string file_name);

  // helper function to allow chained cout cmds: example
  // cout << "The tree: " << endl << t << endl << "---------" << endl;
  // Template Friend Function: the "<>" after function 
  // specifies that this is a template function accepting 
  // template arguments
  friend std::ostream& operator << <>(std::ostream& os, const Tree<GCost> &t);

  // ITERATORS
  //   We simply use delegation to Vector class
  using TConstIterator = typename std::vector<TreeElem<GCost>>::const_iterator;
  inline TConstIterator cbegin() const { return _v.cbegin(); }
  inline TConstIterator cend() const { return _v.cend(); }

  using TIterator = typename std::vector<TreeElem<GCost>>::iterator;
  inline TIterator begin() { return _v.begin(); }
  inline TIterator end() { return _v.end(); }

  using size_type = typename std::vector<TreeElem<GCost>>::size_type;

  using TConstReference = typename std::vector<TreeElem<GCost>>::const_reference;
  inline TConstReference at(size_type n) const { return _v.at(n); }

  using TReference = typename std::vector<TreeElem<GCost>>::reference;
  inline TReference at(size_type n) { return _v.at(n); }

 protected:
 private:
  const Graph<GCost>& _g;
  // The tree is stores as a simple vector of vertices 
  // The associated information wrt each vid stored in the vector
  // 1. parent_vid i.e.parent vertex of the vid
  // 2. Edge Cost (parent_vid, vid): available from graph
  // 3. Path Cost (svid-->vid) total path cost: calculated from graph
  std::vector<TreeElem<GCost>> _v;
};

// Suppress implicit instantiation
extern template std::ostream& 
operator << <uint32_t>(std::ostream& os, const Tree<uint32_t> &t);
extern template class Tree<uint32_t>;

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _TREE_H_
