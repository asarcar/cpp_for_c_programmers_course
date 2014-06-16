// Copyright 2014 Arijit Sarcar Projects Inc.
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

// Standing C++ Headers
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <vector>       // std::vector
#include <sstream>      // std::stringstream
#include <string>       // stoi stoi stod
#include <exception>    // std::exception
// Standing C Headers
#include <cstdlib>      // std::exit std::EXIT_FAILURE
// Google Headers
#include <gflags/gflags.h>  // Parse command line args and flags
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/graph.h"
#include "utils/graph_iter.h"
#include "utils/init.h"

using namespace hexgame;
using namespace hexgame::utils;
using namespace std;

// Flag Declarations
DECLARE_string(input_file);
DECLARE_string(output_file);
DECLARE_string(seed_v_str);

int main(int argc, char **argv) {
  Init::InitEnv(&argc, &argv);

  try {
    DLOG(INFO) << "Test Program Begins: ..." << std::endl 
               << "------------------------" << std::endl;

    // Prep the output file & read the seed_vertices for traversal 
    std::ofstream op;
    op.open(FLAGS_output_file, std::ios::out);
    assert(!op == false);
    Graph::SeedVertices seed_v;
    Graph::gvertexid_t vid;
    std::istringstream iss(FLAGS_seed_v_str);
    while (iss.peek() != EOF) {
      iss >> vid;
      seed_v.push_back(vid);
    }

    // Iterate BFS on the graph: provide the output to output file
    Graph g(FLAGS_input_file);    
    DLOG(INFO) << g << std::endl;
    op << g << std::endl;

    op << "-----------------------------------" << std::endl;
    op << "  Seed Vertices: ";
    for (auto &vid : seed_v) {
      op << " " << vid;
      DLOG(INFO) << " " << vid;
    }
    op << std::endl;
    
    op << "--------BFS ORDER TRAVERSAL--------" << std::endl;
    uint32_t i=0;
    for (auto it = g.vcbegin(Graph::VertexIterType::BFS_ORDER, 
                             seed_v);
         it != g.vcend(Graph::VertexIterType::BFS_ORDER); 
         ++it, ++i) {
      DLOG(INFO) << " " << *it;
      op << " " << *it;
      if ((i+1)%8 == 0)
        op << std::endl;
    }
    op << std::endl;

    op << "--------DFS ORDER TRAVERSAL--------" << std::endl;
    i=0;
    for (auto it = g.vcbegin(Graph::VertexIterType::DFS_ORDER, 
                             seed_v);
         it != g.vcend(Graph::VertexIterType::DFS_ORDER); 
         ++it, ++i) {
      DLOG(INFO) << " " << *it;
      op << " " << *it;
      if ((i+1)%8 == 0)
        op << std::endl;
    }
    op << std::endl;

    op << "-----------------------------------" << std::endl;                      
    DLOG(INFO) << "Test Program Ends: ..." << std::endl
               << "************************" << std::endl; 
  }
  catch (const std::string s) {
    std::cerr << "Exception caught: " << s << std::endl;
  }
  catch (std::exception e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }  

  return 0;
}

DEFINE_string(input_file, "data/input.txt", 
              "Input file with graph input data");
static bool ValidateInputFile(const char* flagname, const std::string& file_name) {
  std::string s(flagname);
  std::ifstream ip;
  ip.open(file_name, std::ios::in);
  if (!ip) {
    std::string s(flagname);
    std::cerr << "Invalid value for --" << s << ": " << file_name << std::endl;
    return false;
  }
  return true;
}
static const bool 
string_dummy = google::RegisterFlagValidator(&FLAGS_input_file, 
                                             &ValidateInputFile);

DEFINE_string(output_file, "bfs_dfs_output.txt",
              "Output file to store graph output data");

DEFINE_string(seed_v_str, "0",
              "Seed Vertices to seed traversal algorithms");
