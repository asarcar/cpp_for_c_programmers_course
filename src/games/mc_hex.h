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

//! @file     mc_hex.h
//! @brief    Definition: SW generated moves playing Hex
//! @author   Arijit Sarcar <sarcar_a@yahoo.com>

#ifndef _MC_HEX_H_
#define _MC_HEX_H_
// C++ Standard Headers
#include <iostream>         // std::cout
#include <random>           // std::default_random_engine
#include <vector>           // std::vector
// C Standard Headers
#include <cassert>
// Google Headers
// Local Headers
#include "games/hex.h"

namespace hexgame { 

//! @addtogroup games
//! @{

//! Generic games interfaces and implementations
namespace games {
//-----------------------------------------------------------------------------

//! @class    MCHex
//! @brief    SW plays human: best Hex moves generated using MC simulation
class MCHex {
 public:
  //! Default dimension chosen for Hex
  const static uint32_t DEFAULT_HEX_DIMENSION = 11;
  //! Default position Human chooses to make a move in the game of hex  
  const static Hex::State DEFAULT_POSITION_CHOICE = Hex::State::RED;
  //! Default time SW is allowed (in secs) by when to make a move
  const static uint32_t DEFAULT_MAX_MOVE_TIME_IN_SECS = 60; 
  //! @brief Default Max # of random moves allowed by SW to compute next move
  //! @details SW is allowed upto move_time (secs) to compute next move
  //! via monte carlo simulation. For each random next move there could
  //! be very high number of possible subsequent moves till the end of the game. 
  //! However, SW only examines upto maximum # of subsequent play permutations
  //! as captured by num_sim_trials_allowed. The default value is as caputed
  //! by this constant variable.
  const static uint32_t DEFAULT_MAX_SIM_TRIALS_ALLOWED = 100;
  //! Default bound on # of moves run in game: 0 implies play till terminates
  const static uint32_t DEFAULT_MAX_MOVES = 0;

  //! @brief Ctor builds MCHex Class used to run SW against Human
  //! @param[in] op_file      file_name used to output hex state
  //! @param[in] dimension    # rows/columns used for Hex Game
  //! @param[in] human_positon_choice Preferred position of human (RED or BLUE?)
  //! @param[in] max_move_time_in_secs  max secs SW is allowed to make a move
  //! @param[in] num_sim_trials_allowed max trials SW allowed for each next move
  //! @param[in] max_moves max moves to play (Default 0 => until game over)
  //! @param[in] auto_test true when tested via scripts (e.g. CTest). 
  //!            In that case: generate random moves on behalf
  //!            of human & use the same seed for random number generation 
  //!            to replay the exact set of moves
  explicit MCHex(
      const std::string& op_file,
      const uint32_t     dimension = DEFAULT_HEX_DIMENSION, 
      const Hex::State   human_position_choice = DEFAULT_POSITION_CHOICE,
      const uint32_t     max_moves = DEFAULT_MAX_MOVES, 
      const bool         auto_test = false,
      const uint32_t     max_move_time_in_secs = DEFAULT_MAX_MOVE_TIME_IN_SECS,
      const uint32_t     num_sim_trials_allowed = DEFAULT_MAX_SIM_TRIALS_ALLOWED);
  ~MCHex(void);

  //! @brief Runs Hex game for upto num_moves or until either Human or SW wins
  //! @details Alternates move between Human and SW. The first move (BLUE)
  //! is based on the initial preference of position made by Human 
  //! (which is passed to the Class Constructor). SW is allowed a bounded time
  //! as well as bounded # of simulation trials (whichever provides a tigher 
  //! bound) to decide on the next move.
  //! The execution of the game terminates either when the budgeted number of
  //! moves (if provided to the constructor) is exhausted (or if arg not 
  //! provided) then only when one of the player (Human or SW) wins.
  //! @returns Winner of the hex_game (if any) after num_moves
  Hex::State run(void);
  
  inline Hex::State get_last_player(void) {return _h.get_last_player();}
  inline uint32_t get_num_moves(void) {return _num_moves;}

  MCHex(const MCHex &)          = delete; //!< @brief disallow copy ctor  
  MCHex(MCHex &&)               = delete; //!< @brief disallow move ctor
  void operator=(const MCHex &) = delete; //!< @brief disallow assignment 
  void operator=(MCHex &&)      = delete; //!< @brief disallow move assignment

 protected:
 private:
  //! Fixed seed generates predictable MC runs when running test SW or debugging
  const static uint32_t FIXED_SEED_FOR_RANDOM_ENGINE = 13607; 
  using VectorHexMoves = std::vector<uint32_t>; 
  const std::string     _op_file; //!< output file used to save hex game state
  const uint32_t        _dimension; //!< #row/cols for Hex game
  //! human position preference: BLUE or RED?
  const Hex::State      _human_position_choice; 
  //! Max # of moves to run in the hex game. 0 -> play till game terminates
  const uint32_t        _max_moves;
  //! Flag is true when the MC class is run from an automated test class
  const bool            _auto_test;
  //! Maximum move time (in secs) budgeted to SW to make a move
  const uint32_t        _max_move_time_in_secs; 
  //! Max # of simulation trials allowed for SW after which it must make a move
  const uint32_t        _num_sim_trials_allowed;
  //! @brief # open positions where we simulate < _num_sim_trails_allowed
  //! @details This value is used to save some time towards the end of 
  //! the game when total pending permutations possible is lower than # of 
  //! trials allowed
  const uint32_t        _num_open_limit;
  //! total # of moves made including both BLUE and RED players 
  uint32_t              _num_moves;
  //! @brief  _shuffle: caches the sequence of moves considered so far
  //! @details _shuffle[0] & _shuffle[1]: 1st move positions by BLUE & RED
  //! _shuffle[2*i] & _shuffle[2*i+1]: i(th) move positions by BLUE & RED
  VectorHexMoves        _shuffle;  //!< scratch pad used to generate random moves
  Hex                   _h; //!< Hex Class: Container Object
  std::ofstream         _ofp;

  //! Valiate human input, accept the position (if validate), and assess winner
  void query_and_process_human_move();

  //! @brief SW plays the next move based on Monte Carlo Simulation
  void sw_play_next_move();

  //! @brief Determine the win ratio based on the generated next move 
  uint32_t sw_determine_win_ratio(const uint32_t next_move,
                                  std::default_random_engine& rnd_e);

  //! @brief Record move and assess winner
  void record_next_move(uint32_t next_move);

  //! @brief Returns i the factorial inverse ceiling of a num: f(i) <= num < f(i+1)
  //! @param[in] num number for which we are computing factorial inverse
  //! @return floor(factorial_inverse(num))
  static inline uint32_t factorial_inverse(uint32_t num) {
    assert(num >= 1);
    uint32_t fac = 1;
    for (uint32_t i=1; i<= num; ++i) {
      fac = fac * i;
      if (fac == num)
        return i;
      if (fac > num)
        return i-1;
    }
    // we should never reach this point of the function
    assert(false);
    return 0;
  }
};
//-----------------------------------------------------------------------------
} // namespace games 

//! @} End of Doxygen games

} // namespace hexgame 

//! @} End of Doxygen hexgame

#endif // _MC_HEX_H_
