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
#include <iomanip>          // std::setw, std::left, std::setfill
#include <sstream>          // std::stringstream
#include <exception>        // throw
// Standard C Headers
#include <cassert>          // assert()
// Google Headers
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/graph_iter.h"
#include "games/hex.h"

using namespace hexgame;
using namespace hexgame::utils;
using namespace std;

namespace hexgame { namespace games {
//-----------------------------------------------------------------------------
// Definition of constant expression: allocated global variable space
constexpr uint32_t Hex::NUM_STATES;
constexpr uint32_t Hex::NUM_PLAYERS;
constexpr uint32_t Hex::DEFAULT_DIMENSION;
constexpr uint32_t Hex::MIN_DIMENSION;
constexpr uint32_t Hex::MAX_DIMENSION;
constexpr int32_t  Hex::INVALID_NODE_POS;

Hex::Hex(const uint32_t dimension) :
    _dim(dimension), _g(dimension*dimension), 
    _state{ ._round = 0, ._last = Hex::State::EMPTY, 
            ._over = false, ._winner = Hex::State::EMPTY,
            ._blue = 0, ._red = 0} {
  if (_dim < Hex::MIN_DIMENSION || _dim > Hex::MAX_DIMENSION) {
    std::stringstream ss;
    ss << "Hex: Game dimension " << dimension 
       << ": accepted-range 26 >= dimension >=3";
    throw ss.str();
  }
  connect_corner_nodes();
  connect_boundary_nodes();
  connect_internal_nodes();

  return;
}

// Provides the node position based on move_str
// Returns INVALID position if the move_str is bad format or invalid
int32_t Hex::get_node_pos_from_str(std::string move_str) {
  char ch;
  uint32_t ew;

  // Winner already established: no need for any more moves of the game
  if (_state._over == true || _state._winner != State::EMPTY) {
    DLOG(INFO) << "Move not accepted: Game " 
               << std::boolalpha << _state._over
               << " Winner : " << get_winner();
    return INVALID_NODE_POS;
  }

  // 1. Tease out the row and col entered for the next move
  std::istringstream iss(move_str);
  if (iss.peek() != EOF)
    iss >> ch;
  else {
    DLOG(INFO) << "Next Move Bad Format: \"" << move_str 
               << "\": enter row-alphabet followed by col (e.g. A0)" 
               << "BAD row-alphabet" << std::endl;
    return INVALID_NODE_POS;
  }
    
  if (iss.peek() != EOF)
    iss >> ew;
  else {
    DLOG(INFO) << "Next Move Bad Format: \"" << move_str 
               << "\": enter row-alphabet followed by col (e.g. A0)" 
               << "BAD column" << std::endl;
    return INVALID_NODE_POS;
  }

  if ((ch >= 'A' && 
       ch < ('A' + static_cast<char>(_dim)) && 
       ew >= 0 && 
       ew < _dim &&
       iss.peek() == EOF) == false) {
    DLOG(INFO) << "Next Move Bad Format: \"" << move_str 
               << "\": enter row-alphabet followed by col (e.g. A0)" 
               << std::endl;
    return INVALID_NODE_POS;
  }

  // 2. Determine the node position where the move is made
  uint32_t ns = static_cast<uint32_t>(ch - 'A');
  uint32_t vid = get_node_pos(ns, ew);
  State s = this->_g.get_vertex_attr(vid);
  // once a board position is taken by BLUE or RED it cannot be 
  // overriden by the opponent
  if (s != State::EMPTY)
    return INVALID_NODE_POS;

  return vid;
}

// Dumps the graph to the file "file_name"
void Hex::output_to_file(std::string file_name) {
  std::ofstream ofp;

  ofp.open(file_name, std::ios::out);
  if (!ofp) {
    std::stringstream ss;
    ss << "Can't open output file " << file_name;
    throw ss.str();
  }

  ofp << *this;
  
  ofp.close();

  return;
}

// Hex Output
// helper function to allow chained cout cmds: example
// cout << "The Hex: " << endl << h << endl << "---------" << endl;
std::ostream& operator <<(std::ostream& os, const Hex &h) {
  std::array<char, Hex::NUM_STATES> 
      disp_state = {'.', 'X', '#'};
  std::array<char, 2> disp_ns_edge = {'\\','/'};
  char ew_edge = '_';

  // Introduction Label:
  std::string hex_dis = 
      "************************************************************\n" 
      "*   ITEM         * SYMBOL  *             NOTES             *\n"
      "************************************************************\n"
      "*   EMPTY        *    .    *                               *\n"
      "*   BLUE         *    X    *          West-East, moves 1st *\n"
      "*   RED          *    #    *          North-South          *\n"
      "************************************************************\n"
      "* East-West      *    -    *                               *\n"
      "* North-South    *   \\ /   *                               *\n"
      "************************************************************\n\n";
  os << "------------------------------------------------------------\n";
  os << "+ NEXT ROUND# " << h._state._round 
     << ": LAST PLAYER " << h.get_last_player() << std::endl;
  os << "+ GAME OVER " << std::boolalpha << h._state._over
     << ": WINNER " << h.get_winner() << std::endl;
  os << "+ TOTAL CELLS: " << h._dim*h._dim 
     <<  ": BLUE CELLS " << h._state._blue 
     << ": RED CELLS " << h._state._red << std::endl;
  os << "------------------------------------------------------------\n\n";
  os << "* BOARD STATE: " << std::endl;
  os << hex_dis;
  
  // North Label
  os << ' ';
  for (uint32_t ew = 0; ew < h._dim; ++ew) {
    os << std::left << std::setfill(' ') << std::setw(4) << h.disp_col(ew);
  }
  os << std::endl;
  for (uint32_t ns = 0; ns < h._dim; ++ns) {
    // East Label
    os << std::right << std::setfill(' ') << std::setw(ns*2 + 1) 
       << Hex::disp_row(ns);
    os << " ";
    // Display State
    for (uint32_t ew = 0; ew < h._dim; ++ew) {
      uint32_t vid = h.get_node_pos(ns, ew);
      Hex::State s = h._g.get_vertex_attr(vid);
      os << std::left << std::setfill(' ') << std::setw(2) 
         << disp_state.at(static_cast<std::size_t>(s));
      // Display East edges (West Edges are displayed in the previous iteration)
      if (ew >= h._dim - 1)
        continue;
      if (h._g.get_edge_value(vid, vid+1) < Graph::INFINITY_COST)
        os << std::left << std::setfill(' ') << std::setw(2) << ew_edge;
    }
    // West Label
    os << std::left << std::setfill(' ') << std::setw(2) 
       << Hex::disp_row(ns) << std::endl; 

    // Full Iteration to Display North South Edges: if not on last row
    if (ns >= h._dim - 1) 
      continue;
    // Given a row_idx = ns, we are printing north south
    // edges from row ns to ns + 1
    os << std::right << std::setfill(' ') << std::setw(ns*2 + 3) << ' ';
    for (uint32_t ew = 0, i = 0; ew < h._dim; ++ew) {
      uint32_t nvid1;
      uint32_t svid1;
      uint32_t svid2;
      if (ew == 0) {
        nvid1 = h.get_node_pos(ns, 0);
        svid1 = h.get_node_pos(ns + 1, 0);
        if (h._g.get_edge_value(nvid1, svid1) < Graph::INFINITY_COST)
          os << std::left << std::setfill(' ') << std::setw(2) 
             << disp_ns_edge.at(i++%2);
        continue;
      }
      nvid1 = h.get_node_pos(ns, ew);
      svid1 = h.get_node_pos(ns+1, ew-1);
      svid2 = h.get_node_pos(ns+1, ew);
      if (h._g.get_edge_value(nvid1, svid1) < Graph::INFINITY_COST)
        os << std::left << std::setfill(' ') << std::setw(2) 
           << disp_ns_edge.at(i++%2);
      if (h._g.get_edge_value(nvid1, svid2) < Graph::INFINITY_COST)
        os << std::left << std::setfill(' ') << std::setw(2) 
           << disp_ns_edge.at(i++%2);
    }
    os << std::endl;
  }
  // South Label
  os << std::right << std::setfill(' ') << std::setw(h._dim*2 + 1) << ' ';
  for (uint32_t ew = 0; ew < h._dim; ++ew) {
    os << std::left << std::setfill(' ') << std::setw(4) << ew;
  }
  os << std::endl;

  return os;
}

void Hex::connect_corner_nodes(void) {
  // connect corner nodes
  // 1. North West: 2 edges
  _g.add_edge(get_node_pos(0, 0), 
              get_node_pos(0, 1)); 
  _g.add_edge(get_node_pos(0, 0), 
              get_node_pos(1, 0));
  // 2. North East: 3 edges
  _g.add_edge(get_node_pos(0, _dim-1), 
              get_node_pos(0, _dim-2));
  _g.add_edge(get_node_pos(0, _dim-1), 
              get_node_pos(1, _dim-2));
  _g.add_edge(get_node_pos(0, _dim-1), 
              get_node_pos(1, _dim-1));
  // 3. South West: 3 edges
  _g.add_edge(get_node_pos(_dim-1, 0),
              get_node_pos(_dim-2, 0));
  _g.add_edge(get_node_pos(_dim-1, 0),
              get_node_pos(_dim-2, 1));
  _g.add_edge(get_node_pos(_dim-1, 0),
              get_node_pos(_dim-1, 1));
  // 4. South East: 2 edges
  _g.add_edge(get_node_pos(_dim-1, _dim-1), 
              get_node_pos(_dim-1, _dim-2)); 
  _g.add_edge(get_node_pos(_dim-1, _dim-1), 
              get_node_pos(_dim-2, _dim-1));
  return;
}

// Connects the edges of all boundary nodes 
// Ignore the very edges
void Hex::connect_boundary_nodes(void) {
  // North Boundary (ns = 0): 
  // 4 edges from each node: west, east, south west, south east
  for (uint32_t ew = 1; ew < _dim -1; ++ew) {
    _g.add_edge(get_node_pos(0, ew),
                get_node_pos(0, ew-1));
    _g.add_edge(get_node_pos(0, ew),
                get_node_pos(0, ew+1));
    _g.add_edge(get_node_pos(0, ew),
                get_node_pos(1, ew-1));
    _g.add_edge(get_node_pos(0, ew),
                get_node_pos(1, ew));
  }
  
  // South Boundary (ns = _dim-1)
  // 4 edges from each node: west, east, north west, north east
  for (uint32_t ew = 1; ew < _dim -1; ++ew) {
    _g.add_edge(get_node_pos(_dim-1, ew),
                get_node_pos(_dim-1, ew-1));
    _g.add_edge(get_node_pos(_dim-1, ew),
                get_node_pos(_dim-1, ew+1));
    _g.add_edge(get_node_pos(_dim-1, ew),
                get_node_pos(_dim-2, ew));
    _g.add_edge(get_node_pos(_dim-1, ew),
                get_node_pos(_dim-2, ew+1));
  } 

  // West Boundary (ew = 0)
  // 4 edges from each node: north west, north east, east, south east
  for (uint32_t ns = 1; ns < _dim -1; ++ns) {
    _g.add_edge(get_node_pos(ns, 0),
                get_node_pos(ns-1, 0));
    _g.add_edge(get_node_pos(ns, 0),
                get_node_pos(ns-1, 1));
    _g.add_edge(get_node_pos(ns, 0),
                get_node_pos(ns, 1));
    _g.add_edge(get_node_pos(ns, 0),
                get_node_pos(ns+1, 0));
  }  

  // East Boundary (ew = _dim-1)
  // 4 edges from each node: north west, west, south west, south east
  for (uint32_t ns = 1; ns < _dim -1; ++ns) {
    _g.add_edge(get_node_pos(ns, _dim-1),
                get_node_pos(ns-1, _dim-1));
    _g.add_edge(get_node_pos(ns, _dim-1),
                get_node_pos(ns, _dim-2));
    _g.add_edge(get_node_pos(ns, _dim-1),
                get_node_pos(ns+1, _dim-2));
    _g.add_edge(get_node_pos(ns, _dim-1),
                get_node_pos(ns+1, _dim-1));
  }

  return;
}

void Hex::connect_internal_nodes(void) {
  // 6 edges from each node: 

  // west, east, north west, north east, south west, south east
  for (uint32_t ns = 1; ns < _dim - 1; ++ns) {
    for (uint32_t ew = 1; ew < _dim -1; ++ew) {
      _g.add_edge(get_node_pos(ns, ew),
                  get_node_pos(ns, ew-1));
      _g.add_edge(get_node_pos(ns, ew),
                  get_node_pos(ns, ew+1));
      _g.add_edge(get_node_pos(ns, ew),
                  get_node_pos(ns-1, ew));
      _g.add_edge(get_node_pos(ns, ew),
                  get_node_pos(ns-1, ew+1));
      _g.add_edge(get_node_pos(ns, ew),
                  get_node_pos(ns+1, ew-1));
      _g.add_edge(get_node_pos(ns, ew),
                  get_node_pos(ns+1, ew));
    }
  }

  return;
}

// Set the next move of the player who desires to 
// occupy vid position on the hex board
// Assumed: all sanity checks are done and the move is a legal move
void Hex::set_next_move(uint32_t vid) {
  State s = this->_g.get_vertex_attr(vid);
  ++_state._last;

  _g.set_vertex_attr(vid, _state._last);

  DLOG(INFO) << "VertexId " << vid 
             << "(" << get_row(vid) << "," << get_col(vid) 
             <<") prev state " << Hex::str_state(s) 
             <<": cur state " << get_last_player() << std::endl;

  // next player and running counters are appropriately set 
  if (_state._last == State::BLUE) {
    ++_state._blue;
  } else {
    ++_state._red;
    ++_state._round;
  }
  // Sanity checks
  assert(_state._blue+_state._red <= _dim*_dim);
  assert((_state._blue -_state._red) <= 1);

  return;
}

// Assess based on positions whether 
// the game is over & (if so) who is the winner
// Game over is declared (by setting _over)
// Winner is declared (by setting _winner) 
// Note that there is always a winner in this game
bool Hex::assess_positions(void) {
  // Play is over when there exists a path from North to South 
  // (RED) or East to West (BLUE). 
  bool blue_won = did_blue_win(); 
  if (blue_won) {
    _state._over = true;
    _state._winner = State::BLUE;
    return true;
  }

  // For a fully occupied Hex 
  // Board (since we are guaranteed that there always
  // exists a winner AND both cannot win), if BLUE does not 
  // win, we return RED.
  if (_state._blue + _state._red == _dim*_dim) {
    _state._over = true;
    _state._winner = State::RED;
    return true;
  }
  
  bool red_won = did_red_win();
  if (red_won == true) {
    _state._over = true;
    _state._winner = State::RED;
    return true;
  }

  return false;
}

// Assess whether player "blue" won the game
bool Hex::did_blue_win(void) {
  // ALGORITHM: DFS seeding all the vertices in the north border occupied by BLUE
  // If any vertex during the DFS iteration occupies the south border then
  // BLUE has won.
  // WHY DFS (not BFS): DFS from each of the west border nodes ensures that
  // were a path to exist from north to south, 
  // on average we would hit the south border in around (_dim^2/2)/2 iterations.
  // On the other hand BFS from each north border node would have 
  // converged in (_dim^2/2 - _dim/2/2) iterations.
  // 1. Add all vertices in the west border (ew == 0)
  //    occupied by BLUE as the seed vertices to initiate DFS.
  // 2. Initiate a DFS on these seed vertices
  // 3. If any of the DFS traversal reaches the east border (ew == _dim-=)
  //    BLUE has won
  
  Graph::SeedVertices seed_v;
  Graph::gvertexid_t  vid;
  DLOG(INFO) << "Assess " << str_state(State::BLUE) << " won?" << std::endl; 
  DLOG(INFO) << "Seed Vertices (# " << seed_v.size() << ")";
  // 1. Add all vertices in the west border (ew == 0)
  //    occupied by BLUE as the seed vertices to initiate DFS.
  for (uint32_t ns=0; ns < _dim; ++ns) {
    vid = get_node_pos(ns, 0);
    if (_g.get_vertex_attr(vid) == State::BLUE) {
      DLOG(INFO) << " " << Hex::disp_row(ns) << Hex::disp_col(0);
      seed_v.push_back(vid);
    }
  }

  // 2. Initiate a DFS on these seed vertices
  uint32_t ew, i=0;
  DLOG(INFO) << "Initiate DFS - vertices visited: ";
  for (auto it = _g.vcbegin(Graph::VertexIterType::DFS_ORDER, seed_v);
       it != _g.vcend(Graph::VertexIterType::DFS_ORDER); 
       ++it, ++i) {
    ew = get_col(*it);
    DLOG(INFO) << " vid" << *it << " " 
               << disp_row(get_row(*it)) << disp_col(ew)
               << " state " << Hex::str_state(_g.get_vertex_attr(*it));
    // 3. If any of the DFS traversal reaches the east border (col == _dim-=)
    //    BLUE has won
    if (ew == _dim-1)
      return true;

    // Sanity Check: no way should we be visiting more than the 
    // maximum number of BLUE nodes possible in the graph
    assert(i < ((_dim*_dim + 1)/2));
  }

  return false;
}

// Assess whether player "red" won the game
bool Hex::did_red_win(void) {
  // ALGORITHM: DFS seeding all the vertices in the north border occupied by RED
  // If any vertex during the DFS iteration occupies the south border then
  // RED has won.
  // WHY DFS (not BFS): DFS from each of the north border nodes ensures that
  // were a path to exist from north to south, 
  // on average we would hit the south border in around (_dim^2/2)/2 iterations.
  // On the other hand BFS from each north border node would have 
  // converged in (_dim^2/2 - _dim/2/2) iterations.
  // 1. Add all vertices in the north border (ns == 0)
  //    occupied by RED as the seed vertices to initiate DFS.
  // 2. Initiate a DFS on these seed vertices
  // 3. If any of the DFS traversal reaches the south border (ns == _dim-1)
  //    RED has won
  
  Graph::SeedVertices seed_v;
  Graph::gvertexid_t  vid;
  DLOG(INFO) << "Assess " << str_state(State::BLUE) << " won?" << std::endl; 
  DLOG(INFO) << "Seed Vertices (# " << seed_v.size() << ")";
  // 1. Add all vertices in the north border (ns == 0)
  //    occupied by RED as the seed vertices to initiate DFS.
  for (uint32_t ew=0; ew < _dim; ++ew) {
    vid = get_node_pos(0, ew);
    if (_g.get_vertex_attr(vid) == State::RED) {
      DLOG(INFO) << " " << Hex::disp_row(0) << Hex::disp_col(ew);
      seed_v.push_back(vid);
    }
  }

  // 2. Initiate a DFS on these seed vertices
  uint32_t ns, i=0;
  DLOG(INFO) << "Initiate DFS - vertices visited: ";
  for (auto it = _g.vcbegin(Graph::VertexIterType::DFS_ORDER, seed_v);
       it != _g.vcend(Graph::VertexIterType::DFS_ORDER); 
       ++it, ++i) {
    ns = get_row(*it);
    DLOG(INFO) << " vid" << *it << " " 
               << disp_row(ns) << disp_col(get_col(*it))
               << " state " << Hex::str_state(_g.get_vertex_attr(*it));
    // 3. If any of the DFS traversal reaches the south border (ns == _dim-1)
    //    BLUE has won
    if (ns == _dim-1)
      return true;

    // Sanity Check: no way should we be visiting more than the 
    // maximum number of RED nodes possible in the graph
    assert(i < ((_dim*_dim + 1)/2));
  }

  return false;
}

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace games {
