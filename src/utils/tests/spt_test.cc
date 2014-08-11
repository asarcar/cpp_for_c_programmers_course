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

// Standard C++ Headers
#include <iostream>     // std::cout
#include <limits>       // std::numeric_limits
#include <vector>       // std::vector
#include <string>       // stoi stoi stod
// Standard C Headers
#include <cstdlib>      // std::exit std::EXIT_FAILURE
// Google Headers
#include <gflags/gflags.h>  // Parse command line args and flags
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/init.h"
#include "utils/spt_dijkstra.h"

using namespace hexgame;
using namespace hexgame::utils;
using namespace std;

// Google Flag Declarations
// Flags
// Input/Output File
DECLARE_bool(gen_random_graph_flag);
// gen_random_graph_file: honored when gen_random_graph_flag is true
DECLARE_string(gen_random_graph_op_file); 
DECLARE_string(input_file);
DECLARE_string(output_file);
DECLARE_int32(num_vertices);
DECLARE_bool(are_edges_directed);
DECLARE_double(edge_density);
DECLARE_int32(min_distance);
DECLARE_int32(max_distance);
DECLARE_int32(src_vertex_id);
DECLARE_int32(dst_vertex_id);
DECLARE_bool(auto_test);

using GCost=uint32_t;

class SPTGraphTester {
 public:
  SPTGraphTester(const bool         auto_test,
                 const std::string& op_file,
                 const std::string& ip_file,
                 const uint32_t     num_vertices,
                 const bool         are_edges_directed,
                 const double       edge_density,
                 const uint32_t     min_distance,
                 const uint32_t     max_distance,
                 const bool         gen_random_graph_flag,
                 const std::string& gen_random_graph_op_file,
                 const uint32_t     src_vertex_id,
                 const uint32_t     dst_vertex_id) :
    _auto_test{auto_test}, 
    _op_file(op_file), _ip_file(ip_file),
    _num_vertices{num_vertices}, _are_edges_directed{are_edges_directed},
    _edge_density{edge_density}, _min_distance{min_distance},
    _max_distance{max_distance},
    _gen_random_graph_flag{gen_random_graph_flag},
    _gen_random_graph_op_file(gen_random_graph_op_file),
    _src_vertex_id{src_vertex_id}, _dst_vertex_id{dst_vertex_id}
  {
    // Validate output file
    if (_op_file.empty() == false) {
      std::ofstream ofp;
      ofp.open(_op_file, std::ios::out);
      if (!ofp) {
        ostringstream oss;
        oss << "Can't open output file " << _op_file;
        throw oss.str();
      }
      ofp.close();
    }

    // Validate input file
    if (_ip_file.empty() == false) {
      std::ifstream ifp;
      ifp.open(_ip_file, std::ios::in);
      if (!ifp) {
        ostringstream oss;
        oss << "Can't open input file " << _ip_file;
        throw oss.str();
      }
      ifp.close();
    }

    // Validate gen random graph output file (if provided must be provided)
    if (_gen_random_graph_op_file.empty() == false) {
      std::ofstream ofp;
      ofp.open(_gen_random_graph_op_file, std::ios::out);
      if (!ofp) {
        ostringstream oss;
        oss << "Can't open random graph output file " << _gen_random_graph_op_file;
        throw oss.str();
      }
      ofp.close();
    }
    return;
  }
  ~SPTGraphTester(void) = default;

  void RandomlyGeneratedGraphTest(void);
  void InputFileReadGraphTest(void);
 protected:
 private:
  const bool         _auto_test;
  const std::string  _op_file;
  const std::string  _ip_file;
  const uint32_t     _num_vertices;
  const bool         _are_edges_directed;
  const double       _edge_density;
  const uint32_t     _min_distance;
  const uint32_t     _max_distance;
  const bool         _gen_random_graph_flag;
  const std::string  _gen_random_graph_op_file;
  const uint32_t     _src_vertex_id;
  const uint32_t     _dst_vertex_id;

  void ProcessGraph(const Graph<GCost> &g, const bool from_ip_file);
};
 
void SPTGraphTester::RandomlyGeneratedGraphTest(void) {
  DLOG(INFO) << "RandomlyGeneratedGraphTest: Initiated";
  GEdgeType type = _are_edges_directed ? 
                   GEdgeType::DIRECTED: 
                   GEdgeType::UNDIRECTED;
  Graph<GCost> g(type, _num_vertices, _edge_density, 
          _min_distance, _max_distance, _auto_test);
  g.output_to_file(_gen_random_graph_op_file);  
  ProcessGraph(g, false);
  DLOG(INFO) << "RandomlyGeneratedGraphTest: Completed";

  return;
}

void SPTGraphTester::InputFileReadGraphTest(void) {
  DLOG(INFO) << "InputFileReadGraphTest: Initiated";
  Graph<GCost> g(_ip_file);
  // generate output only if NOT called from automated test script
  // reason is CTest runs tests parallely. Multiple tests will stomp
  // on the same output file: instead have one test generate op file
  ProcessGraph(g, true); 
  DLOG(INFO) << "InputFileReadGraphTest: Completed";
  return;
}

void SPTGraphTester::ProcessGraph(const Graph<GCost>& g,
                                  const bool   from_ip_file) {
  DLOG(INFO) << g << std::endl;
  
  SPTDijkstra<GCost> spt(g);
  DLOG(INFO) << spt;
    
  if (g.get_num_vertices() <= 2) {
    DLOG(INFO) << "Graph too small (# vertices="<< g.get_num_vertices() 
               << ") to run interesting algorithms: exiting...";
    return;
  }

  // Shortest path
  spt.run_spt_dijkstra(_src_vertex_id); 
  // Do NOT generate output from the InputFileRead if the
  // module is called from automated test scripts.
  // Reason is CTest is multi-threaded and runs different routines
  // parallely - we want to avoid stomping out the same file
  if (!(from_ip_file && _auto_test))
    spt.output_to_file(_op_file);
  
  GCost path_cost;
  // Get the path cost between the two vertices
  path_cost = spt.get_path_size(_src_vertex_id, _dst_vertex_id);

  DLOG(INFO) << "Vertex v[" << _src_vertex_id 
             << "] to v[" << _dst_vertex_id << "]: ";
  if (path_cost < kGInfinityCost<GCost>()) {
    DLOG(INFO) << " Path Cost is " << path_cost;
  }
  else {
    DLOG(INFO) << " Path Does not exist";
  }
  if (_auto_test) {
    // compare as unsigned integers upto 2 decimal places
    uint32_t val = (from_ip_file) ? 700: 400;
    uint32_t val2 = 100*path_cost;
    CHECK_EQ(val, val2) 
        << "Path Cost ERROR: from " << _src_vertex_id 
        << " to " << _dst_vertex_id
        << ": expecting " << val << ": computed " << val2;
  }

  double path1 = spt.get_avg_path_size_for_vertex(_src_vertex_id);
  DLOG(INFO) << "Average path length of the shortest path "
             << "from source vertex v" << _src_vertex_id 
             << " to every other reachable destination vertex is: " 
             << path1;

  double path2 = spt.get_avg_path_size();
  DLOG(INFO) << "Average path length of the shortest paths "
             << "from every vertex (as source) " 
             << "to every reachable destination vertex is: " 
             << path2;

  if (_auto_test) {      
    // compare as unsigned integers upto 2 decimal places
    uint32_t val = (from_ip_file) ? 385 : 362;
    uint32_t val2 = 100*path1;
    CHECK_EQ(val, val2) 
        << "Avg Path Len ERROR: src_vertex " << _src_vertex_id 
        << " to rest of graph: expecting " << val << ": computed " << val2;
    val = (from_ip_file) ? 400 : 200;
    val2 = 100*path2;
    CHECK_EQ(val, val2) 
        << "Summary Avg Path Len ERROR: from all vertices to all vertices: "
        << ": expecting " << val << ": computed " << val2;
  }
  
  return;
}

// Not "using namespace" directive to enforce more descriptive names
// Later we will move to using typedefs 
int main(int argc, char **argv) {
  Init::InitEnv(&argc, &argv);
  
  // Initialize all non default values as desired in the auto-test
  if (FLAGS_auto_test == true) {
    FLAGS_gen_random_graph_flag = true;
    FLAGS_src_vertex_id = 5;
    FLAGS_dst_vertex_id = 10;
  }

  std::string pgm = "/spt_test-op.txt";
  std::string output_file;
  if (FLAGS_output_file.empty() == false)  
    output_file = FLAGS_output_file;
  else if (FLAGS_auto_test == true)
    output_file = std::string(argv[0]) + "-op.txt";
  else if (FLAGS_log_dir.empty() == false) 
    output_file = FLAGS_log_dir + pgm;
  else
    output_file = "." + pgm;
  std::string random_file = FLAGS_gen_random_graph_op_file;
  if (random_file.empty() == true) {
    random_file = output_file + "-random_graph.txt";
  }
  uint32_t min_distance = std::max(1, FLAGS_min_distance);
  uint32_t max_distance = std::max(FLAGS_min_distance, FLAGS_max_distance);
  uint32_t src_vertex_id = FLAGS_src_vertex_id >= FLAGS_num_vertices?
                           1 : FLAGS_src_vertex_id;
  uint32_t dst_vertex_id = ( (FLAGS_dst_vertex_id >= FLAGS_num_vertices) ||
                             (FLAGS_dst_vertex_id == static_cast<int32_t>(src_vertex_id)) ) ?
                           0 : FLAGS_dst_vertex_id;


  try {
    DLOG(INFO) << "Test Program Begins: " << argv[0] << "..." << std::endl;
    DLOG(INFO) << "Test Parameters" 
               << ": auto_test " << std::boolalpha << FLAGS_auto_test
               << ": output file " << output_file 
               << ": input file " <<  FLAGS_input_file
               << std::endl
               << ": num vertices " << FLAGS_num_vertices
               << ": are edges directed " 
               << std::boolalpha << FLAGS_are_edges_directed
               << ": edge density " << FLAGS_edge_density
               << ": min distance " << min_distance
               << ": max distance " << max_distance
               << std::endl
               << ": gen random graph flag " 
               << std::boolalpha << FLAGS_gen_random_graph_flag
               << ": random file " << random_file
               << ": src vertex " << src_vertex_id
               << ": dst vertex " << dst_vertex_id
               << "------------------------";
    
    SPTGraphTester sptTester(FLAGS_auto_test, 
                             output_file, FLAGS_input_file,
                             FLAGS_num_vertices, FLAGS_are_edges_directed, 
                             FLAGS_edge_density, min_distance, max_distance,
                             FLAGS_gen_random_graph_flag, random_file,
                             src_vertex_id, dst_vertex_id);

    if (FLAGS_gen_random_graph_flag == true || FLAGS_auto_test == true) {
      sptTester.RandomlyGeneratedGraphTest();
    }  
    if (FLAGS_input_file.empty() == false) { 
      sptTester.InputFileReadGraphTest();
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

// Flags
// Input/Output File
DEFINE_bool(gen_random_graph_flag, false, 
            "Created graph with edges/cost generated randonly"
            " else read from file");

DEFINE_string(gen_random_graph_op_file, "",
              "Generated graph file: "
              "generation occurs when gen_random_graph_file=true");

DEFINE_string(input_file, "", 
              "Input file with graph input data: "
              "honored when gen_random_graph=false");

DEFINE_string(output_file, "",
              "Output file to store graph output data");
// Graph Parameters
DEFINE_int32(num_vertices, 50,
             "# of vertices of graph: not honored when graph read from file");
static bool ValidateNumVertices(const char* flagname, int32_t num_v) {
  std::string s(flagname);
  if ( (num_v <= 1) || 
       (static_cast<GVertexId>(num_v) >= 
        kGMaxVertexId<GCost>()) ) {
    std::cerr << "Invalid value for --" << s << ": " << num_v 
              << ": should be [2," << kGMaxVertexId<GCost>() << ")" 
              << std::endl;
    return false;
  }
  return true;
}
static const bool 
num_vertices_dummy = google::RegisterFlagValidator(&FLAGS_num_vertices, 
                                                   &ValidateNumVertices);
DEFINE_bool(are_edges_directed, false, 
            "directed edges(?): not honored when graph read from file");

DEFINE_double(edge_density, 0.5, 
              "prob(edge creation): not honored when graph read from file");
static bool ValidateEdgeDensity(const char* flagname, double prob_e) {
  std::string s(flagname);
  if ( (prob_e > 1) || (prob_e < 0) ) {
    std::cerr << "Invalid value for --" << s << ": " << prob_e 
              << ": should be (0,1)" << std::endl;
    return false;
  }
  return true;
}
static const bool 
edge_density_dummy = google::RegisterFlagValidator(&FLAGS_edge_density, 
                                                   &ValidateEdgeDensity);

DEFINE_int32(min_distance, 1,
             "min distance of edge: not honored when graph read from file");
static bool ValidateMinDistance(const char* flagname, int32_t min_d) {
  std::string s(flagname);
  if ( (min_d <= 0) || 
       (static_cast<GCost>(min_d) >= 
        kGInfinityCost<GCost>()) ) {
    std::cerr << "Invalid value for --" << s << ": " << min_d 
              << ": should be [1," << kGInfinityCost<GCost>() 
              << ")" << std::endl;
    return false;
  }
  return true;
}
static const bool 
min_distance_dummy = google::RegisterFlagValidator(&FLAGS_min_distance, 
                                                   &ValidateMinDistance);
DEFINE_int32(max_distance, 10,
             "max distance of edge: not honored when graph read from file; "
             " when max_distance < min_distance: max_distance = min_distance");
static bool ValidateMaxDistance(const char* flagname, int32_t max_d) {
  std::string s(flagname);
  if ( (max_d <= 0) || 
       (static_cast<GCost>(max_d) >= 
        kGInfinityCost<GCost>()) ) {
    std::cerr << "Invalid value for --" << s << ": " << max_d 
              << ": should be [1," 
              << kGMaxVertexId<GCost>() << ")" << std::endl;
    return false;
  }
  return true;
}
static const bool 
max_distance_dummy = google::RegisterFlagValidator(&FLAGS_max_distance, 
                                                   &ValidateMaxDistance);


DEFINE_int32(src_vertex_id, 1, "vertex id of root for SPT algorithm"); 
static bool ValidateSrcVertexId(const char* flagname, int32_t src_v) {
  std::string s(flagname);
  if ( (src_v < 0) || 
       (static_cast<GVertexId>(src_v) >= 
        kGMaxVertexId<GCost>()) ) {
    std::cerr << "Invalid value for --" << s << ": " << src_v 
              << ": should be [0," 
              << kGMaxVertexId<GCost>() 
              << ")" << std::endl;
    return false;
  }
  return true;
}
static const bool 
src_vertex_id_dummy = google::RegisterFlagValidator(&FLAGS_src_vertex_id, 
                                                    &ValidateSrcVertexId);
DEFINE_int32(dst_vertex_id, 0, "vertex id of the dest in SPT algorithm");
static bool ValidateDstVertexId(const char* flagname, int32_t dst_v) {
  std::string s(flagname);
  if ( (dst_v < 0) || 
       (static_cast<GVertexId>(dst_v) >= 
        kGMaxVertexId<GCost>()) ) {
    std::cerr << "Invalid value for --" << s << ": " << dst_v 
              << ": should be [0," 
              << kGMaxVertexId<GCost>() 
              << ")" << std::endl;
    return false;
  }
  return true;
}
static const bool 
dst_vertex_id_dummy = google::RegisterFlagValidator(&FLAGS_dst_vertex_id, 
                                                    &ValidateDstVertexId);

DEFINE_bool(auto_test, false, 
            "test run programmatically (when true) or manually (when false)");

DEFINE_string(output_dir, "",
              "Output directory to store game status");
