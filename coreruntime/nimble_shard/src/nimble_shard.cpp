/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdlib>
#include <iostream>

#include "core_utils/shard.hpp"

[[noreturn]]
void print_usage_and_exit(const char* progName) {
  std::cerr << "Usage: " << progName << " <MD5 <input string>| SHARD_STDIN>" << std::endl;
  std::exit(1);
}

void read_and_print_shard_numbers() {
  std::string line;
  while (std::getline(std::cin, line)) {
    std::cout << util::calculate_shard_number(line) << '\n';
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage_and_exit(argv[0]);
  }

  const std::string mode = argv[1];
  if (mode == "SHARD_STDIN") {
    read_and_print_shard_numbers();
    return 0;
  }

  if (argc != 3) {
    print_usage_and_exit(argv[0]);
  }

  const std::string input = argv[2];
  if (mode == "MD5") {
    std::cout << util::get_md5(input) << std::endl;
  } else {
    print_usage_and_exit(argv[0]);
  }
}
