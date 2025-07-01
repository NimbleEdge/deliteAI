/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include "executor_structs.h"
#include "ne_fwd.hpp"
#include "nimble_net_util.hpp"

class IOSHelper {
 public:
  static CTensor get_string_subscript(IosObject obj, const std::string& key);
  static CTensor get_int_subscript(IosObject obj, int index);
  static int get_size(IosObject obj);
  static void deallocate_cTensor(CTensor* cTensor);
  static void set_subscript(IosObject obj, const std::string& key, OpReturnType value);
  static void set_subscript(IosObject obj, int idx, OpReturnType value);
  static std::string to_string(IosObject obj);
  static IosObject arrange(IosObject obj, const std::vector<int>& list);
  static bool in(IosObject obj, const std::string& key);
  static void release(IosObject obj);
  static CTensor get_keys(IosObject obj);
};
