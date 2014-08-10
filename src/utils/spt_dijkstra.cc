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
#include <vector>           // vector
// Standard C Headers
#include <cassert>          // assert
//Google Libraries
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/graph.h"
#include "utils/graph_iter.h"
#include "utils/prio_q.h"
#include "utils/spt_dijkstra.h"
#include "utils/tree.h"


using namespace std;

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------
// Priority Q: Keeps a list of candidate edges that are candidates for 
// Shortest Path Tree. 
// Initial Condition: 
//   1. All vertices are at infinite cost in the spanning tree
//   2. All vertices (v1) have root vertex as immediate parent with infinite cost

// PQElem are units store in prioirty Q while running the MST Prim 
// algorithm: 
using PQElem = std::pair<Graph::gvertexid_t, SPTDijkstra::SPTElem>;

// helper function to allow chained cout cmds: example
static inline std::ostream& operator << (std::ostream& os, const PQElem& elem) {
  os << "{(" << elem.first << "->" << elem.second.first << ")" 
     << elem.second.second << "}";
  return os;
}


// Common Useful Function Objects
class MinCostVertex {
 public:
  // Vertex v1 has "lower" prio than v2 if v1 has "higher" cost than v2
  bool operator() (const PQElem& e1, const PQElem& e2) const {
    return (e1.second.second > e2.second.second);
  }
 protected:
 private:
};

class EqRefVertexId {
 public:
  // Vertex v1 is "equal" from a "find" perspective if the reference (first)
  // vertex ids are the same
  bool operator() (const PQElem& e1, const PQElem& e2) const {
    return (e1.first == e2.first);
  }
 protected:
 private:
};

//   run_spt_dijkstra with srv_vid as root of the tree
//     arg1: root vertex id
void SPTDijkstra::run_spt_dijkstra(Graph::gvertexid_t root_vid) {
  if (root_vid >= _g.get_num_vertices()) {
    DLOG(ERROR) << "Graph has " << _g.get_num_vertices() 
                << " vertices: spt_dijkstra called with vertex_id " 
                << root_vid;
    throw std::out_of_range("VertexId exceeds # of vertices in graph");
  }
  
  // priority Q: keeps all vertices that are candidates but not yet 
  // a part of the Minimum Spanning Tree
  PrioQ <PQElem, MinCostVertex, EqRefVertexId> pq;

  // 1. Initiatlize Data Structures:
  // 1.a. spt = {} i.e. parents of all vertices is vertex 0 with INFINITE cost
  // 1.B. pq = {all vertices of graph with cost 0 except root vertex}
  // Start with adding all the vertices to pq with infinite cost except
  // the "root" graph: we will add vertex root_id with cost 0.
  // Add the rest of the vertices with cost "infinity"
  PQElem elem; 
  SPTElem e;
  Graph::gvertexid_t vid = 0;
  for (auto it = _spt.begin(); it != _spt.end(); ++it, ++vid) {    
    e = make_pair(root_vid, Graph::kInfinityCost);
    *it = e;
    if (vid == root_vid)
      e.second = 0;
    elem = std::make_pair(vid, e);
    pq.insert_elem(elem);
  }

  // 2. Iterate until pq is empty or we do not find any vertex that is 
  //    at an infinite cost (i.e. not reachable at all)
  // a. spt <- pick the vertex with the minimal edge cost from pq.
  // b. Update pq: If any nbr vertex in pq has now a lower path cost 
  //    via the vertex that was just added to SPT, then update
  //    that path cost for the nbr than what is currently in pq
  uint32_t num_edges = _g.get_num_edges();
  uint32_t num_iter=0;

  while (pq.get_size() > 0) {
    // 2.a. spt <- pick the vertex with the minimal edge cost from pq.
    PQElem elem(pq.get_top());
    SPTElem e = elem.second;
    
    Graph::gvertexid_t v=elem.first; // current vertex examined
    Graph::gcost_t vcost=e.second;   // path cost of reaching v

    DLOG(INFO) << "PriQ: size " << pq.get_size() << "-> top elem = [" << v << "]:<" 
               << e.first << "," << e.second << ">";
    DLOG(INFO) << "PriQ State: " << pq;

    // If the cost of the lowest cost edge is infinity then we do not
    // have a tree solution that covers all vertices (i.e. graph is 
    // partitioned when traversing from root_vid: terminate the loop
    if (vcost >= Graph::kInfinityCost) {
      DLOG(INFO) << "Graph does NOT have a shortest path tree that "
                 << "covers all nodes of the tree";
      break;
    }
    pq.pop_top();
    
    // add the topmost element to the tree
    _spt.at(v) = e;
    
    // 2.b. Update pq: If any nbr vertex in pq has now a lower path cost 
    //      via the vertex v that was just added to SPT, then update
    //      that path cost for the nbr than what is currently in pq
    // 2.b.i. Traverse all the vertices nbr reachable from v. 
    //        Iterate through all edges of vertex v to all other vertices ov
    for (auto it =_g.ecbegin(v); it != _g.ecend(v); ++it) {
      Graph::evalue_type edge = *it;
      DLOG(INFO) << "Reference Vertex " << v << ": Examining Edge " 
                 << edge << " num_iter " << num_iter << std::endl;
      
      // sanity check: iteration should terminate in at most 2*E
      assert((num_iter++) < (num_edges << 1));
      assert((edge.first.first == v) || (edge.first.second == v));
      
      // 2.b.ii. Identify the other vertex nbr reachable through
      //         v with associated cost. 
      //         If nbr is already one among 
      //         the shortest path tree vertices we can ignore this vertex
      Graph::gvertexid_t nbr = (edge.first.first == v) ? 
                               edge.first.second : edge.first.first;
      e = _spt.at(nbr);
      if (e.second < Graph::kInfinityCost)
        continue;
      
      // 3. Compute the cost of reaching nbr in the SPT that now includes v
      // 3.a. If this vertex nbr is reachable for the first time (was not even
      //      visible in pri Q, then add this vertex to the pri Q with the 
      //      reachability cost.
      Graph::gcost_t ncost = edge.second + vcost;
      bool match;
      PQElem nbr_elem(std::make_pair(nbr, std::make_pair(v, ncost)));
      PQElem& pq_elem = pq.contains_elem(nbr_elem, match);
      if (match == false) {
        pq.insert_elem(nbr_elem);
        continue;
      }
      
      DLOG(INFO) << "\tNeighbor Vertex: " << nbr << " Cost " << ncost 
                 << " past cost: " << pq_elem.second.second << std::endl;

      // 3.b.1. If the cost of reaching nbr via v is not less 
      //        than past estimated cost via some other 
      //        destination we can ignore this path: goto next iteration
      if (ncost >= pq_elem.second.second)
        continue;
      
      // 3.b.2. Update the new cost in the pri Q.
      pq.chg_val(pq_elem, nbr_elem);
    }
  }

  return;
}

//   get_path_size
//     arg1: source vertex id
//     arg2: destination vertex id
//     Return: path_cost from source to destination vertex id
Graph::gcost_t 
SPTDijkstra::get_path_size(Graph::gvertexid_t vid1, 
                           Graph::gvertexid_t vid2) {
  // We first retrieve the shortest path from vid1 to all vertices
  // Then we just walk the vector until we hit vertex vid2

  // For efficiency we could have terminated the loop as soon as vid2 is 
  // reached in the shortest path loop instead of trying to find the shortest 
  // path to all other vertices.
  // Here: I am being lazy and just trying to reuse the code of 
  // run_spt_dijkstra :-)
  this->run_spt_dijkstra(vid1);

  for (auto it = this->cbegin(); it != this->cend(); ++it) {
    if (it->first == vid2) 
      return (it->second);
  } 

  // Set path cost to "INFINITY" as the vid2 is not
  // reachable from vid1
  return Graph::kInfinityCost;
}

//   get_avg_path_size_for_vertex
//     arg1: source vertex id
//     return: avg path size from given vertex id to all other reachable 
//             vertices
double SPTDijkstra::get_avg_path_size_for_vertex(Graph::gvertexid_t vid) {
  // We first retrieve the shortest path from vid to all vertices
  // Then we just walk the vector summing up all cost and divide
  // by the number of entries in the vector 
  this->run_spt_dijkstra(vid);

  Graph::gcost_t path_cost = 0;
  Graph::gvertexid_t num_vertices = 0;
  for (auto it = this->cbegin(); it != this->cend(); ++it, ++num_vertices) {
    // skip over the source vertex itself as that is reachable at cost 0
    // skip over vertices that are not reachable at all
    if ((it->first == vid) || (it->second == Graph::kInfinityCost))
      continue;
    path_cost += it->second;
  }

  // For pathological case where there are no edges from the vertex
  // return MAX possible value
  if (num_vertices == 0)
    return Graph::kInfinityCost;

  return (static_cast<double>(path_cost)/num_vertices);
}

//   get_avg_path_size
//     return: avg path size from all vertices to all other reachable 
//             vertices
double SPTDijkstra::get_avg_path_size(void) {
  // Retrieve all vertices in the graph
  // Keep a running total of path_size from each source vertex to 
  // all destination vertices.
  // Compute the average over all the data

  uint32_t num = 0;
  Graph::gcost_t path_cost = 0;

  for (Graph::gvertexid_t vid=0; vid < _g.get_num_vertices(); ++vid) {
    this->run_spt_dijkstra(vid);
    for (auto it = this->cbegin(); it != this->cend(); ++it, ++num) {
      // skip over the source vertex itself as that is reachable at cost 0
      // skip over vertices that are not reachable at all
      if ((it->first == vid) || (it->second == Graph::kInfinityCost))
        continue;
      path_cost += it->second;
    }
  }

  // For pathological case where there are no edges from the vertex
  // return MAX possible value
  if (num == 0)
    return Graph::kInfinityCost;

  return (path_cost/num);
}

// Dumps the state of the SPT in file_name
void SPTDijkstra::output_to_file(std::string file_name) {
  std::ofstream ofp;

  ofp.open(file_name, std::ios::out);
  if (!ofp) {
    std::stringstream ss;
    ss << "Can't open output file " << file_name;
    throw ss.str();
  }
  // Dump the state of Tree in the output stream
  ofp << *this;
  
  ofp.close();

  return;
}

// helper function to allow chained output cmds: example
// cout << "The SPT: " << endl << spt << endl << "---------" << endl;
std::ostream& operator << (std::ostream& os, const SPTDijkstra &spt) {
  // Identify root vertex of the tree
  // as the graph does not allow self referential nodes
  // i.e. an edge from a node N to itself, we designate a root of the SPT
  // tree by having it point to itself with cost 0 
  // similarly: vertex exists in the tree when its cost to the 
  // parent is not Infinity
  Graph::gvertexid_t root_vid{Graph::kMaxVertexId};
  Graph::gcost_t tot_path{0};
  Graph::gvertexid_t vid=0;
  for (auto it = spt.cbegin(); it != spt.cend(); ++it, ++vid) {
    if (vid == it->first) {
      assert(it->second == 0);
      root_vid = vid; // remember the root vertex
    }
    if (it->second >= Graph::kInfinityCost)
      continue;
    tot_path += it->second; // calculate the total cost of the MST
  }
  assert(root_vid != Graph::kMaxVertexId); // tree must have a seed vertex

  os << "#*******************************#" << std::endl;
  os << "# SHORTEST PATH TREE OUTPUT     #" << std::endl;
  os << "#-------------------------------#" << std::endl;
  os << "# FORMAT:                       #" << std::endl;
  os << "#+++++++++++++++++++++++++++++++#" << std::endl;
  os << "# SPT Dijkstra root vertex: " << root_vid << " #" << std::endl;
  os << "# SPT Total path cost: " << tot_path << "     #" << std::endl;
  os << "#+++++++++++++++++++++++++++++++#" << std::endl;
  os << "#= num_vertices                 #" << std::endl;
  os << "#=== vid parent_vid path_cost   #" << std::endl;
  os << "#################################" << std::endl;
  os << spt.get_num_vertices() << std::endl;
  
  vid = 0;
  for (auto it = spt.cbegin(); it != spt.cend(); ++it, ++vid) {
    if ((vid == it->first) || (it->second >= Graph::kInfinityCost))
      continue;
    os << vid << " " << it->first << " " << it->second << std::endl;
  }
  os << "#################################" << std::endl;
  
  os << "#*******************************#" << std::endl;

  return os;
}

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
