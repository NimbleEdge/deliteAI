/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "onnx.hpp"

/**
 * @brief Compute the Jaccard similarity between two strings.
 *
 * The similarity is computed based on character sets. It builds a set from `otherStr`
 * and compares it to a precomputed set (`charSet`) of the input string.
 *
 * @param otherStr The second string to compare against.
 * @param charSet A binary set indicating the presence of characters in the first string.
 * @param str1UnionSize The number of unique characters in the first string.
 * @return Jaccard similarity score as a float.
 */
static float jaccard_new(const std::string& otherStr, char* charSet, int str1UnionSize) {
  int intersectionSize = 0;
  int unionSize = str1UnionSize;
  char otherSet[256] = {0};

  for (char c : otherStr) {
    unsigned char uc = static_cast<unsigned char>(c);
    if (!otherSet[uc]) {
      otherSet[uc] = 1;

      if (charSet[uc]) {
        ++intersectionSize;
      } else {
        ++unionSize;
      }
    }
  }

  if (unionSize == 0) {
    return 0;
  }
  return (static_cast<float>(intersectionSize) / static_cast<float>(unionSize));
}

/**
 * @brief Kernel implementation for the JaccardSimilarity custom ONNX operator.
 *
 * This struct defines the actual logic for computing Jaccard similarity between
 * an input string and each string in a vocabulary tensor.
 */
struct JaccardSimilarityOpKernel {
  /**
   * @brief Perform computation for Jaccard similarity.
   *
   * Takes a single input string and a vocabulary of strings, and produces a float tensor
   * of similarity scores.
   *
   * @param context ONNX kernel context containing input and output tensors.
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
    char charSet[256];
    int unionSize = 0;
    memset(charSet, 0, 256);
    // Build character set from input string
    for (int i = 0; i < string_in.size(); i++) {
      if (charSet[static_cast<unsigned char>(string_in[i])] == 0) {
        charSet[static_cast<unsigned char>(string_in[i])] = 1;
        unionSize++;
      }
    }

    // Compute similarity against each vocabulary entry
    for (int i = 0; i < length; i++) {
      std::string vocab_string = vocabTensor.GetStringTensorElement(i);
      distances_out[i] = jaccard_new(vocab_string, charSet, unionSize);
    }
  }
};

/**
 * @brief Custom operator registration for the JaccardSimilarity ONNX operator.
 *
 * Registers the kernel, defines the operator name, types, and provider.
 */
struct JaccardSimilarityOp : Ort::CustomOpBase<JaccardSimilarityOp, JaccardSimilarityOpKernel> {
  /**
   * @brief Create an instance of the kernel.
   *
   * @return Pointer to the created kernel.
   */
  void* CreateKernel(const OrtApi& /* api */, const OrtKernelInfo* /* info */) const {
    return std::make_unique<JaccardSimilarityOpKernel>().release();
  };

  /**
   * @brief Get the name of the operator.
   *
   * @return Operator name as a C-string.
   */
  const char* GetName() const { return "JaccardSimilarity"; };

  /**
   * @brief Get the execution provider type.
   *
   * @return "CPUExecutionProvider".
   */
  const char* GetExecutionProviderType() const { return "CPUExecutionProvider"; };

  /**
   * @brief Get the number of input tensors expected.
   *
   * @return Number of input tensors.
   */
  size_t GetInputTypeCount() const { return 2; };

  /**
   * @brief Get the data type of a specific input.
   *
   * @param index Index of the input tensor.
   * @return Data type (string).
   */
  ONNXTensorElementDataType GetInputType(size_t index /*index*/) const {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;
  };

  /**
   * @brief Get the number of output tensors produced.
   *
   * @return Number of output tensors.
   */
  size_t GetOutputTypeCount() const { return 1; };

  /**
   * @brief Get the data type of a specific output.
   *
   * @param index Index of the output tensor.
   * @return Data type (float).
   */
  ONNXTensorElementDataType GetOutputType(size_t index /*index*/) const {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
  };
};
