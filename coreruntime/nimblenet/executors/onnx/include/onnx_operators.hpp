/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "onnx.hpp"
#ifdef CUSTOM_STRING_OPS
#include "jaccard_similarity_op.hpp"
#include "jaro_winkler_op.hpp"
#endif

static inline void register_custom_onnx_operators(Ort::CustomOpDomain& domain) {
#ifdef CUSTOM_STRING_OPS
  static const JaroWinklerOp c_jaroWinklerOp;
  static const JaccardSimilarityOp c_jacardSimilarityOp;
  domain.Add(&c_jaroWinklerOp);
  domain.Add(&c_jacardSimilarityOp);
#endif
}
