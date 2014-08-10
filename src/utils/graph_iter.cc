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

// Standard C++ Headers
#include <exception>        // throw
#include <iostream>
// Standard C Headers
#include <cassert>          // assert()
// Google Headers
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/graph.h"
#include "utils/graph_iter.h"

using namespace std;

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------

EdgeCIter::EdgeCIter(const Graph &g, 
                     Graph::gvertexid_t vid, 
                     Graph::gvertexid_t next_nbr_vid):
    _g(g), _vid(vid), _nbr_vid(next_nbr_vid) {

  if (this->_vid >= _g.get_num_vertices())
    throw std::out_of_range("VertexId exceeds # of vertices in graph");
  this->_nbr_vid = _g.get_next_nbr(vid, next_nbr_vid);
    
  return;
}

// Implements the prefix increment case: ++iter; (not iter++)
Graph::econst_iterator& EdgeCIter::operator++() {
  // Move to the immediately next possible nbr and call get_next_nbr
  this->_nbr_vid = this->_g.get_next_nbr(this->_vid, this->_nbr_vid + 1);

  return (*this);
}

// Returns the reference to value stored in container
Graph::econst_reference EdgeCIter::operator*() {
  // always order the edges for undirected edges such that 
  // vid1 < vid2 as we do not have edges twice in the container
  // to save memory
  Graph::gvertexid_t vid1 = _vid;
  Graph::gvertexid_t vid2 = this->_nbr_vid;
  if (vid1 > vid2) {
    vid1 = vid2;
    vid2 = _vid;
  }

  auto it = this->_g._edges.find(std::make_pair(vid1, vid2));
  assert(it != this->_g._edges.cend());

  DLOG(INFO) << "Iterator eState: " 
             << " _vid= " << this->_vid 
             << " _nbr_vid= " << this->_nbr_vid << endl;

  DLOG(INFO) << "Iterator Value Returned: " << *it;

  return (*it);
}

Graph::econst_iterator Graph::ecbegin(Graph::gvertexid_t vid) const { 
  return Graph::econst_iterator(*this, vid, 0); 
}

Graph::econst_iterator Graph::ecend(Graph::gvertexid_t vid) const { 
  return Graph::econst_iterator(*this, vid, Graph::kMaxVertexId); 
}

// helper function to allow chained cout cmds: example
std::ostream& operator << (std::ostream& os, 
                           const Graph::econst_reference edge) {
  os << edge.first.first << " " << edge.first.second << " " << edge.second;
  return os;
}

VertexCIter::VertexCIter(Graph::VertexIterType itype,
                         const Graph &g, 
                         const Graph::SeedVertices& seed_v) :
    _itype(itype), _g(g), 
    _next_vid{0}, _end(false), 
    _visited(g.get_num_vertices()) {
  // 1. add all the vertices in the seed vector to the container
  for (auto &vid : seed_v) {
    if (_visited.is_bit_set(vid) == true)
      continue;
    _visited.set_bit(vid);
    this->push(vid);
  }

  // 2. seed the process by calling the ++ operator so that the top vertex
  //    of the container is all set to be returned.
  this->operator++();

  return;
}


// Implements the prefix increment case: ++iter; (not iter++)
Graph::vconst_iterator& 
VertexCIter::operator++() {
  // already at end: done
  if (_end == true) {
    throw std::string("Out of range: ++can't execute: reached end error");
  }

  // the container is empty: reached end and done
  if (this->size() == 0) {
    _end = true;
    return *this;
  }

  // 1. candidate vertex: get the topmost element (candidate to return) & pop the container
  _next_vid = this->top();
  this->pop();

  // 2. Iterate over all the edge nbrs of the candidate vertex. Add to the container 
  //    as the future contenders unless they have already been visited
  Graph::gedgeid_t edge;
  Graph::gvertexid_t nbr_vid;
  for (auto it = this->_g.ecbegin(_next_vid); it != this->_g.ecend(_next_vid); ++it) {
    edge = (*it).first;
    nbr_vid = ((edge.first == _next_vid) ? edge.second : edge.first);
    if (_visited.is_bit_set(nbr_vid) == true)
      continue;
    _visited.set_bit(nbr_vid);
    this->push(nbr_vid);
  }

  return *this;
}

// Returns the reference to value stored in container
Graph::vconst_reference 
VertexCIter::operator*() {
  DLOG(INFO) << "Iteratorv State " 
             << ": _itype " << Graph::str_vertex_iter_type(_itype) 
             << ": _next_vid " << _next_vid 
             << std::boolalpha
             << ": _end " << _end ;

  DLOG(INFO) << "Iterator Value Returned: " << _next_vid;

  return _next_vid;
}

Graph::vconst_iterator 
Graph::vcbegin(Graph::VertexIterType itype, 
               const Graph::gvertexid_t &seed_vid) const {
  // Case 1: We start with one seed vertex to start the BFS/DFS
  // Just map it to the multi-vertex seed case where N==1
  Graph::SeedVertices seed_v(1, seed_vid);
  return Graph::vconst_iterator(itype, *this, seed_v);
}

// The BFS or DFS search is initiated by all the vertices
// provided in the vector. In a multi-threaded applicatio
// we can initiate the search parallely in each 
// thread for each of the seed vertices
Graph::vconst_iterator 
Graph::vcbegin(Graph::VertexIterType itype, 
               const Graph::SeedVertices& seed_v) const {
  return Graph::vconst_iterator(itype, *this, seed_v);
}

Graph::vconst_iterator Graph::vcend(Graph::VertexIterType itype) const {
  vector<Graph::gvertexid_t> empty_v(0);
  return Graph::vconst_iterator(itype, *this, empty_v);
}

void VertexCIter::push(const Graph::gvertexid_t& vid) {
  if (_itype == Graph::VertexIterType::BFS_ORDER) {
    _q.push(vid);
    return;
  }
  _s.push(vid);
  return;
}

// Based on itype (BFS/DFS) push the queue or stack
void VertexCIter::pop(void) {
  if (_itype == Graph::VertexIterType::BFS_ORDER) {
    _q.pop();
    return;
  }
  _s.pop();
  return;
}

// Based on itype (BFS/DFS) front the queue or top the stack
Graph::gvertexid_t& VertexCIter::top(void) {
  if (_itype == Graph::VertexIterType::BFS_ORDER) {
    return _q.front();
  }
  return _s.top();
}
  
// Based on itype (BFS/DFS) return the size of queue or stack
std::size_t VertexCIter::size(void) {
  if (_itype == Graph::VertexIterType::BFS_ORDER) {
    return _q.size();
  }
  return _s.size();
}

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
