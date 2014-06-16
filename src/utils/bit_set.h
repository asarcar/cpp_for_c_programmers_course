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

#ifndef _BIT_SET_H_
#define _BIT_SET_H_

// Standard C++ Headers
#include <iostream>         // std::cout
#include <limits>           // std::numeric_limits
#include <vector>           // std::vector
// Standard Headers
#include <cassert>          // assert
// Google Headers
// Local Headers

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------

//
// BitSet is a compact representation to 
// allow one to keep binary (True/False Present/NotPresent)
// indicators of a sequence of finite and limited range of values
// It is very similar to bitset except bitset only accepts compile
// time argument wrt the number of elements
//
class BitSet {
 public:
  explicit BitSet(uint32_t num_bits = 0) : 
      _v(BitSet::size(num_bits), 0) {}
  ~BitSet() = default;
  // Prevent unintended bad usage: 
  // Disallow: copy ctor/assignable or move ctor/assignable (C++11)
  BitSet(const BitSet&) = delete;
  BitSet(BitSet&& o) : _v(std::move(o._v)) {}
  void operator=(const BitSet &) = delete;
  void operator=(BitSet &&) = delete; // C++11 only
  
  inline void set_bit(uint32_t pos) {
    _v.at(BitSet::word_pos(pos)) |= (1 << BitSet::bit_pos(pos));
  }

  inline void clr_bit(uint32_t pos) {
    _v.at(BitSet::word_pos(pos)) &= ~(1 << BitSet::bit_pos(pos));
  }

  inline bool is_bit_set(uint32_t pos) const {
    return ((_v.at(BitSet::word_pos(pos)) & (1 << BitSet::bit_pos(pos))) != 0);
  }
  
  inline void resize(uint32_t num_bits) {
    _v.resize(BitSet::size(num_bits), 0);
  }
 protected:
 private:
  // Private Data Structures
  using gword_t = uint32_t;
  static const uint32_t WORD_BITS = std::numeric_limits<gword_t>::digits;
  // every element upto _n elements is represented by a bit
  std::vector<gword_t> _v; 

  // Provides the size of the bitmap in words
  static inline uint32_t size(uint32_t n) {
    return (((n*n) + sizeof(gword_t) - 1)/sizeof(gword_t));
  }
  static inline uint32_t word_pos(uint32_t pos) { 
    return (pos / WORD_BITS);
  }
  static inline uint32_t bit_pos(uint32_t pos) {
    return (pos % WORD_BITS);
  }
};

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _BIT_SET_H_
