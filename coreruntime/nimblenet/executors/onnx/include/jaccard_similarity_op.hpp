/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "onnx.hpp"

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

struct JaccardSimilarityOpKernel {
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
    for (int i = 0; i < string_in.size(); i++) {
      if (charSet[static_cast<unsigned char>(string_in[i])] == 0) {
        charSet[static_cast<unsigned char>(string_in[i])] = 1;
        unionSize++;
      }
    }

    for (int i = 0; i < length; i++) {
      std::string vocab_string = vocabTensor.GetStringTensorElement(i);
      distances_out[i] = jaccard_new(vocab_string, charSet, unionSize);
    }
  }
};

// legacy custom op registration
struct JaccardSimilarityOp : Ort::CustomOpBase<JaccardSimilarityOp, JaccardSimilarityOpKernel> {
  void* CreateKernel(const OrtApi& /* api */, const OrtKernelInfo* /* info */) const {
    return std::make_unique<JaccardSimilarityOpKernel>().release();
  };

  const char* GetName() const { return "JaccardSimilarity"; };

  const char* GetExecutionProviderType() const { return "CPUExecutionProvider"; };

  size_t GetInputTypeCount() const { return 2; };

  ONNXTensorElementDataType GetInputType(size_t index /*index*/) const {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING;
  };

  size_t GetOutputTypeCount() const { return 1; };

  ONNXTensorElementDataType GetOutputType(size_t index /*index*/) const {
    return ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
  };
};
