# Copyright 2014 asarcar Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Author: Arijit Sarcar <sarcar_a@yahoo.com>
find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  message(FATAL_ERROR 
    "Doxygen needed to build the documentation. Please install it correctly")
endif(NOT DOXYGEN_FOUND)

if(CMAKE_BUILD_DOCUMENTATION)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE ${CMAKE_BINARY_DIR}/Doxyfile)
  set(DOXYFILE_HTML ${CMAKE_DOC_DIR}/html/index.html)

  # Configure the Template Doxyfile for project
  configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY IMMEDIATE)

  add_custom_command( 
    OUTPUT ${DOXYFILE_HTML}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    MAIN_DEPENDENCY ${DOXYFILE} ${DOXYFILE_IN}
    COMMENT "Generating Documentation for Project ${CMAKE_PROJECT_NAME} with Doxygen" VERBATIM
  )

  add_custom_target( doc ALL DEPENDS ${DOXYFILE_HTML} )
endif()
