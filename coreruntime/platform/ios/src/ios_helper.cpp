/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ios_helper.hpp"

#include <type_traits>

#include "core_utils/fmt.hpp"
#include "data_variable.hpp"
#include "executor_structs.h"
#include "frontend_layer.h"
#include "nimble_net_util.hpp"

template <typename Func, typename... Args>
static void call_function(Func f, Args&&... args) {
  if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>) {
    f(std::forward<Args>(args)...);
  } else {
    handle_nimblenet_status(f(std::forward<Args>(args)...));
  }
}

static void handle_nimblenet_status(NimbleNetStatus* status) {
  if (!status) return;

  std::string message = status->message;
  deallocate_ios_nimblenet_status_global(status);
  THROW("Got error from ios layer: %s", message.c_str());
}

#define CALL_IOS_FUNCTION(f, ...)          \
  do {                                     \
    if (!f) {                              \
      THROW("Function " #f " is not set"); \
    }                                      \
    call_function(f, __VA_ARGS__);         \
  } while (0)


CTensor IOSHelper::get_string_subscript(IosObject obj, const std::string& key) {
  CTensor ret;
  CALL_IOS_FUNCTION(get_ios_object_string_subscript_global, obj, key.c_str(), &ret);
  return ret;
}

CTensor IOSHelper::get_int_subscript(IosObject obj, int idx) {
  CTensor ret;
  CALL_IOS_FUNCTION(get_ios_object_int_subscript_global, obj, idx, &ret);
  return ret;
}

int IOSHelper::get_size(IosObject obj) {
  int val;
  CALL_IOS_FUNCTION(get_ios_object_size_global, obj, &val);
  return val;
}

void IOSHelper::deallocate_cTensor(CTensor* cTensor) {
  CALL_IOS_FUNCTION(deallocate_frontend_ctensor_global, cTensor);
}

void IOSHelper::set_subscript(IosObject obj, const std::string& key, OpReturnType value) {
  const char* name = "value";
  CTensor valueTensor = value->to_cTensor(const_cast<char*>("value"), &value);
  CALL_IOS_FUNCTION(set_ios_object_string_subscript_global, obj, key.c_str(), &valueTensor);
}

void IOSHelper::set_subscript(IosObject obj, int idx, OpReturnType value) {
  const char* name = "value";
  CTensor valueTensor = value->to_cTensor(const_cast<char*>("value"), &value);
  CALL_IOS_FUNCTION(set_ios_object_int_subscript_global, obj, idx, &valueTensor);
}

std::string IOSHelper::to_string(IosObject obj) {
  char* str = nullptr;
  CALL_IOS_FUNCTION(ios_object_to_string_global, obj, &str);
  auto cppStr = std::string{str};
  free(str);
  return cppStr;
}

IosObject IOSHelper::arrange(IosObject obj, const std::vector<int>& list) {
  IosObject newObj;
  CALL_IOS_FUNCTION(ios_object_arrange_global, obj, list.data(), list.size(), &newObj);
  return newObj;
}

bool IOSHelper::in(IosObject obj, const std::string& key) {
  bool result;
  CALL_IOS_FUNCTION(in_ios_object_global, obj, key.c_str(), &result);
  return result;
}

void IOSHelper::release(IosObject obj) { CALL_IOS_FUNCTION(release_ios_object_global, obj); }

CTensor IOSHelper::get_keys(IosObject obj) {
  CTensor value;
  CALL_IOS_FUNCTION(get_keys_ios_object_global, obj, &value);
  return value;
}
