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
#include <algorithm>        // count_if
#include <exception>        // throw
#include <fstream>          //i/ofstream
#include <iostream>
#include <sstream>          //i/ostringstream
// Standard C Headers
#include <cassert>          // assert()
// Google Headers
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/find_merge.h"

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------
// Forward Declarations 
// constexpr definitions
constexpr uint32_t FindMerge::MIN_NODES;
constexpr uint32_t FindMerge::DEFAULT_NUM_NODES;
constexpr int FindMerge::DEFAULT_PARENT_NODE_IDX;
// End of Forward Declarations

FindMerge::FindMerge(std::string file_name) {
  std::ifstream inp;
  std::string line;

  inp.open(file_name, std::ios::in);
  if (!inp) {
    std::ostringstream oss;
    oss << "Can't open input file " << file_name;
    throw oss.str();
  }

  // Read Num of Nodes
  uint32_t num_nodes{0}; 
  while (getline(inp, line)) {
    // Skip over commented lines of the file
    if (line.at(0) == '#')
      continue;
    std::stringstream ss(line);
    ss >> num_nodes; 
    break;
  }

  uint32_t num_edges{0}; 
  while (getline(inp, line)) {
    // Skip over commented lines of the file
    if (line.at(0) == '#')
      continue;
    std::stringstream ss(line);
    ss >> num_edges; 
    break;
  }
  
  if ((num_nodes == 0) || 
      (num_nodes < FindMerge::MIN_NODES) ||
      (num_edges == 0)) {
    std::ostringstream oss;
    oss << "File " << file_name 
        << ": bad format: num_nodes = " << num_nodes 
        << " num_edges = " << num_edges;
    throw oss.str();
  }

  std::vector<std::pair<uint32_t,uint32_t>> edges(num_edges);
  uint32_t i=0;
  char ch1, ch2, ch3;
  while (getline(inp, line)) {
    // Skip over commented lines of the file
    if (line.at(0) == '#')
      continue;
    if (i >= num_edges) {
      ++i;
      continue;
    }
    std::istringstream iss(line);
    iss >> ch1 >> edges.at(i).first >> ch2 >> edges.at(i).second >> ch3;
    DLOG(INFO) << "Edge [" << i << "] entered: (" 
               << edges.at(i).first << "," << edges.at(i).second 
               << ")" << std::endl;
    if (ch1 != '(' || ch2 != ',' || ch3 != ')' ||
        edges.at(i).first >= num_nodes ||
        edges.at(i).second >= num_nodes) {
      std::ostringstream oss;
      oss << "File: " << file_name << ": bad format " 
          << "Edge [" << i << "] Format Error: should be (i,j)";
      throw oss.str();
    }
    ++i;
  }

  if (i != num_edges) {
    std::ostringstream oss;
    oss << "File " << file_name << ": bad format" 
        << ": num_edges " << num_edges 
        << " != " << i << " edges specified";
    throw oss.str();
  }

  // Now that we know all entries are appropriate 
  // set the vector size appropriately and execute the
  // merge find operation based on edges
  _v.resize(num_nodes, FindMerge::DEFAULT_PARENT_NODE_IDX);
  for (auto &edge : edges) {
    this->merge_set(edge.first, edge.second);
  }

  inp.close();
  
  return;
}

// Traverse up the node hierarchy to get the highest ancestor
// Note: The set count is the -ve of the number in the highest ancestor
int FindMerge::find_set(uint32_t node_idx) {
  assert(node_idx >= 0 && node_idx < _v.size());
  
  int idx, gf_idx;
  // identify the set_id uniquely identified by the highest ancestor index
  // traverse up from the node to the highest ancestor (root) of the tree
  for (idx = node_idx; _v.at(idx) >= 0; idx = _v.at(idx)) {
    DLOG(INFO) << "find_set: node_idx " << node_idx 
               << ": idx " << idx << ": _v[idx] " << _v.at(idx)
               << ": _v[_v[idx]] " << _v.at(_v.at(idx));
        
    // Limit the height of tree by always relinking node to grandfather
    // Otherwise: we can have a second loop linking all visited nodes 
    // to the root. In practise the linking to grandchildren to root 
    // whenever one can is good enough
    if ((gf_idx = _v.at(_v.at(idx))) >= 0)
      _v.at(idx) = gf_idx;
  }
  
  return idx;
}

// useful when invoked from functions expecting const FM
int FindMerge::find_set(const uint32_t node_idx) const {
  int idx;
  for (idx = node_idx; _v.at(idx) >= 0; idx = _v.at(idx));
  return idx;
}

// merge the two sets and return the identity of the merged set
int FindMerge::merge_set(int node_idx1, int node_idx2) {
  int set_id1 = find_set(node_idx1);
  if (node_idx1 == node_idx2) 
    return set_id1;

  int set_id2 = find_set(node_idx2);
  
  DLOG(INFO) << "merge_set: node_idx1 " << node_idx1 
             << ": node_idx2 " << node_idx2 
             << ": set_id1 " << set_id1
             << ": set_id2 " << set_id2 
             << ": _v[set_id1] " << _v.at(set_id1) 
             << ": _v[set_id2] " << _v.at(set_id2);
        
  // The two sets are already merged
  if (set_id1 == set_id2)
    return set_id1;
  
  // The merge algorithm simply ensures that the 
  // the smaller set merges to the larger set
  // i.e. highest ancestor of smaller set points
  // to the highest ancestor of larger set
  
  // Remember the value stored for the set_id 
  // is the -ve number of children
  
  // As such, the smaller of the two sets has
  // the larger nubmer of children
  if (_v.at(set_id1) < _v.at(set_id2)) { 
    _v.at(set_id1) += _v.at(set_id2);
    _v.at(set_id2) = set_id1;
    return set_id1;
  }
  _v.at(set_id2) += _v.at(set_id1);
  _v.at(set_id1) = set_id2;
  return set_id2;
}

//   Dumps the state of the find_merge in file_name
void FindMerge::output_to_file(std::string file_name) {
  std::ofstream ofp;

  ofp.open(file_name, std::ios::out);
  if (!ofp) {
    std::stringstream ss;
    ss << "Can't open output file " << file_name;
    throw ss.str();
  }

  ofp << *this;
  
  ofp.close();

  return;
}

std::ostream& operator <<(std::ostream& os, 
                          const FindMerge& fm) {
  os << "-----------------------------" << std::endl;
  os << "Num Nodes: " << fm._v.size() << std::endl;
  os << "Array: { ";
  
  std::vector<int> sets;
  for (uint32_t idx = 0; idx < fm._v.size(); ++idx) {
    os << fm._v.at(idx) << " ";
    if (fm._v.at(idx) < 0)
      sets.push_back(idx);
  }
  os << "}" << std::endl;
  
  os  << "SETS (#sets: " << sets.size() << ")" << std::endl
      << "----------------" << std::endl;
  for (int &set_id : sets) {
    os << "  Set ID: " << set_id;
    os << " { ";
    for (uint32_t idx=0; idx<fm._v.size(); ++idx) {
      if (set_id == fm.find_set(idx))
        os << idx << " ";
    } 
    os << "}" << std::endl;
  }
  os << "-----------------------------" << std::endl;
  
  return os;
}

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
