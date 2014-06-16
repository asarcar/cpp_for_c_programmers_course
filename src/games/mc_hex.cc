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
//! @file mc_hex.cc
//! @author Arijit Sarcar <sarcar_a@yahoo.com>
//! @brief Implementation: Generate the best move on Hex based on MC simulation

// Standard C++ Headers
#include <algorithm>        // std::max, std::random_shuffle, std::find
#include <chrono>           // std::chrono::...
#include <functional>       // std::bind
#include <iostream>         // std::cout
#include <random>           // std::distribution, random engine, ...
#include <sstream>          // std::stringstream
#include <vector>           // std::vector
// Standard C Headers
#include <cassert>          // assert
#include <cstdlib>          // std::time
// Google Headers
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "games/mc_hex.h"

using namespace hexgame;
using namespace hexgame::utils;
using namespace std;
using namespace std::placeholders;

namespace hexgame { namespace games {

//-----------------------------------------------------------------------------
MCHex::MCHex(const std::string& op_file,
             const uint32_t     dimension, 
             const Hex::State   human_position_choice,
             const uint32_t     max_moves,
             const bool         auto_test,
             const uint32_t     max_move_time_in_secs,
             const uint32_t     num_sim_trials_allowed) :
  _op_file{op_file}, _dimension{dimension}, 
  _human_position_choice{human_position_choice}, 
  _max_moves{max_moves}, _auto_test{auto_test},
  _max_move_time_in_secs((auto_test)?1:max_move_time_in_secs), 
  _num_sim_trials_allowed((auto_test)?10:num_sim_trials_allowed), 
  _num_open_limit{MCHex::factorial_inverse(num_sim_trials_allowed)},
  _num_moves{0}, _shuffle(dimension*dimension),  _h{dimension} 
{
  _ofp.open(_op_file, std::ios::out);
  if (!_ofp) {
    ostringstream oss;
    oss << "Can't open output file " << op_file;
    throw oss.str();
  }

  // Initialize _shuffle to all possible unique node positions 
  // (0 .. dimension**2-1)
  for (uint32_t i=0; i<dimension*dimension; i++)
    _shuffle.at(i) = i;

  return;
}

MCHex::~MCHex(void) {
  _ofp.close();
  return;
}

Hex::State MCHex::run(void) {
  assert(_num_moves == 0); // set up to call run only once
  uint32_t max_moves = (_max_moves == 0)? 
                       _dimension*_dimension: 
                       std::min(_max_moves, _dimension*_dimension);
  DLOG(INFO) << "MCHex::run()"
             << ": output_file " << _op_file
             << ": dimension " << _dimension
             << ": human position choice " 
             << Hex::str_state(_human_position_choice) << std::endl 
             << ": max_moves " << max_moves 
             << ": max_move_time_in_secs " << _max_move_time_in_secs
             << ": num_sim_trials_allowed " << _num_sim_trials_allowed;

  try {
    DLOG(INFO) << "************************" << std::endl
               << "Hex Game Begins: ..." << std::endl 
               << "--------------------";
    _ofp << "HEX GAME: Human vs SW " << std::endl
         << "  Human Chose Position: " 
         << Hex::str_state(_human_position_choice) << std::endl;
    _ofp << _h;
    if (!_auto_test) 
      std::cout << _h;
    DLOG(INFO) << _h;

    Hex::State s = Hex::State::BLUE;

    for (uint32_t move = 0; move < max_moves; ++move, ++s) {
      assert(_num_moves < max_moves);
      if (_human_position_choice == s) {
        // Human Generated move: validate and process input
        query_and_process_human_move();
      } else {
        // SW generated move based on Monte carlo simulation
        sw_play_next_move();
      }

      _ofp << _h;
      if (!_auto_test) 
        std::cout << _h;
      DLOG(INFO) << _h;

      if (_h.is_play_over()) {
        DLOG(INFO) << "HEX GAME Won: Moves Played " << move
                   << ": Winner " << _h.get_winner() << std::endl;
        break;
      } 
    }
  } 
  catch (const std::string s) {
    std::cerr << "Exception caught: " << s << std::endl;
  }
  catch (std::exception e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }  

  return (_h.get_winner());
}

void MCHex::query_and_process_human_move(void) {
  int32_t pos;
  if (_auto_test == true) {
    // choose any deterministic move that is open
    // since _shuffle is being randomly permuted by SW we might
    // as well just pick the element at index _next_move
    pos = _shuffle[_num_moves];
  } else {
    std::cout << "Player <" << _h.get_next_player()
              << ">: enter move in rowcol format (e.g. A0): ";
    std::string move_str;
    std::cin >> move_str;
    DLOG(INFO) << "Player<" << _h.get_next_player() 
               << ">: " << move_str;
    // Keep taking turns in playing
    pos = _h.get_node_pos_from_str(move_str);
    while (pos < 0) {
      std::cout << "Player Entered Invalid Move: " << move_str << std::endl;
      DLOG(INFO) << "Player Entered Invalid Move: " << move_str;
      std::cout << "Last Player<" << _h.get_last_player() 
                << ">: Player " 
                << _h.get_next_player()
                << "- enter move in rowcol format (e.g. A0): ";
      std::cin >> move_str;
      pos = _h.get_node_pos_from_str(move_str);
    }
  }

  record_next_move(pos);

  return;
}

//! @details SW chooses the next move based on Monte Carlo simulation
//! 1. The simulation generates as many next random move as possible.
//! 2. For each next possible move it chooses the next move with 
//!    best win ratio.
void MCHex::sw_play_next_move(void) {
  // Objective: Identify the next move with the best # of win ratio 
  // - note that since the number of trials is fixed, the # of wins 
  // is equivalent to win_ratio.
  uint32_t next_move, num_win, max_num_win, max_next_move;
  uint32_t seed = (_auto_test == true) ? 
                  MCHex::FIXED_SEED_FOR_RANDOM_ENGINE:
                  std::random_device{}();
  std::default_random_engine rnd_e{seed};

  // a. We randomly permute all elements that are open positions
  // to see what is the next possible move. Given that all elements from 
  // index _next_move onwards in _shuffle are open positions we just permute _shuffle
  // array from _next_move onwards. This gives us the candidate next_move.
  // b. Evaluate the win ratio given the current next move.
  // c. Start the evaluation for another possible next_move unless total time
  //    budgeted to SW has exceeded.
  // d. Play the move with the maximum number of wins

  // Generating the next_move is bounded by possible open_elements
  uint32_t num_open_elements = _dimension*_dimension - _num_moves;
  assert(num_open_elements > 0);

  std::chrono::time_point<std::chrono::system_clock> start, now;
  std::chrono::duration<double> elapsed_seconds;
  start = std::chrono::system_clock::now();

  for (uint32_t num_next_moves_explored = 0; 
       num_next_moves_explored < num_open_elements;
       ++num_next_moves_explored) {

    // Initialize the appropriate random generator class to permute the _shuffle array
    std::shuffle(_shuffle.begin() + _num_moves, _shuffle.end(), rnd_e);

    // Next Move Candidate Identified: now evaluate its win ratio
    next_move = _shuffle.at(_num_moves);
    num_win = sw_determine_win_ratio(next_move, rnd_e);

    if (num_next_moves_explored == 0 || 
        num_win > max_num_win) {
      max_num_win = num_win;
      max_next_move = next_move;
    }

    assert(max_next_move < _shuffle.size());

    now = std::chrono::system_clock::now();
    elapsed_seconds = start - now;
    
    // We have exhausted the time budget: let us take a bet on the winner 
    // as the next move candidate
    if (elapsed_seconds.count() > _max_move_time_in_secs)
      break;
  }

  DLOG(INFO) << "SW Simulated Winner: NextMove " << max_next_move
             << ": #Wins " << max_num_win;

  record_next_move(max_next_move);

  return;
}

//! @details: For a given next move we determine the win ratio 
//! realized as a result of making the specified next move
//! 2.1. Hold the elements from 0 to _next_move constant - assuming the element in
//! _next_move index of _shuffle array is the next move explored we permute 
//! remaining elements as a monte carlo simulation step to evaluate the win ratio.
//! 2.2. Evaluate the win/loss outcome based on the permutation in step (2.1) 
//! 2.3. Repeat step (2.1) and (2.2) until DEFAULT_MAX_SIM_TRIALS_ALLOWED or until 
//! when number of permutations pending exceed what is total possible 
//! @param[in] next_move the next move under consideration
//! @return win-ratio i.e. # of wins realized by returned next move
uint32_t 
MCHex::sw_determine_win_ratio(const uint32_t next_move,
                              std::default_random_engine& rnd_e) {
  uint32_t num_wins =  0;
  // next_move already pegged: num_moves & pending moves adjusted accordingly
  uint32_t num_moves = _num_moves + 1; 
  uint32_t num_open_elements = _dimension*_dimension - _num_moves - 1;
  uint32_t max_trials = (num_open_elements > _num_open_limit) ? 
                        _num_sim_trials_allowed : num_open_elements;

  for (uint32_t num_trials = 0; num_trials < max_trials; ++num_trials) {
    // for the first trial we can just assume the current state of 
    // _shuffle to reflect a possible sequence of moves - otherwise
    // we permute the remaining elements
    if (num_trials > 0) 
      std::shuffle(_shuffle.begin() + num_moves, _shuffle.end(), rnd_e);
    
    // We have now decided all the sequence of moves that is going to unfold
    // in the game. Let us determine the outsome for this sequence by actually
    // playing it out. In order to not mess up the "real" game while we 
    // are playing it in "simulation" we will save and restore the state of Hex
    _h.save_state();

    // Bind arguments to allow execution of _h.set_next_move 
    // on each _shuffle member from num_moves onwards
    for_each(_shuffle.begin() + _num_moves, _shuffle.end(), 
             std::bind(&Hex::set_next_move, &_h, _1));
    _h.assess_positions();
    // Sanity check: game was played to the very end: we must have a winner
    assert(_h.is_play_over() == true);
    // SW win count increases if winner is NOT human
    if (_h.get_winner() != _human_position_choice)
      ++num_wins;
    _h.restore_state();
  }

  DLOG(INFO) << "SW Simulated Num_Moves " << num_moves 
             << ": Max Trials " << max_trials 
             << ": Next Move " << next_move << ": # wins " << num_wins;

  return num_wins;
}

//! @details Record move in the _shuffle array where we keep all the moves
//! made so far such that _shuffle[0..i..(num_moves-1)] specifies position 
//! occupied by each player for move # i. All unoccupied positions appear
//! in shuffle array after num_moves position.
//! @param[in] next_move that the player has decided to make
void MCHex::record_next_move(uint32_t next_move) {
  assert(_num_moves < _shuffle.size());
  // 1. Update the _shuffle array: Our objective is to keep the 
  // shuffle array organized such that all entries from 0.._num_moves 
  // are in the exact position occupied by players as play progressed 
  // from move 0 to move _num_moves. Any subsequent elements are 
  // unoccupied positions. Note For index 0 to _num_moves - 1 
  // all elements in those indices
  // 1.1. Find the index which occupies the next_move position
  // 1.2. Swap that entry with the one in _shuffle[_next_move]
  uint32_t i;
  for (i=_num_moves; i<_shuffle.size(); ++i) {
    if (_shuffle.at(i) == next_move)
      break;
  }
  assert(i < _shuffle.size());
  // Swap the element at index i with the one at index _num_moves
  std::swap(_shuffle.at(_num_moves), _shuffle.at(i));

  _num_moves++;

  _h.set_next_move(next_move);
  _h.assess_positions();

  return;
}
//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace games {
