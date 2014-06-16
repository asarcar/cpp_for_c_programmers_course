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
#include <exception>    // std::exception
#include <fstream>      // std::ifstream
#include <iomanip>      // std::boolalpha
#include <iostream>     // std::cout
#include <sstream>      // std::o/istringstream
#include <string>       // stoi stoi stod
#include <vector>       // std::vector
// Standing C Headers
#include <cstdlib>      // std::exit std::EXIT_FAILURE
// Google Headers
#include <gflags/gflags.h>  // Parse command line args and flags
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/find_merge.h"
#include "utils/init.h"

using namespace hexgame;
using namespace hexgame::utils;
using namespace std;

// Flag Declarations
DECLARE_bool(input_from_file);
DECLARE_string(input_file);
DECLARE_string(output_file);
DECLARE_int32(num_nodes);
DECLARE_int32(num_edges);
DECLARE_string(edge_str);

int main (int argc, char **argv)
{
  Init::InitEnv(&argc, &argv);

  DLOG(INFO) << std::boolalpha
             << "find_merge_test called: "
             << "input_from_file " << FLAGS_input_from_file
             << ": input_file " << FLAGS_input_file
             << ": output_file " << FLAGS_output_file 
             << ": num_nodes " << FLAGS_num_nodes 
             << ": num_edges " << FLAGS_num_edges
             << ": edge_str " << FLAGS_edge_str;

  try {
    DLOG(INFO) << "Test Program Begins: ..." << std::endl 
               << "------------------------" << std::endl;
    
    if (FLAGS_input_from_file == false) {
      std::vector<std::pair<uint32_t,uint32_t>> edges(FLAGS_num_edges);
      std::istringstream iss(FLAGS_edge_str);
      char ch1, ch2, ch3;
      for (int i=0; i<FLAGS_num_edges; ++i) {
        iss >> ch1 >> edges.at(i).first >> ch2 >> edges.at(i).second >> ch3;
        DLOG(INFO) << "Edge [" << i << "] entered: (" 
                   << edges.at(i).first << "," << edges.at(i).second 
                   << ")" << std::endl;
        if (ch1 != '(' || ch2 != ',' || ch3 != ')' ||
            edges.at(i).first >= static_cast<uint32_t>(FLAGS_num_nodes) ||
            edges.at(i).second >= static_cast<uint32_t>(FLAGS_num_nodes)) {
          DLOG(ERROR) << "Edge [" << i << "] Format Error: should be (i,j)";
          return (-1);
        }
      }

      FindMerge fm(FLAGS_num_nodes);
      for (auto &edge : edges) {
        fm.merge_set(edge.first, edge.second);
      }
    
      DLOG(INFO) << fm;
      fm.output_to_file(FLAGS_output_file);
    } else {
      FindMerge fm(FLAGS_input_file);
      DLOG(INFO) << fm;
      fm.output_to_file(FLAGS_output_file);
    }
    
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

DEFINE_bool(input_from_file, false, 
            "set when num_nodes, num_edges, and edges are read from file");

DEFINE_string(input_file, "data/find_merge_input.txt",
              "Input file to create find_merge test");
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
inp_file_dummy = google::RegisterFlagValidator(&FLAGS_input_file, 
                                               &ValidateInputFile);

DEFINE_string(output_file, "find_merge_output.txt",
              "Output file to store find_merge output");

DEFINE_int32(num_nodes, 5, 
             "# of nodes over which the find merge algorithm is to run"); 
static bool ValidateNumNodes(const char* flagname, int32_t num) {
  std::string s(flagname);
  if (static_cast<uint32_t>(num) < FindMerge::MIN_NODES) {
    std::cerr << "Invalid value for --" << s << ": " << num
              << ": should be >=" << FindMerge::MIN_NODES << std::endl;
    return false;
  }
  return true;
}
static const bool 
num_nodes_dummy = google::RegisterFlagValidator(&FLAGS_num_nodes, 
                                                &ValidateNumNodes);

DEFINE_int32(num_edges, 5, 
             "# of edges over which to run find merge"); 
static bool ValidateNumEdges(const char* flagname, int32_t num) {
  std::string s(flagname);
  if (num < 1) {
    std::cerr << "Invalid value for --" << s << ": " << num 
              << ": should be >1" << std::endl;
    return false;
  }
  return true;
}
static const bool 
edges_dummy = google::RegisterFlagValidator(&FLAGS_num_edges, 
                                            &ValidateNumEdges);

DEFINE_string(edge_str, "(0,1) (1,2) (0,2) (3,4)",
              "Edge String: edges (snode,dnode) that creates merged sets");
