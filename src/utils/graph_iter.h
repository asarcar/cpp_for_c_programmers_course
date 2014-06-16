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

#ifndef _GRAPH_ITER_H_
#define _GRAPH_ITER_H_

// Standard C++ Headers
#include <iostream>         // std::cout
#include <limits>           // std::numeric_limits
#include <queue>            // std::queue
#include <stack>            // std::stack
#include <vector>           // std::vector
// Standard Headers
#include <cassert>          // assert
// Google Headers
// Local Headers
#include "utils/graph.h"

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------

// ITERATOR Definitions
// Edge Iterator
class EdgeCIter {
 public:
  explicit EdgeCIter(const Graph &g, 
                     Graph::gvertexid_t vid=0, 
                     Graph::gvertexid_t next_nbr_vid=0);
  
  bool operator ==(const EdgeCIter& o) const {
    return ((this->_g == o._g) && 
            (this->_vid == o._vid) &&
            (this->_nbr_vid == o._nbr_vid));
  }
  
  bool operator !=(const EdgeCIter& o) const { 
    return !(*this == o); 
  }
  
  // Implements the prefix increment case: ++iter; (not iter++)
  Graph::econst_iterator& operator++(); 
  // Returns the reference to value stored in container
  Graph::econst_reference operator*(); 
 protected:
 private:
  const Graph &_g;
  // vertex_id over which the iterator is elaborating edges
  const Graph::gvertexid_t _vid; 
  // neighbor vertex_id that is the next edge candidate for _vid
  Graph::gvertexid_t _nbr_vid; 
};

// helper function to allow chained cout cmds: example
std::ostream& operator << (std::ostream& os, 
                           const Graph::econst_reference edge);

// Vertex Iterator
class VertexCIter {
 public:
  explicit VertexCIter(Graph::VertexIterType itype,
                       const Graph &g, 
                       const Graph::SeedVertices& seed_v);
  
  inline bool operator ==(const VertexCIter& o) const {
    return ((this->_itype == o._itype) &&
            (this->_g == o._g) && 
            (this->_q.size() == o._q.size()) &&
            (this->_s.size() == o._s.size()) &&
            (this->_end == o._end));
  }
  
  inline bool operator !=(const VertexCIter& o) const { 
    return !(*this == o); 
  }

  // Implements the prefix increment case: ++iter; (not iter++)
  Graph::vconst_iterator& operator++(); 
  // Returns the reference to value stored in container
  Graph::vconst_reference operator*(); 
 protected:
 private:
  Graph::VertexIterType _itype;
  const Graph &         _g;
  Graph::gvertexid_t    _next_vid;
  bool                  _end;
  BitSet                _visited; 
  std::queue<Graph::gvertexid_t> _q; // container for BFS
  std::stack<Graph::gvertexid_t> _s; // container used for DFS
  // tracks all the vertices that have already been visited via bitmap

  // Private Methods
  // Based on itype (BFS/DFS) push the queue or stack
  void push(const Graph::gvertexid_t& vid);
  // Based on itype (BFS/DFS) push the queue or stack
  void pop(void);
  // Based on itype (BFS/DFS) front the queue or top the stack
  Graph::gvertexid_t& top(void);
  // Based on itype (BFS/DFS) return the size of queue or stack
  std::size_t size(void);
};

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _GRAPH_ITER_H_
