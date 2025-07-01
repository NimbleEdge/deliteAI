/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frontend_data_variable.hpp"

#include "ios_reference_data_variable.hpp"
#include "nimble_net_util.hpp"

OpReturnType FrontendDataVariable::create(void* obj) {
  auto iosObj = static_cast<IosObject*>(obj);
  return OpReturnType(new IOSReferenceDataVariable{std::move(*iosObj)});
}
