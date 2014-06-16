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

//! @file mc_hex_test.cc
//! @author Arijit Sarcar <sarcar_a@yahoo.com>
//! @brief Test one iteration of hex game move based on monte carlo simulation.

// C++ Standard Headers
#include <iostream>         // std::cout
// C Standard Headers
#include <cassert>          // assert
// Google Headers
#include <gflags/gflags.h>  // Parse command line args and flags
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/init.h"
#include "games/mc_hex.h"

using namespace hexgame;
using namespace hexgame::utils;
using namespace hexgame::games;
using namespace std;

// Flag Declarations
DECLARE_bool(auto_test);
DECLARE_string(output_dir);

class MCHexTester {
 public:
  MCHexTester(const bool         auto_test,
              const std::string& file_name, 
              const std::string& sm_file_name) : 
    _auto_test{auto_test},
    _mc_hex(file_name, 11, Hex::State::RED, 
            (auto_test)?1:0, auto_test), // if manual test play till end
    // first mover advantage easily leveraged in small hex boards
    _mc_hex_small(sm_file_name, 3, Hex::State::BLUE, 
                  0, auto_test) {};     
  MCHexTester(void) = delete;
  void BigHexTest(void);
  void SmallHexTest(void);
 private:
  const bool _auto_test;
  MCHex      _mc_hex;
  MCHex      _mc_hex_small;
};

void MCHexTester::BigHexTest(void) {
  Hex::State winner = _mc_hex.run();
  if (!_auto_test)
    return;
  CHECK_EQ(_mc_hex.get_last_player(), Hex::State::BLUE) 
      << "First player: " << _mc_hex.get_last_player() << " error";
  CHECK_EQ(_mc_hex.get_num_moves(), 1) 
      << "Num Moves: " << _mc_hex.get_num_moves() << " != 1: error";
  CHECK_EQ(winner, Hex::State::EMPTY) << "Winner: " << winner << " error";

  return;
}

void MCHexTester::SmallHexTest(void) {
  Hex::State winner = _mc_hex_small.run();
  if (!_auto_test)
    return;
  // One has to allow at least 2*dimention-1 moves for the game to terminate 
  CHECK_GE(_mc_hex_small.get_num_moves(), (2*3 - 1)) 
      << "Num moves: " << _mc_hex_small.get_num_moves() << " error";
  CHECK_NE(winner, Hex::State::EMPTY) << "Winner: " << winner << " error";
  return;
}

int main(int argc, char **argv) {
  Init::InitEnv(&argc, &argv);

  std::string pgm = "/mc_hex_test";
  std::string output_file_prefix;
  if (FLAGS_output_dir.empty() == false)  
    output_file_prefix = FLAGS_output_dir + pgm;
  else if (FLAGS_auto_test == true)
    output_file_prefix = std::string(argv[0]);
  else if (FLAGS_log_dir.empty() == false) 
    output_file_prefix = FLAGS_log_dir + pgm;
  else
    output_file_prefix = "." + pgm;

  std::string file_name = output_file_prefix + "-op.txt";
  std::string sm_file_name = output_file_prefix + "-sm_op.txt";
  
  DLOG(INFO) << "Test Program Begins: " << argv[0] << "..." << std::endl;
  DLOG(INFO) << "Test Parameters" 
             << ": output_dir " << FLAGS_output_dir 
             << ": op_file " << file_name << ": sm_op_file " << sm_file_name
             << ": auto_test " << std::boolalpha << FLAGS_auto_test
             << "------------------------";
    
  try {
    MCHexTester tester(FLAGS_auto_test, file_name, sm_file_name);
    tester.BigHexTest();
    tester.SmallHexTest();
  }
  catch (const std::string s) {
    std::cerr << "Exception caught: " << s << std::endl;
  }
  catch (std::exception e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }  

  DLOG(INFO) << "Test Program Ends: ..." << std::endl
             << "************************"; 

  return 0;
}

DEFINE_bool(auto_test, false, 
            "test run programmatically (when true) or manually (when false)");

DEFINE_string(output_dir, "",
              "Output directory to store game status");
