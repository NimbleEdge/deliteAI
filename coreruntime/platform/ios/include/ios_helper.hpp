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

/**
 * @brief Helper class for iOS-specific tensor and object operations.
 *
 * Provides static utility functions for manipulating iOS objects and tensors from C++.
 */
class IOSHelper {
 public:
  /**
   * @brief Gets a subscripted value from an iOS object by string key.
   *
   * @param obj The iOS object.
   * @param key The key to access.
   * @return CTensor The resulting tensor.
   */
  static CTensor get_string_subscript(IosObject obj, const std::string& key);

  /**
   * @brief Gets a subscripted value from an iOS object by integer index.
   *
   * @param obj The iOS object.
   * @param index The index to access.
   * @return CTensor The resulting tensor.
   */
  static CTensor get_int_subscript(IosObject obj, int index);

  /**
   * @brief Gets the size of an iOS object.
   *
   * @param obj The iOS object.
   * @return int The size.
   */
  static int get_size(IosObject obj);

  /**
   * @brief Deallocates a CTensor.
   *
   * @param cTensor Pointer to the tensor to deallocate.
   */
  static void deallocate_cTensor(CTensor* cTensor);

  /**
   * @brief Sets a subscripted value in an iOS object by string key.
   *
   * @param obj The iOS object.
   * @param key The key to set.
   * @param value The value to assign.
   */
  static void set_subscript(IosObject obj, const std::string& key, OpReturnType value);

  /**
   * @brief Sets a subscripted value in an iOS object by integer index.
   *
   * @param obj The iOS object.
   * @param idx The index to set.
   * @param value The value to assign.
   */
  static void set_subscript(IosObject obj, int idx, OpReturnType value);

  /**
   * @brief Converts an iOS object to a string.
   *
   * @param obj The iOS object.
   * @return std::string String representation.
   */
  static std::string to_string(IosObject obj);

  /**
   * @brief Arranges an iOS object using a list of indices.
   *
   * @param obj The iOS object.
   * @param list The list of indices.
   * @return IosObject The arranged object.
   */
  static IosObject arrange(IosObject obj, const std::vector<int>& list);

  /**
   * @brief Checks if a key is present in an iOS object.
   *
   * @param obj The iOS object.
   * @param key The key to check.
   * @return bool True if present, false otherwise.
   */
  static bool in(IosObject obj, const std::string& key);

  /**
   * @brief Releases the memory for an iOS object.
   *
   * @param obj The iOS object to release.
   */
  static void release(IosObject obj);

  /**
   * @brief Gets the keys of an iOS object as a tensor.
   *
   * @param obj The iOS object.
   * @return CTensor Tensor containing the keys.
   */
  static CTensor get_keys(IosObject obj);
};
