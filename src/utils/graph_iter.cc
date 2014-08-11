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
template <typename GCost>
GVertexCIter<GCost>::GVertexCIter(GVertexIterType itype,
                                  const Graph<GCost> &g, 
                                  const GVertexIterSeed& seed_v) :
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
template <typename GCost>
GVertexCIter<GCost>& GVertexCIter<GCost>::operator++() {
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
  GEdgeId edge;
  GVertexId nbr_vid;
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
template <typename GCost>
GVertexIterConstReference
GVertexCIter<GCost>::operator*() {
  DLOG(INFO) << "Iteratorv State " 
             << ": _itype " << Graph<GCost>::str_vertex_iter_type(_itype) 
             << ": _next_vid " << _next_vid 
             << std::boolalpha
             << ": _end " << _end ;

  DLOG(INFO) << "Iterator Value Returned: " << _next_vid;

  return _next_vid;
}

template <typename GCost>
GVertexCIter<GCost> 
Graph<GCost>::vcbegin(GVertexIterType itype, 
                      const GVertexId &seed_vid) const {
  // Case 1: We start with one seed vertex to start the BFS/DFS
  // Just map it to the multi-vertex seed case where N==1
  GVertexIterSeed seed_v(1, seed_vid);
  return GVertexCIter<GCost>(itype, *this, seed_v);
}

// The BFS or DFS search is initiated by all the vertices
// provided in the vector. In a multi-threaded applicatio
// we can initiate the search parallely in each 
// thread for each of the seed vertices
template <typename GCost>
GVertexCIter<GCost> 
Graph<GCost>::vcbegin(GVertexIterType itype, 
                      const GVertexIterSeed& seed_v) const {
  return GVertexCIter<GCost>(itype, *this, seed_v);
}

template <typename GCost>
GVertexCIter<GCost> 
Graph<GCost>::vcend(GVertexIterType itype) const {
  vector<GVertexId> empty_v(0);
  return GVertexCIter<GCost>(itype, *this, empty_v);
}

template <typename GCost>
void GVertexCIter<GCost>::push(const GVertexId& vid) {
  if (_itype == GVertexIterType::BFS_ORDER) {
    _q.push(vid);
    return;
  }
  _s.push(vid);
  return;
}

// Based on itype (BFS/DFS) push the queue or stack
template <typename GCost>
void GVertexCIter<GCost>::pop(void) {
  if (_itype == GVertexIterType::BFS_ORDER) {
    _q.pop();
    return;
  }
  _s.pop();
  return;
}

// Based on itype (BFS/DFS) front the queue or top the stack
template <typename GCost>
GVertexId& GVertexCIter<GCost>::top(void) {
  if (_itype == GVertexIterType::BFS_ORDER) {
    return _q.front();
  }
  return _s.top();
}
  
// Based on itype (BFS/DFS) return the size of queue or stack
template <typename GCost>
size_t GVertexCIter<GCost>::size(void) {
  if (_itype == GVertexIterType::BFS_ORDER) {
    return _q.size();
  }
  return _s.size();
}

template <typename GCost>
GEdgeCIter<GCost>::GEdgeCIter(const Graph<GCost> &g, 
                              GVertexId vid, 
                              GVertexId next_nbr_vid):
    _g(g), _vid(vid), _nbr_vid(next_nbr_vid) {

  if (this->_vid >= _g.get_num_vertices())
    throw std::out_of_range("VertexId exceeds # of vertices in graph");
  this->_nbr_vid = _g.get_next_nbr(vid, next_nbr_vid);
    
  return;
}

// Implements the prefix increment case: ++iter; (not iter++)
template <typename GCost>
GEdgeCIter<GCost>& 
GEdgeCIter<GCost>::operator++() {
  // Move to the immediately next possible nbr and call get_next_nbr
  this->_nbr_vid = this->_g.get_next_nbr(this->_vid, this->_nbr_vid + 1);

  return (*this);
}

// Returns the reference to value stored in container
template <typename GCost>
GEdgeIterConstReference<GCost> GEdgeCIter<GCost>::operator*() {
  // always order the edges for undirected edges such that 
  // vid1 < vid2 as we do not have edges twice in the container
  // to save memory
  GVertexId vid1 = _vid;
  GVertexId vid2 = this->_nbr_vid;
  if (vid1 > vid2) {
    vid1 = vid2;
    vid2 = _vid;
  }

  GEdgeContainerIter<GCost> it = this->_g._edges.find(std::make_pair(vid1, vid2));
  assert(it != this->_g._edges.cend());

  DLOG(INFO) << "Iterator eState: " 
             << " _vid= " << this->_vid 
             << " _nbr_vid= " << this->_nbr_vid << endl;

  GEdgeIterConstReference<GCost> edge{*it};
  DLOG(INFO) << "Iterator Value Returned: " 
             << edge.first.first << " " << edge.first.second << " " << edge.second;

  return (*it);
}

template <typename GCost>
GEdgeCIter<GCost> 
Graph<GCost>::ecbegin(GVertexId vid) const { 
  return GEdgeCIter<GCost>(*this, vid, 0); 
}

template <typename GCost>
GEdgeCIter<GCost> 
Graph<GCost>::ecend(GVertexId vid) const { 
  return GEdgeCIter<GCost>(*this, vid, kGMaxVertexId<GCost>()); 
}

// Trigger instantiation of Graph Iterators <uint32_t>
template class Graph<uint32_t>; // Graph Iterator Members
template class GVertexCIter<uint32_t>;
template class GEdgeCIter<uint32_t>;


//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
