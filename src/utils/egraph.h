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
// Class eGraph:
// Extends Graph class to allow storing values to Graph Nodes

#ifndef _EGRAPH_H_
#define _EGRAPH_H_
// C++ Standard Headers
#include <fstream>          // std::ifstream & std::ofstream
#include <functional>       // std::BinaryPredicate, std::equal_to
#include <iostream>         // std::cout
#include <vector>           // std::vector
// C Standard Headers
// Local Headers
#include "utils/graph.h"

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------
// VT: Vertex Attribute Tempate
template <typename vAttr, typename vAttrIsEqual = std::equal_to<vAttr>>
class eGraph : public Graph {
 public:
  explicit eGraph(uint32_t num_vertices=0) :
      Graph(Graph::EdgeType::UNDIRECTED, num_vertices, 0, 1, 1),
      _vmap(num_vertices) {}
  explicit eGraph(std::string file_name);
  virtual ~eGraph() = default;

  // Prevent unintended bad usage: 
  // Disallow: copy ctor/assignable or move ctor/assignable (C++11)
  eGraph(const eGraph &) = delete;
  eGraph(eGraph &&) = delete; // C++11 only
  void operator=(const eGraph &) = delete;
  void operator=(eGraph &&) = delete; // C++11 only

  inline void set_vertex_attr(Graph::gvertexid_t vid, const vAttr& vA) {
    _vmap.at(vid) = vA;
    return;
  }

  inline const vAttr& get_vertex_attr(Graph::gvertexid_t vid) const {
    return _vmap.at(vid);
  }

  inline vAttr& get_vertex_attr(Graph::gvertexid_t vid) {
    return _vmap.at(vid);
  }

  // get_next_nbr: provide the first nbr vertex that is available
  // immediately from or after the passed nbr_vid 
  // *as long as* the attribute of the vertices match
  virtual gvertexid_t get_next_nbr(gvertexid_t vid, gvertexid_t nbr_vid) const {
    Graph::gvertexid_t vid_end = get_num_vertices();
    assert(vid < vid_end);
    for (Graph::gvertexid_t vid2 = nbr_vid; vid2 < vid_end; ++vid2) {
      // We should replace the comparison of attributes with a functor
      if (isset_adjmap(std::make_pair(vid, vid2)) &&
          _vattr_is_equal(_vmap.at(vid), _vmap.at(vid2)))
        return vid2;
    }
    return Graph::kMaxVertexId;
  }
  // Save State & Restore State: used by MC simulation to run "what if scenarios" 
  // without messing up current state of eGraph
  inline void save_state(void) { 
    _save = _vmap; 
    return; 
  }
  inline void restore_state(void) { 
    _vmap = _save; 
    return; 
  }
 protected:
 private:
  // Vertex Value: Every Vertex has an associated information of arbitrary 
  // complexity and size based on the preference of user
  using gvertex_map_t = std::vector<vAttr>;
  gvertex_map_t _vmap;
  gvertex_map_t _save;
  vAttrIsEqual _vattr_is_equal;
};

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _EGRAPH_H_
