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
#endif  // CUSTOM_STRING_OPS

/**
 * @brief Registers custom ONNX operators with the given domain.
 *
 * This function conditionally registers string similarity operators
 * (Jaro-Winkler and Jaccard) into the ONNX Runtime if CUSTOM_STRING_OPS
 * is defined at compile time. The operators are added to the provided
 * custom operator domain, making them available during ONNX model execution.
 *
 * @param domain The Ort::CustomOpDomain to which custom ops will be added.
 */
static inline void register_custom_onnx_operators(Ort::CustomOpDomain& domain) {
#ifdef CUSTOM_STRING_OPS
  static const JaroWinklerOp c_jaroWinklerOp;            /**< Custom Jaro-Winkler operator */
  static const JaccardSimilarityOp c_jacardSimilarityOp; /**< Custom Jaccard Similarity operator */
  domain.Add(&c_jaroWinklerOp);
  domain.Add(&c_jacardSimilarityOp);
#endif  // CUSTOM_STRING_OPS
}
