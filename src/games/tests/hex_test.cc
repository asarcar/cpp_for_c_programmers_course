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
#include <algorithm>        // std::shuffle
#include <fstream>          // std::ofstream
#include <iostream>         // std::cout
#include <sstream>          // std::stringstream
#include <string>           // std::string
#include <exception>        // std::exception
// Standing C Headers
#include <cstdlib>      // std::exit std::EXIT_FAILURE
#include <cassert>      // assert
// Google Headers
#include <gflags/gflags.h>  // Parse command line args and flags
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "games/hex.h"
#include "utils/init.h"

using namespace hexgame;
using namespace hexgame::games;
using namespace hexgame::utils;
using namespace std;

// Flag Declarations
DECLARE_string(output_dir);
DECLARE_int32(dimension);
DECLARE_int32(num_moves);
DECLARE_bool(auto_test);

class HexTester {
 public:
  HexTester(const std::string& file, 
            const uint32_t     dimension):
      _op_file(file), _dimension(dimension), 
      _hex(dimension), _shuffle(dimension*dimension) {
    // Initialize shuffle in case it is used for auto_test
    // Randomize with a predictable seed so that the test
    // is repeatable
    for(uint32_t i=0; i<_shuffle.size(); ++i) 
      _shuffle.at(i) = i;
    std::shuffle(_shuffle.begin(), _shuffle.end(), 
                 std::default_random_engine{2014});
    // Assume op_file is always provided && it is valid/writable etc.
    _ofp.open(_op_file, std::ios::out);
    if (!_ofp) {
      ostringstream oss;
      oss << "Can't open input file " << file;
      throw oss.str();
    }
    return;
  }
  HexTester(void) = delete;
  void RunTillNMovesTest(const uint32_t num_moves,
                         const bool     auto_test);
 private:
  std::string       _op_file;
  uint32_t          _dimension;
  Hex               _hex;
  vector<uint32_t>  _shuffle;
  ofstream          _ofp;
};

void HexTester::RunTillNMovesTest(const uint32_t num_moves,
                                  const bool     auto_test) {
  uint32_t max_moves = (num_moves == 0)? 
                       _dimension*_dimension: 
                       std::min(num_moves, _dimension*_dimension);
  _ofp << _hex;
  if (!auto_test)
    std::cout << _hex;
  DLOG(INFO) << _hex;

  Hex::State s = Hex::State::BLUE;
  for (uint32_t move = 0; move < max_moves; ++move, ++s) {
    if (auto_test) {
      DLOG(INFO) << "Player <" << Hex::str_state(s)
                 << ">: occupied node# " << _shuffle.at(move);
      // The random shuffle of possible positions is the order game is played
      _hex.set_next_move(_shuffle.at(move));
      _hex.assess_positions();
    } else {
      std::cout << "Player <" << Hex::str_state(s)
                << ">: enter move in rowcol format (e.g. A0): ";
      std::string move_str;
      std::cin >> move_str;
      DLOG(INFO) << "Player<" << Hex::str_state(s) 
                 << ">: " << move_str;
      // Keep taking turns in playing
      while (_hex.play_next_move(move_str) == false) {
        std::cout << "Player Entered Invalid Move: " << move_str << std::endl;
        DLOG(INFO) << "Player Entered Invalid Move: " << move_str;
        std::cout << "Last Player<" << _hex.get_last_player() 
                << ">: Next Player " 
                  << Hex::str_state(s)
                  << "- enter move in rowcol format (e.g. A0): ";
        std::cin >> move_str;
      }
    }
    
    _ofp << _hex;
    if (!auto_test) 
      std::cout << _hex;
    DLOG(INFO) << _hex;
    
    if (_hex.is_play_over()) {
      DLOG(INFO) << "HEX GAME Won: Moves Played " << move
                 << ": Winner " << _hex.get_winner() << std::endl;
      break;
    }     
  }

  return;
}

int main(int argc, char **argv) {
  Init::InitEnv(&argc, &argv);

  std::string pgm = "/hex_test";
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

  DLOG(INFO) << "Test Program Begins: " << argv[0] << "..." << std::endl;
  DLOG(INFO) << "Test Parameters" 
             << ": output_dir " << FLAGS_output_dir 
             << ": op_file " << file_name
             << ": dimension " << FLAGS_dimension 
             << ": num_moves " << FLAGS_num_moves
             << ": auto_test " << std::boolalpha << FLAGS_auto_test
             << "------------------------";


  HexTester tester(file_name, FLAGS_dimension);

  tester.RunTillNMovesTest(FLAGS_num_moves, FLAGS_auto_test);
  
  DLOG(INFO) << "Test Program Ends: ..." << std::endl
             << "************************"; 
  return 0;
}

DEFINE_string(output_dir, "",
              "Output directory to store hex game status");

DEFINE_int32(dimension, 5, 
             "dimension (# of rows or # of cols) of the hex game"); 
static bool ValidateDimension(const char* flagname, int32_t dim) {
  std::string s(flagname);
  if ((static_cast<uint32_t>(dim) < Hex::MIN_DIMENSION) || 
      (static_cast<uint32_t>(dim) > Hex::MAX_DIMENSION)) {
    std::cerr << "Invalid value for --" << s << ": " << dim 
              << ": should be [" << Hex::MIN_DIMENSION 
              << "," << Hex::MAX_DIMENSION
              << "]" << std::endl;
    return false;
  }
  return true;
}
static const bool 
dim_dummy = google::RegisterFlagValidator(&FLAGS_dimension, 
                                          &ValidateDimension);

DEFINE_int32(num_moves, 0, 
             "# of moves played in total in hex game: 0 implies play till end"); 

DEFINE_bool(auto_test, false, 
            "inputs generated manually (when false) or programmatically");
