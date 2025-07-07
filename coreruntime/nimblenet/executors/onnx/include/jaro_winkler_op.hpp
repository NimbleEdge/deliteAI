/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "onnx.hpp"

/**
 * @brief Computes the Jaro similarity between two strings.
 *
 * Jaro similarity is a metric for measuring the similarity between two strings,
 * based on matches and transpositions within a specified character distance.
 *
 * @param s1 First string.
 * @param s2 Second string.
 * @return Jaro similarity score between 0.0 (no similarity) and 1.0 (identical).
 */
static double jaro(const std::string& s1, const std::string& s2) {
  int s1_len = s1.size();
  int s2_len = s2.size();

  if (s1_len == 0 || s2_len == 0) {
    return 0.0;
  }

  int match_distance = std::max(s1_len, s2_len) / 2 - 1;

  std::vector<bool> s1_matches(s1_len, false);
  std::vector<bool> s2_matches(s2_len, false);

  int matches = 0;
  int transpositions = 0;

  // Count matching characters within allowed distance
  for (int i = 0; i < s1_len; ++i) {
    int start = std::max(0, i - match_distance);
    int end = std::min(i + match_distance + 1, s2_len);

    for (int j = start; j < end; ++j) {
      if (s2_matches[j]) continue;
      if (s1[i] != s2[j]) continue;
      s1_matches[i] = true;
      s2_matches[j] = true;
      ++matches;
      break;
    }
  }

  if (matches == 0) {
    return 0.0;
  }

  // Count transpositions
  int k = 0;
  for (int i = 0; i < s1_len; ++i) {
    if (!s1_matches[i]) continue;
    while (!s2_matches[k]) ++k;
    if (s1[i] != s2[k]) ++transpositions;
    ++k;
  }

  double m = (double)matches;
  return ((m / s1_len) + (m / s2_len) + ((m - transpositions / 2.0) / m)) / 3.0;
}

/**
 * @brief Computes the Jaro-Winkler similarity between two strings.
 *
 * Extends the Jaro similarity by giving more weight to common prefixes.
 *
 * @param s1 First string.
 * @param s2 Second string.
 * @return Jaro-Winkler similarity score between 0.0 and 1.0.
 */
static float jaro_winkler(const std::string& s1, const std::string& s2) {
  double j = jaro(s1, s2);

  int prefix = 0;
  for (int i = 0; i < std::min(4, (int)std::min(s1.size(), s2.size())); ++i) {
    if (s1[i] == s2[i])
      ++prefix;
    else
      break;
  }

  return j + (prefix * 0.1 * (1 - j));
}

/**
 * @brief Kernel implementation for the JaroWinkler ONNX custom operator.
 *
 * Computes the Jaro-Winkler similarity between an input string and each entry in a vocabulary
 * tensor.
 */
struct JaroWinklerOpKernel {
  /**
   * @brief Perform the similarity computation.
   *
   * Takes a single input string and a list of vocabulary strings, and fills the output tensor
   * with Jaro-Winkler similarity scores.
   *
   * @param context ONNX runtime kernel context.
   */
  void Compute(OrtKernelContext* context) {
    Ort::KernelContext ctx(context);
    auto inputString = ctx.GetInput(0);
    auto vocabTensor = ctx.GetInput(1);

    const std::string& string_in = inputString.GetStringTensorElement(0);

    auto dimensions = vocabTensor.GetTensorTypeAndShapeInfo().GetShape();
    auto length = vocabTensor.GetTensorTypeAndShapeInfo().GetElementCount();
    auto distances_out_tensor = ctx.GetOutput(0, dimensions);
    float* distances_out = distances_out_tensor.GetTensorMutableData<float>();

    for (int i = 0; i < length; i++) {
      std::string vocab_string = vocabTensor.GetStringTensorElement(i);
      distances_out[i] = jaro_winkler(string_in, vocab_string);
    }
  }
};

/**
 * @brief Registers the JaroWinkler ONNX custom operator.
 *
 * Provides information about inputs, outputs, name, and execution provider.
 */
struct JaroWinklerOp : Ort::CustomOpBase<JaroWinklerOp, JaroWinklerOpKernel> {
  /**
   * @brief Creates the kernel instance.
   *
   * @return Pointer to the created kernel.
   */
  void* CreateKernel(const OrtApi& /* api */, const OrtKernelInfo* /* info */) const {
    return std::make_unique<JaroWinklerOpKernel>().release();
  }

  /**
   * @brief Returns the name of the custom operator.
   *
   * @return Operator name as C-string.
   */
  const char* GetName() const { return "JaroWinkler"; }

  /**
   * @brief Specifies the execution provider.
   *
   * @return "CPUExecutionProvider".
   */
  const char* GetExecutionProviderType() const { return "CPUExecutionProvider"; }

  /**
   * @brief Returns the number of input tensors.
   *
   * @return Number of inputs (2).
   */
  size_t GetInputTypeCount() const { return 2; }

  /**
   * @brief Returns the type of input tensor at the specified index.
   *
   * @param index Index of the input tensor.
   * @return ONNX tensor element type (string).
   */
  ONNXTensorElementDataType GetInputType(size_t index) const {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;
  }

  /**
   * @brief Returns the number of output tensors.
   *
   * @return Number of outputs (1).
   */
  size_t GetOutputTypeCount() const { return 1; }

  /**
   * @brief Returns the type of output tensor at the specified index.
   *
   * @param index Index of the output tensor.
   * @return ONNX tensor element type (float).
   */
  ONNXTensorElementDataType GetOutputType(size_t index) const {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
  }
};
