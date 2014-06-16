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

// 
// Class PrioQ:
// DESCRIPTION:
//   Creates a class which provides more functionality than the one 
//   provided by STL priorityQ:
//   a. chg_prio(PQ, elem, prio): 
//      1. elem is new: adds the element to the Q
//      2. elem exists in Q: change the priority (node value) of element
//         and insert it back in the correct position.
//   b. contains(PQ, elem): returns true is the element exist in the Q
//   Note that other functions of priorityQ are already available in the 
//   standard template.
//   c. pop_top(PQ): removes the top element of the queue.
//   d. add_elem(PQ, queue_element): insert queue_element into queue
//   e. get_top(PQ):returns the top element of the queue.
//   f. get_size(PQ): return the number of queue_elements.
//

#ifndef _PRIO_Q_H_
#define _PRIO_Q_H_

// Standing C++ Headers
#include <algorithm>    // std::make_heap, std::pop_heap, std::push_heap,...
#include <functional>   // std::less/greater
#include <fstream>      // std::ifstream & std::ofstream
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <vector>       // std::vector
// Standard C Headers
#include <cassert>      // assert
// Google Headers
// Local Headers

namespace hexgame { namespace utils {
//-----------------------------------------------------------------------------

//
// Forward Declarations for Template Class
//
template <class T> class equal;
// Allow friend class that uses the template class as arguments
template <class T, class GtCmpObjFn, class EqCmpObjFn> class PrioQ;
template <class T, class GtCmpObjFn, class EqCmpObjFn>
std::ostream& operator <<(std::ostream& os, 
                          const PrioQ<T, GtCmpObjFn, EqCmpObjFn>& pq);
// End: Forward Declarations
    
// GtCmpObjFn: top elem (hi priority obj: first arg) returns false 
//             when compared to all other objects (second arg) 
//             in the container.
// EqCmpObjFn: returns true if arg1 (reference to object of type
//             container element and arg2 (reference to object in 
//             container are considered "equal".
template <class T, 
          class GtCmpObjFn = std::greater<T>,   
          class EqCmpObjFn = hexgame::utils::equal<T>>
class PrioQ {
 public:
  using pqsize_t = uint32_t;

  // Contructors: Default:
  // PrioQ <T, Cmp> () {}
  // Destructors: Default
  // Copy, Move, =, ...: Default
 
  // METHODS:
  //   chg_val(PQ, refval, newval): 
  //     elem exists in Q: change the priority (node value) of element
  //     and recreate the heap whose properties have been violated.
  //     TODO: this is very inefficient - need to figure out a better way.
  inline void chg_val(T& refval, T& newval) {
    refval = newval;
    std::make_heap(_vec.begin(), _vec.end(), _gtCmp);
    return;
  }
  
  //   get_size(PQ): return the number of queue_elements.
  inline pqsize_t get_size() const {return _vec.size();}

  //   get_top(PQ):returns the top element of the queue.
  inline const T& get_top() const {return _vec.front();}

  //   get_top(PQ): removes the top element of the queue.
  inline void pop_top() { 
    std::pop_heap(_vec.begin(), _vec.end(), _gtCmp);
    _vec.pop_back(); 
  }

  // insert_elem(PQ, element): insert element into queue
  inline void insert_elem(const T& elem) {
    _vec.push_back(elem);
    std::push_heap(_vec.begin(), _vec.end(), _gtCmp);
    return;
  }

  T& contains_elem(const T& elem, bool& match);

  void output_to_file(std::string file_name);

  // Template Friend Function: the "<>" after function 
  // specifies that this is a template function accepting 
  // template arguments
  friend std::ostream& operator << <>(std::ostream& os, 
                                      const PrioQ
                                      <T, 
                                      GtCmpObjFn, 
                                      EqCmpObjFn>& pq);

  // ITERATORS
  using const_iterator = typename std::vector<T>::const_iterator;
  inline const_iterator cbegin() const { return _vec.cbegin(); }
  inline const_iterator cend() const { return _vec.cend(); }

 private:
  std::vector<T> _vec;
  GtCmpObjFn     _gtCmp;
  EqCmpObjFn     _eqCmp;
};

//
// Objects are stored in a vector
// 
template <class T>
class equal {
 public:
  bool operator() (const T&x, const T&y) const {return x == y;}
};

//
// contains_elem(PQ, T& elem, bool& match): 
//    returns true if the elem exist in the Q
//    if matched the reference to the container element is returned
template <class T, class GtCmpObjFn, class EqCmpObjFn>
T& PrioQ<T, GtCmpObjFn, EqCmpObjFn>::contains_elem(const T& elem, bool& match)
{
    match = false;
    
    for (auto it = _vec.cbegin(); it != _vec.cend(); ++it) {
      if (_eqCmp(elem, *it) == true) {
        match = true;
        return const_cast<T&>(*it);
      }
    }
    
    return const_cast<T&>(elem);
}

// Dumps the state of the priority queue in file_name
template <class T, class GtCmpObjFn, class EqCmpObjFn>
void PrioQ<T, GtCmpObjFn, EqCmpObjFn>::output_to_file(std::string file_name) {
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

template <class T, class GtCmpObjFn, class EqCmpObjFn>
std::ostream& operator << (std::ostream& os, 
                           const PrioQ<T, GtCmpObjFn, EqCmpObjFn> &pq) {
  os << "#**********************" << std::endl;
  os << "# PRIORITY QUEUE OUTPUT" << std::endl;
  os << "#----------------------" << std::endl;
  os << "# FORMAT: " << std::endl;
  os << "# num_of_elements" << std::endl;
  os << "# idx value" << std::endl;
  os << "#######################" << std::endl;
  os << pq.get_size() << std::endl;

  int i = 0;
  for (auto it = pq.cbegin(); it != pq.cend(); ++it)
    os << "[" << i++ << "]: " << *it << std::endl;
  os << "#######################" << std::endl;

  os << "#**********************" << std::endl;

  return os;
}

//-----------------------------------------------------------------------------
} } // namespace hexgame { namespace utils {

#endif // _PRIO_Q_H_
