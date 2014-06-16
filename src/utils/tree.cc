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


namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------
void Tree::output_to_file(std::string file_name) {
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
// cout << "The tree: " << endl << t << endl << "---------" << endl;
std::ostream& operator <<(std::ostream& os, const Tree& t) {
  os << "#*****************************#" << std::endl;
  os << "# TREE OUTPUT                 #" << std::endl;
  os << "#-----------------------------#" << std::endl;
  os << "# FORMAT:                     #" << std::endl;
  os << "#= num_vertices               #" << std::endl;
  os << "#@@@ vid parent_vid info      #" << std::endl;
  os << "###############################" << std::endl;
  os << t._g.get_num_vertices() << std::endl;
  
  uint32_t i=0;
  for (auto it = t.cbegin(); it != t.cend(); ++it) {
    // as the graph doe not allow self referential nodes
    // i.e. an edge from a node N to itsel, we designate a root of a tree 
    // by having it point to itself: we skip over that node
    if (i == it->first) {
      i++;
      continue;
    }
    os << i << " " << it->first << " " << it->second << std::endl;
    i++;
  }
  os << "###############################" << std::endl;
  
  os << "#*****************************#" << std::endl;

  return os;
}

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {
