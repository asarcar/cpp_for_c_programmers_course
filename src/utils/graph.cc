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
#include <algorithm>        // std::max()
#include <fstream>          // std::ifstream & std::ofstream
#include <functional>       // std::bind
#include <iostream>
#include <random>           // std::distribution, random engine, ...
#include <sstream>          // std::stringstream
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
// Contructor
// Init an undirected or directed graph (based on type) with num_vertices.
// The edges created with probability of edge_density.
// The edge cost is chosen with equal prob in range mix to max range
template <typename GCost>
Graph<GCost>::Graph(const GEdgeType type,
                    const uint32_t num_vertices,
                    const double   edge_density, 
                    const GCost  min_distance_range, 
                    const GCost  max_distance_range,
                    // true when called from automated test scripts
                    const bool     auto_test) : 
    _type(type), _num_vertices{num_vertices}, _adjmap(num_vertices) {
  // 1. Loop through each source vertex: 
  // 2. Loop through each "new" edge candidate
  // 2.a. Discard candidate edge if prob is out of range.
  // 2.b. Otherwise: Set edge cost based on random number & add edge 

  GVertexId vid1, vid2;
  GVertexId min2 = 0;

  // For repeatable & predictable data generation (auto_test == TRUE) we 
  // generate the same seeds for random number generation to ensure 
  // that same graph (same edges & edge costs) is generated every time
  uint32_t cost_seed = (auto_test == true) ? 
                       Graph<GCost>::kFixedCostSeedForRandomEngine:
                       std::random_device{}();
  auto cost_fn = 
    std::bind(std::uniform_int_distribution<uint32_t>
              {min_distance_range, max_distance_range}, 
              std::default_random_engine{cost_seed}); // C++11 only    
  uint32_t create_edge_seed = (auto_test == true) ? 
                              Graph<GCost>::kFixedEdgePresenceSeedForRandomEngine:
                              std::random_device{}();
  // scaled_fn = generate a random_number between 0 and 2**20 (~= 1 million)
  auto scaled_fn = 
      std::bind(std::uniform_int_distribution<uint32_t>{0, (1 << 20)},
                std::default_random_engine{create_edge_seed}); // C++11 only    
  // prob = scaled_fn/(2**20) => scaled_fn = prob*(2**20)
  uint32_t scaled_val = edge_density*(1 << 20);

  // 1. Loop through each source vertex: 
  for (vid1 = 0; vid1 < this->get_num_vertices(); vid1++) {
    //    Undirected graph: edge candidate - (vi, vj) i <= j
    min2 = (this->_type == GEdgeType::UNDIRECTED) ? vid1: 0;

    // 2. Loop through each "new" edge candidate
    for (vid2 = min2; vid2 < this->get_num_vertices(); vid2++) {
      // assume our graph will not have self-referential edges
      // i.e. edge (vi,vj) does not exist when i == j
      if (vid1 == vid2) 
        continue;

        // 2.a. Discard candidate edge if prob is out of range.
      if (scaled_fn() >= scaled_val)
        continue;

      // 2.b. Otherwise: Set edge cost based on random number & add edge
      add_edge(vid1, vid2, cost_fn());
    } 
  }

  return;
}

// Init an undirected graph (note that sample file data provided in HW does not 
// specify directed or undirected: hence we will always assume undirected graph)
// based on content of the file
template <typename GCost>
Graph<GCost>::Graph(string file_name): 
    _type(GEdgeType::UNDIRECTED) {
  ifstream inp;
  string line;

  inp.open(file_name, std::ios::in);
  if (!inp) {
    ostringstream oss;
    oss << "Can't open input file " << file_name;
    throw oss.str();
  }

  // Read # of Vertices & init internal structures
  GVertexId num_v{0}; // C++11 style initializer
  
  while (getline(inp, line)) {
    // Skip over commented lines of the file
    if (line.at(0) == '#')
      continue;
    stringstream ss(line);
    ss >> num_v;
    break;
  }
  if ((num_v == 0) || (num_v > kGMaxVertexId<GCost>())) {
    ostringstream oss;
    oss << "File " << file_name << ": bad format: num_v = " << num_v;
    throw oss.str();
  }

  _num_vertices = num_v;
  _adjmap.resize(num_v);

  // Read the edges and set cost
  GVertexId vid1{0}, vid2{0}; 
  GCost cost{0};

  while (getline(inp, line)) {
    // Skip over commented lines of the file
    if (line.at(0) == '#')
      continue;
    stringstream ss(line);
    ss >> vid1 >> vid2 >> cost;
    if ((vid1 >= num_v) || (vid2 >= num_v) || (cost == 0)) {
      ostringstream oss;
      oss << "File: " << file_name << ": bad format (num_v) " << num_v << endl;
      oss << "Line: " << line << endl;
      oss << "vid1 " << vid1 << ": vid2 " << vid2 << ": cost " << cost << endl;
      throw oss.str();
    }
    add_edge(vid1, vid2, cost);
  }

  inp.close();
  
  return;
}


// add (g, x, y): adds to G the edge from x to y, if it is not there.
// Creates an edge (adjacency) in the graph with edge and (optional) value
template <typename GCost>
void Graph<GCost>::add_edge(GVertexId v1, GVertexId v2, GCost value) {
  // Establish adjacency & add the edge to graph
  GEdgeId eid = make_pair(v1, v2);
  // For undirected graph: we always index the edge as (vi, vj) i<=j
  if ((this->_type == GEdgeType::UNDIRECTED) && (v1 > v2)) 
    eid = make_pair(v2, v1);
  
  DLOG(INFO) << "Creating edge <" << eid.first << "," << eid.second 
             << "> with cost " << value;

  set_adjmap(eid); // adjacency established
  // new edge: use the [] operator for unordered map
  _edges[eid] = value; // add edge; update cost 

  return;
}

// delete (G, x, y): removes the edge from x to y, if it is there.
// Removes an edge (adjacency) in the graph with edge and (optional) value
template <typename GCost>
void Graph<GCost>::del_edge(GVertexId v1, GVertexId v2) {
  // Clear adjacency & remove the edge to graph
  GEdgeId eid = make_pair(v1, v2);
  // For undirected graph: we always index the edge as (vi, vj) i<=j
  if ((this->_type == GEdgeType::UNDIRECTED) && (v1 > v2)) 
    eid = make_pair(v2, v1);

  clr_adjmap(eid); // adjacency cleared
  _edges.erase(eid); // remove edge

  return;
}

// set_edge_value (G, x, y, v): sets the value associated to the edge (x,y) to v.
// bad arg check: non existent edge automatically checked by container
template <typename GCost>
void Graph<GCost>::set_edge_value(GVertexId v1, GVertexId v2, GCost value) {
  // Validate edge exists: update edge cost
  GEdgeId eid = make_pair(v1, v2);
  // For undirected graph: we always index the edge as (vi, vj) i<=j
  if ((this->_type == GEdgeType::UNDIRECTED) && (v1 > v2)) 
    eid = make_pair(v2, v1);

  if (isset_adjmap(eid) != true)
    return;
  // update edge cost: use at() operator to be safe
  _edges.at(eid) = value; 

  return;
}

// get_edge_value( G, x, y): returns the value associated to the edge (x,y).
// non existent edge: return infinity cost
// bad arg check: non existent edge automatically checked by container
template <typename GCost>
GCost Graph<GCost>::get_edge_value(GVertexId v1, GVertexId v2) const {
  GEdgeId eid = std::make_pair(v1, v2);
  // Undirected Graph: Edges stored as (vi, vj) i <= j
  if ((this->_type == GEdgeType::UNDIRECTED) && (v1 > v2))
    eid = std::make_pair(v2, v1);

  if (isset_adjmap(eid) != true)
    return kGInfinityCost<GCost>();
  // edge must exist: use the at() operator
  return this->_edges.at(eid);
}

// Dumps the graph to the file "file_name"
template <typename GCost>
void Graph<GCost>::output_to_file(std::string file_name) {
  ofstream ofp;

  ofp.open(file_name, ios::out);
  if (!ofp) {
    stringstream ss;
    ss << "Can't open output file " << file_name;
    throw ss.str();
  }

  ofp << *this;
  
  ofp.close();

  return;
}

// Graph Output
// helper function to allow chained cout cmds: example
// cout << "The graph: " << endl << g << endl << "---------" << endl;
template <typename GCost>
ostream& operator << (ostream& os, const Graph<GCost> &g) {
  os << "#************************#" << endl;
  os << "# GRAPH:                 #" << endl;
  os << "#------------------------#" << endl;
  os << "# FORMAT:                #" << endl;
  os << "# num_vertices           #" << endl;
  os << "# svid dvid edge_cost    #" << endl; 
  os << "#^^^^^^^^^^^^^^^^^^^^^^^^#" << endl;
  os << "# GEdgeType: " << ((g._type == GEdgeType::UNDIRECTED) ? "U" : "D")
     << "#" << endl;
  os << "# #V: " << g.get_num_vertices() 
     << ";" << " #E(uniq): " << g.get_num_edges() << "#" << endl;
  os << "##########################" << endl;
  os << g.get_num_vertices() << endl;

  // Iterate through all vertices of the graph
  for (GVertexId vid=0; vid <g.get_num_vertices(); ++vid) {
    // Iterate through all edges of the given vertex
    for (GEdgeCIter<GCost> it = g.ecbegin(vid); it != g.ecend(vid); ++it) {
      GEdgeIterConstReference<GCost> edge{*it};
      os << edge.first.first << " " << edge.first.second << " " << edge.second << endl;    
    }
  }    

  os << "#************************#" << endl;
  
  return os;
}

// Trigger instantiation
template class Graph<uint32_t>;
template ostream& operator << <uint32_t>(ostream& os, const Graph<uint32_t> &g);

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
