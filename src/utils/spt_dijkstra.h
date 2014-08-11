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
// Class SPTDijkstra:
// DESCRIPTION:
//   Exercises the shortest path algorithm given a graph.
//
// EXAMPLE USAGE:
//   sp = SPTDijkstra(g)
//   if (sp.get_spt_dijkstra(v1, v2, vertex_vector) == true) 
//     process the vertices provided in vertex_vector
//   if (sp.get_path_cost(vid1, vid2, path_cost) == true)
//     process path_cost
//   avg_path = sp.get_avg_path_size(vid, avg_path) provides avg_path_len

#ifndef _SPT_DIJKSTRA_H_
#define _SPT_DIJKSTRA_H_

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
class SPTDijkstra;

template <typename GCost>
std::ostream& operator <<(std::ostream&, const SPTDijkstra<GCost>&);
// End of Forward Declarations

template <typename GCost>
class SPTDijkstra {
 public:

  // Contructors
  //     Creates a SPTDijkstra class that is ready to provide results of 
  //     the algorithm on Graph g. At construction it runs Dijkstra with 
  //     vertex 0 as the default root vertex. 
  //     One can subsequently run_spt_dijkstra with another root vertex
  //     if so desired.
  SPTDijkstra(const Graph<GCost>& g): _g(g), _spt(g) { run_spt_dijkstra(0); }

  // Destructor
  ~SPTDijkstra() {}

  // METHODS:
  //   run_spt_dijkstra with srv_vid as root of the tree
  //     arg1: root vertex id
  void run_spt_dijkstra(GVertexId root_vid);

  //   get_path_size
  //     arg1: source vertex id
  //     arg2: destination vertex id
  //     Return: path_cost from source to destination vertex id
  GCost 
  get_path_size(GVertexId vid1, GVertexId vid2);

  //   get_avg_path_size_for_vertex
  //     arg1: source vertex id
  //     return: avg path size from given vertex id to all other reachable 
  //             vertices
  double get_avg_path_size_for_vertex(GVertexId vid);

  //   get_avg_path_size
  //     return: avg path size from all vertices to all other reachable 
  //             vertices
  double get_avg_path_size(void);

  // return number of vertices in the tree
  inline uint32_t get_num_vertices() const { return _spt.get_num_vertices(); }

  //   Dumps the state of the SPT in file_name
  void output_to_file(std::string file_name);

  // helper function to allow chained output cmds: example
  // cout << "The SPT: " << endl << spt << endl << "---------" << endl;
  friend std::ostream& operator << <>(std::ostream& os, const SPTDijkstra<GCost> &spt);

  // ITERATORS: 
  //   We simply use delegation to Tree class
  using SConstIterator = typename Tree<GCost>::TConstIterator;
  inline SConstIterator cbegin() const { return _spt.cbegin(); }
  inline SConstIterator cend() const { return _spt.cend(); }

  // Set method
  using size_type = typename Tree<GCost>::size_type;
  using SConstReference = typename Tree<GCost>::TConstReference;
  inline SConstReference at(size_type n) const { return _spt.at(n); }

 protected:

 private:
  const Graph<GCost> &_g;
  Tree<GCost> _spt;
  // as the graph does not allow self referential nodes
  // i.e. an edge from a node N to itsel, we designate a root of a tree 
  // by having it point to itself in the tree
};

// Suppress implicit instantiation
extern template std::ostream& 
operator << <uint32_t>(std::ostream& os, const SPTDijkstra<uint32_t> &spt);
extern template class SPTDijkstra<uint32_t>;

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _SPT_DIJKSTRA_H_
