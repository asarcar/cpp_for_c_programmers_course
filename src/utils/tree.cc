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
#include <fstream>          // std::ifstream & std::ofstream
#include <iostream>
#include <sstream>          // std::stringstream
#include <exception>        // throw
// Standard C Headers
#include <cassert>          // assert()
// Google Headers
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/tree.h"

using namespace std;

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------
template <typename GCost>
void Tree<GCost>::output_to_file(string file_name) {
  ofstream ofp;

  ofp.open(file_name, ios::out);
  if (!ofp) {
    stringstream ss;
    ss << "Can't open output file " << file_name;
    throw ss.str();
  }
  // Dump the state of Tree in the output stream
  ofp << *this;
  
  ofp.close();

  return;
}


// helper function to allow chained output cmds: example
// cout << "The tree: " << endl << t << endl << "---------" << endl;
template <typename GCost>
ostream& operator <<(ostream& os, const Tree<GCost>& t) {
  os << "#*****************************#" << endl;
  os << "# TREE OUTPUT                 #" << endl;
  os << "#-----------------------------#" << endl;
  os << "# FORMAT:                     #" << endl;
  os << "#= num_vertices               #" << endl;
  os << "#@@@ vid parent_vid info      #" << endl;
  os << "###############################" << endl;
  os << t._g.get_num_vertices() << endl;
  
  uint32_t i=0;
  for (auto it = t.cbegin(); it != t.cend(); ++it) {
    // as the graph doe not allow self referential nodes
    // i.e. an edge from a node N to itsel, we designate a root of a tree 
    // by having it point to itself: we skip over that node
    if (i == it->first) {
      i++;
      continue;
    }
    os << i << " " << it->first << " " << it->second << endl;
    i++;
  }
  os << "###############################" << endl;
  
  os << "#*****************************#" << endl;

  return os;
}

// Trigger instantiation
template class Tree<uint32_t>;
template ostream& operator << <uint32_t>(ostream& os, const Tree<uint32_t> &t);

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
