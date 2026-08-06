//
//  Copyright 2019 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/flags/internal/program_name.h>

#include <string>

#include <turbo/macros/config.h>
#include <turbo/base/no_destructor.h>
#include <turbo/base/thread_annotations.h>
#include <turbo/flags/internal/path_util.h>
#include <string_view>
#include <mutex>

namespace turbo {

namespace flags_internal {

static std::mutex& ProgramNameMutex() {
  static std::mutex mutex;
  return mutex;
}
KUMO_CONST_INIT static std::string* program_name TURBO_GUARDED_BY(
    ProgramNameMutex()) TURBO_PT_GUARDED_BY(ProgramNameMutex()) = nullptr;

std::string ProgramInvocationName() {
  std::unique_lock l(ProgramNameMutex());
  return program_name ? *program_name : "UNKNOWN";
}

std::string ShortProgramInvocationName() {
  std::unique_lock l(ProgramNameMutex());
  return program_name ? std::string(flags_internal::Basename(*program_name))
                      : "UNKNOWN";
}

void SetProgramInvocationName(std::string_view prog_name_str) {
  std::unique_lock l(ProgramNameMutex());
  if (!program_name) {
    program_name = new std::string(prog_name_str);
  } else {
    program_name->assign(prog_name_str.data(), prog_name_str.size());
  }
}

}  // namespace flags_internal

}  // namespace turbo
