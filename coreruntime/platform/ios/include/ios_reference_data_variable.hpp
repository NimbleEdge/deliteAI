/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "frontend_data_variable.hpp"
#include "nimble_net_util.hpp"

/**
 * @brief iOS-specific implementation of FrontendDataVariable for bridging native and iOS objects.
 *
 * Provides overrides for data access, mutation, and conversion between native and iOS representations.
 */
class IOSReferenceDataVariable : public FrontendDataVariable {
  IosObject _iosObj; /**< Underlying iOS object reference. */

  /**
   * @brief Returns a string representation of the iOS object (internal helper).
   *
   * @return std::string String representation.
   */
  std::string common_print() const;

  /**
   * @brief Returns a string representation of the iOS object.
   *
   * @return std::string String representation.
   */
  std::string print() override { return common_print(); }

  /**
   * @brief Converts the iOS object to JSON.
   *
   * @return nlohmann::json JSON representation.
   */
  nlohmann::json to_json() const override { return common_print(); }

  /**
   * @brief Returns a boolean value for the iOS object (always true).
   *
   * @return bool Always true.
   */
  bool get_bool() override { return true; }

  /**
   * @brief Returns the container type (always SINGLE).
   *
   * @return int Container type enum value.
   */
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

 public:
  /**
   * @brief Constructs an IOSReferenceDataVariable from an IosObject.
   *
   * @param obj The iOS object to wrap.
   */
  IOSReferenceDataVariable(IosObject&& obj);

  /**
   * @brief Destructor for IOSReferenceDataVariable.
   */
  ~IOSReferenceDataVariable();

  /**
   * @brief Gets a subscripted value by string key.
   *
   * @param key The key to access.
   * @return OpReturnType The result of the subscript operation.
   */
  OpReturnType get_string_subscript(const std::string& key) override;

  /**
   * @brief Gets a subscripted value by integer index.
   *
   * @param idx The index to access.
   * @return OpReturnType The result of the subscript operation.
   */
  OpReturnType get_int_subscript(int idx) override;

  /**
   * @brief Returns a raw pointer to the underlying data.
   *
   * @return void* Raw pointer.
   */
  void* get_raw_ptr() override;

  /**
   * @brief Returns the size of the iOS object.
   *
   * @return int Size.
   */
  int get_size() override;

  /**
   * @brief Sets a subscripted value.
   *
   * @param subscript The subscript to set.
   * @param val The value to assign.
   */
  void set_subscript(const OpReturnType& subscript, const OpReturnType& val) override;

  /**
   * @brief Arranges the object using the given argument.
   *
   * @param argument The argument for arrangement.
   * @return OpReturnType The result of the arrangement.
   */
  OpReturnType arrange(const OpReturnType argument) override;

  /**
   * @brief Checks if an element is in the object.
   *
   * @param elem The element to check.
   * @return bool True if present, false otherwise.
   */
  bool in(const OpReturnType& elem) override;

  /**
   * @brief Calls a member function by index with arguments and call stack.
   *
   * @param memberFuncIndex Index of the member function.
   * @param arguments Arguments to pass.
   * @param stack Call stack reference.
   * @return OpReturnType The result of the function call.
   */
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;
};
