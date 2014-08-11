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
#include <iostream>
#include <vector>           // vector
// Standard C Headers
#include <cassert>          // assert
//Google Libraries
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/graph.h"
#include "utils/graph_iter.h"
#include "utils/mst_prim.h"
#include "utils/prio_q.h"
#include "utils/tree.h"

using namespace std;

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------

// PQElem are units store in prioirty Q while running the MST Prim 
// algorithm: 
template <typename GCost>
using PQElem = typename std::pair<GVertexId, TreeElem<GCost>>;

// Priority Q: Keeps a list of candidate edges that are candidates for 
// Minimum Spanning Tree. 
// Initial Condition: 
//   1. All vertices are at infinite cost in the spanning tree
//   2. All vertices (v1) have seed vertex as immediate parent with infinite cost

// helper function to allow chained cout cmds: example
template <typename GCost>
static inline ostream& 
operator << (std::ostream& os, const PQElem<GCost>& elem) {
  os << "[" << elem.first << "]: <" 
     << elem.second.first << "," << elem.second.second << ">";
  return os;
}

// Common Useful Function Objects
template <typename GCost>
class MinCostVertex {
 public:
  // Vertex v1 has "lower" prio than v2 if v1 has "higher" cost than v2
  bool operator() (const PQElem<GCost>& e1, const PQElem<GCost>& e2) const {
    return (e1.second.second > e2.second.second);
  }
 protected:
 private:
};

template <typename GCost>
class EqRefVertexId {
 public:
  // Vertex v1 is "equal" from a "find" perspective if the reference (first)
  // vertex ids are the same
  bool operator() (const PQElem<GCost>& e1, const PQElem<GCost>& e2) const {
    return (e1.first == e2.first);
  }
 protected:
 private:
};

// Creates a MSTPrim class that runs Prim's algorithm on Graph g 
// and creates a tree.
template <typename GCost>
MSTPrim<GCost>::MSTPrim(const Graph<GCost> &g): _g(g), _mst(g) {
  // priority Q: keeps all vertices that are candidates but not yet 
  // a part of the Minimum Spanning Tree
  PrioQ <PQElem<GCost>, MinCostVertex<GCost>, EqRefVertexId<GCost>> pq;

  // 1. Initiatlize Data Structures:
  // 1.a. mst = {} i.e. parents of all vertices is vertex 0 with INFINITE cost
  // 1.B. pq = {all vertices of graph with cost 0 except seed vertex}
  // Start with adding all the vertices to pq with infinite cost except
  // the "seed" graph: we will add vertex 0 with cost 0.
  // Add the rest of the vertices with cost "infinity"
  PQElem<GCost> elem; 
  TreeElem<GCost> e;
  GVertexId vid{0};
  for (auto it = _mst.begin(); it != _mst.end(); ++it, ++vid) {    
    e = make_pair(0, kGInfinityCost<GCost>());
    *it = e;
    if (vid == 0)
      e.second = 0;
    elem = make_pair(vid, e);
    pq.insert_elem(elem);
  }

  // 2. Iterate until pq is empty or we do not find any vertex that is 
  //    at an infinite cost (i.e. not reachable at all)
  // a. mst <- pick the vertex with the minimal edge cost from pq.
  // b. Update pq: vertex in pq with lower cost edge to mst 
  //    than what is currently in pq
  uint32_t num_edges = _g.get_num_edges();
  
  uint32_t num_iter=0;
  while (pq.get_size() > 0) {
    // 2.a. mst <- pick the vertex with the minimal edge cost from pq.
    PQElem<GCost> elem(pq.get_top());
    TreeElem<GCost> e = elem.second;
    GVertexId v=elem.first;
    DLOG(INFO) << "PriQ: size " << pq.get_size() << "-> top elem = [" << v << "]:<" 
               << e.first << "," << e.second << ">";
    DLOG(INFO) << "PriQ State: " << pq;
    // If the cost of the lowest cost edge is infinity then we do not
    // have a spanning tree solution for this tree: terminate the loop
    if (e.second >= kGInfinityCost<GCost>()) {
      DLOG(INFO) << "Graph does have a minimum spanning tree solution";
      break;
    }
    pq.pop_top();
    
    // add the topmost element to the tree
    _mst.at(v) = e;
    
    // 2.b. Update pq: If any nbr vertex in pq has now a lower edge cost 
    //      via the vertex v that was just added to MST, then update
    //      that edge cost for the nbr than what is currently in pq
    // 2.b.i. Traverse all the vertices nbr reachable from v. 
    //        Iterate through all edges of vertex v to all other vertices ov
    for (auto it =_g.ecbegin(v); it != _g.ecend(v); ++it) {
      GEdgeIterConstReference<GCost> edge{*it};
      DLOG(INFO) << "Reference Vertex " << v << ": Examining Edge " 
                 << edge.first.first << " " << edge.first.second << " " 
                 << edge.second << " num_iter " << num_iter << endl;
      
      // sanity check: iteration should terminate in at most 2*E
      assert((num_iter++) < (num_edges << 1));
      assert((edge.first.first == v) || (edge.first.second == v));
      
      // 2.b.ii. Identify the other vertex nbr reachable through 
      //         v with associated cost
      //         If nbr is already one among the minimum span vertices 
      //         we can ignore this vertex
      GVertexId nbr = (edge.first.first == v) ? 
                      edge.first.second : edge.first.first;
      e = _mst.at(nbr);
      if (e.second < kGInfinityCost<GCost>())
        continue;
      
      // 3. Compute the cost of reaching nbr in the MST that now includes v
      // 3.a. If this vertex nbr is reachable for the first time (was not even
      //      visible in pri Q, then add this vertex to the pri Q with the 
      //      reachability cost.
      GCost ncost = edge.second;
      bool match;
      PQElem<GCost> nbr_elem(std::make_pair(nbr, std::make_pair(v, ncost)));
      PQElem<GCost>& pq_elem = pq.contains_elem(nbr_elem, match);
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

//   Dumps the state of minimum spanning tree in file_name
template <typename GCost>
void MSTPrim<GCost>::output_to_file(string file_name) {
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

// helper function to allow chained cout cmds: example
// cout << "The tree: " << endl << mst << endl << "---------" << endl;
template <typename GCost>
ostream& operator << (ostream& os, const MSTPrim<GCost> &mst) {
  // Identify seed vertex of the tree
  // as the graph does not allow self referential nodes
  // i.e. an edge from a node N to itself, we designate the seed of the MST
  // tree by having it point to itself with cost 0 
  // similarly: vertex exists in the tree when its cost to the 
  // parent is not Infinity
  GVertexId seed_vid{kGMaxVertexId<GCost>()};
  GCost mst_cost{0};
  GVertexId vid=0;
  for (auto it = mst.cbegin(); it != mst.cend(); ++it, ++vid) {
    if (vid == it->first) {
      assert(it->second == 0);
      seed_vid = vid; // remember the seed vertex
    }
    if (it->second >= kGInfinityCost<GCost>())
      continue;
    mst_cost += it->second; // calculate the total cost of the MST
  }
  assert(seed_vid != kGMaxVertexId<GCost>()); // tree must have a seed vertex

  os << "#***************************#" << endl;
  os << "# MINIMUM SPANNING TREE:    #" << endl;
  os << "#---------------------------#" << endl;
  os << "# FORMAT:                   #" << endl;
  os << "#+++++++++++++++++++++++++++#" << std::endl;
  os << "# MST Prim Seed Vertex: " << seed_vid << " #" << std::endl;
  os << "# MST Prim Cost: " << mst_cost << "       #" << std::endl;
  os << "#+++++++++++++++++++++++++++#" << std::endl;
  os << "#= num_vertices             #" << endl;
  os << "#=== vid par_vid edge_cost  #" << endl; 
  os << "#############################" << endl;
  os << mst.get_num_vertices() << endl;
  vid=0;
  for (auto it = mst.cbegin(); it != mst.cend(); ++it, ++vid) {
    if ((vid == it->first) || (it->second >= kGInfinityCost<GCost>()))
      continue;
    os << vid << " " << it->first << " " << it->second << endl;
  }
  os << "#############################" << endl;

  os << "#***************************#" << endl;

  return os;
}

// Trigger instantiation
template class MSTPrim<uint32_t>;
template ostream& operator << <uint32_t>(ostream& os, const MSTPrim<uint32_t> &mst);

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
