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

#ifndef _HEX_H_
#define _HEX_H_
// C++ Standard Headers
#include <fstream>          // std::ifstream & std::ofstream
#include <functional>       // std::BinaryPredicate, std::equal_to
#include <iostream>         // std::cout
#include <vector>           // std::vector
// C Standard Headers
#include <cassert>
// Local Headers
#include "utils/egraph.h"

namespace hexgame { namespace games {
//-----------------------------------------------------------------------------
class Hex {
 public:
  // Each Position: Empty or taken by one of two players (Blue, Red)
  enum class State {EMPTY=0, BLUE, RED};

  constexpr static uint32_t NUM_STATES = 3; // Empty, Blue, and Red
  constexpr static uint32_t NUM_PLAYERS = 2; // Blue and Red
  constexpr static uint32_t DEFAULT_DIMENSION = 5;
  constexpr static uint32_t MIN_DIMENSION = 3;
  constexpr static uint32_t MAX_DIMENSION = 26;
  constexpr static int32_t  INVALID_NODE_POS = -1;

  // Display String: State Enumberator
  static inline const std::string& str_state(const State& s) {
    static std::array<std::string, Hex::NUM_STATES>
        StateStr = {{"\"EMPTY\"", "\"BLUE\"", "\"RED\""}};
    return StateStr.at(static_cast<std::size_t>(s));
  }

  explicit Hex(uint32_t dimension = DEFAULT_DIMENSION);
  ~Hex() = default;

  // Prevent unintended bad usage: 
  // Disallow: copy ctor/assignable or move ctor/assignable (C++11)
  Hex(const Hex&) = delete;
  Hex(Hex &&) = delete; // C++11 only
  void operator=(const Hex &) = delete;
  void operator=(Hex &&) = delete; // C++11 only

  // Public Methods
  // Dumps the Hex to the file "file_name"
  void output_to_file(std::string file_name);

  // Provides the node position based on move_str
  // Returns INVALID position if the move_str is bad format or invalid
  int32_t get_node_pos_from_str(std::string move_str);

  // Return false if move for the next player is illegal
  // Otherwise: accept the move and modify state accordingly
  inline bool play_next_move(std::string move_str) {
    int32_t pos = get_node_pos_from_str(move_str);
    // Deal with invalid move_str
    if (pos < 0)
      return false;
    // Assess Valid Position, If so occupy position & assess winner (if any)
    set_next_move(pos);
    assess_positions();

    return true;
  }

  //! @brief Occupy a given position for the next player and assess winner (if any)
  //! @details Assumed: all sanity checks have been made to assess move is valid
  //! Set the next move of the player who desires to 
  //! occupy vid position on the hex board
  //! Assumed: all sanity checks are done and the move is a legal move
  void set_next_move(uint32_t vid);

  //! @brief Assess whether game is over & (if so) who is the winner
  //! @details Game over is declared (by setting _over)
  //! Winner is declared (by setting _winner) 
  //! Note that there is always a winner in this game
  bool assess_positions(void);

  // Return true if play has completed/ended:
  inline bool is_play_over(void) { return _state._over; }
  
  // Returns winner (empty) if no winner
  inline const Hex::State
  get_winner(void) const { return _state._winner; }

  // display the last player: string format
  inline const Hex::State 
  get_last_player(void) const { return _state._last; }

  // Returns the next player that is due to play the next move
  // Cannot insert the function inlined as the compiler has not 
  // yet seen the ++ operator
  // inline const std::string& disp_next_player(void);
  inline State get_next_player(void) {
    return (get_last_player() == State::RED) ? State::BLUE : State::RED;
  }

  // Save State & Restore State: used by MCHex to run "what if scenarios" without
  // messing up current state of Hex
  inline void save_state(void) { _g.save_state(); _save = _state; return; }
  inline void restore_state(void) { _state = _save; _g.restore_state(); return; }

  // helper function to allow chained cout cmds: example
  // cout << "The Hex: " << endl << h << endl << "---------" << endl;
  friend std::ostream& operator << (std::ostream& os, const Hex &h);
  
 protected:
 private:
  using HexGraph = utils::eGraph<State>;
  typedef struct _HexState {
    // round progresses to next value when both players have made their move
    uint32_t     _round; 
    // Players alternate from BLUE to RED, until one of them has won the game
    State        _last;
    // cached status: true when game is over: 
    // optimization to avoid continuing the turn taking game in case it is over
    bool         _over; 
    // set to winner of the game whenever a winner is determined
    State        _winner;
    // Local cached running variables
    uint32_t     _blue;  // number of positions occupied by blue
    uint32_t     _red;   // number of positions occupied by red
  } HexState;

  const uint32_t _dim;
  HexGraph       _g;
  HexState       _state;  
  HexState       _save; 

  // PRIVATE METHODS
  // Create appropriate edges for the graph by connecting corner, boundary
  // and internal nodes
  void connect_corner_nodes(void);
  void connect_boundary_nodes(void);
  void connect_internal_nodes(void);
  // Assess whether player "blue" won the game
  bool did_blue_win(void);
  // Assess whether player "red" won the game
  bool did_red_win(void);
  // Given the row and column position, provide the overall vertex index 
  inline uint32_t get_node_pos(const uint32_t row, const uint32_t col) const {
    return (_dim*row + col);
  }
  // Given the overall index of the vertex, provide the row position
  inline uint32_t get_row(uint32_t node_pos) {
    assert(node_pos <= _dim*_dim);
    return node_pos / _dim;
  }
  // Given the overall index of the vertex, provide the col position
  inline uint32_t get_col(uint32_t node_pos) {
    assert(node_pos <= _dim*_dim);
    return node_pos % _dim;
  }
  // Display the row
  inline static char disp_row(uint32_t ns) {return static_cast<char>('A' + ns);} 
  // Display the col
  inline static uint32_t disp_col(uint32_t ew) {return ew;} 
};

// ++State
inline Hex::State& operator++(Hex::State &s) {
  if (s == Hex::State::RED) {
    s = Hex::State::BLUE;
  } else {
    s = static_cast<Hex::State>(static_cast<uint32_t>(s) + 1);
  }
  return s;
}

inline std::ostream& operator << (std::ostream& os, const Hex::State& s) {
  os << Hex::str_state(s);
  return os;
}

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace games {

#endif // _HEX_H_
