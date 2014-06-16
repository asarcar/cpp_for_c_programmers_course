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

//! @file   init.cc
//! @brief  Implementation: init environment, enable logging, parse flags, etc.
//! @author Arijit Sarcar <sarcar_a@yahoo.com>

// C++ Standard Headers
#include <iostream>         // std::cout
// C Standard Headers
#include <cassert>          // assert
// Google Headers
#include <gflags/gflags.h>  // Parse command line args and flags
#include <glog/logging.h>   // Daemon Log function
// Local Headers
#include "utils/init.h"

DECLARE_bool(logtostderr);
DECLARE_string(log_dir);

namespace hexgame { 
namespace utils {

void Init::InitEnv(int *argc_p, char **argv_p[]) {
  assert(argc_p != nullptr);
  assert(argv_p != nullptr);

  google::ParseCommandLineFlags(argc_p, argv_p, true);
  if (FLAGS_log_dir.empty()) {
    // Every test *should* be invoked with an appropriate environment variable 
    // for the output directory where all logs and output goes
    // if log_dir is not set just direct all logs to the TEST OUTPUT directory
    const char *test_op_dir = getenv("TEST_OUTPUT_DIR");
    if (test_op_dir != nullptr)
      FLAGS_log_dir = test_op_dir;
    else
      FLAGS_logtostderr = true;
  }

  google::InitGoogleLogging((*argv_p)[0]);
  google::InstallFailureSignalHandler();

  DLOG(INFO) << "Program " << (*argv_p)[0] << " initialized"
             << ": logtostderr="<< std::boolalpha << FLAGS_logtostderr
             << ": log_dir=" << FLAGS_log_dir;
  DLOG(INFO) << "  HEAPCHECK=" << getenv("HEAPCHECK")
             << ": HEAPCHECK_DUMP_DIRECTORY=" << getenv("HEAPCHECK_DUMP_DIRECTORY");
  DLOG(INFO) << "  HEAPPROFILE=" << getenv("HEAPPROFILE");
  DLOG(INFO) << "  CPUPROFILE=" << getenv("CPUPROFILE");

  return;
}

} // namespace utils 
} // namespace hexgame


