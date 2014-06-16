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
class SPTDijkstra {
 public:
  using SPTElem=Tree::TreeElem;

  // Contructors
  //     Creates a SPTDijkstra class that is ready to provide results of 
  //     the algorithm on Graph g. At construction it runs Dijkstra with 
  //     vertex 0 as the default root vertex. 
  //     One can subsequently run_spt_dijkstra with another root vertex
  //     if so desired.
  SPTDijkstra(const Graph& g): _g(g), _spt(g) { run_spt_dijkstra(0); }

  // Destructor
  ~SPTDijkstra() {}

  // METHODS:
  //   run_spt_dijkstra with srv_vid as root of the tree
  //     arg1: root vertex id
  void run_spt_dijkstra(Graph::gvertexid_t root_vid);

  //   get_path_size
  //     arg1: source vertex id
  //     arg2: destination vertex id
  //     Return: path_cost from source to destination vertex id
  Graph::gcost_t 
  get_path_size(Graph::gvertexid_t vid1, Graph::gvertexid_t vid2);

  //   get_avg_path_size_for_vertex
  //     arg1: source vertex id
  //     return: avg path size from given vertex id to all other reachable 
  //             vertices
  double get_avg_path_size_for_vertex(Graph::gvertexid_t vid);

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
  friend std::ostream& operator << (std::ostream& os, const SPTDijkstra &spt);

  // ITERATORS: 
  //   We simply use delegation to Tree class
  using const_iterator = typename Tree::const_iterator;
  inline const_iterator cbegin() const { return _spt.cbegin(); }
  inline const_iterator cend() const { return _spt.cend(); }

  // Set method
  using size_type = typename Tree::size_type;

  using const_reference = typename Tree::const_reference;
  inline const_reference at(size_type n) const { return _spt.at(n); }

 protected:

 private:
  const Graph &_g;
  Tree _spt;
  // as the graph doe not allow self referential nodes
  // i.e. an edge from a node N to itsel, we designate a root of a tree 
  // by having it point to itself in the tree
};

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _SPT_DIJKSTRA_H_
