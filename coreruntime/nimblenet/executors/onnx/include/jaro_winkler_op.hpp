/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "onnx.hpp"

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

// static void compute_distance(const std::string& str, const std::vector<std::string>& vec,
//                              float* jarowinklerArray) {
//   int i = 0;
//   for (const auto& s : vec) {
//     auto distance = jaro_winkler(str, s);
//     jarowinklerArray[i] = distance;
//     i++;
//   }
// }

/* Newer Method to Write Custom Operator for ONNX Runtime

struct JaroWinklerOp {
  JaroWinklerOp(const OrtApi* ort_api, const OrtKernelInfo* info) {}

  // a "Compute" member function is required to be present
  void Compute(const Ort::Custom::Tensor<std::string>& strings_in,
               const Ort::Custom::Tensor<std::string>& vocabulary_in,
               // const Ort::Custom::Tensor<std::string>& strings_in3,
               Ort::Custom::Tensor<float>& distance_out) {
    auto jaroDataArray =
        distance_out.Allocate({static_cast<int64_t>(vocabulary_in.NumberOfElement())});
    compute_distance(strings_in.Data()[0], vocabulary_in.Data(), jaroDataArray);

    // strings_out->SetStringOutput(string_pool, {static_cast<int64_t>(string_pool.size())});
  }
};

void ElementWiseAddCompute(const OrtKernelContext* context) {
  Ort::CustomOpApi ort;
  // OrtKernelContext* context = reinterpret_cast<OrtKernelContext*>(op_kernel_context);

  const OrtValue* input_X = ort.KernelContext_GetInput(context, 0);
  const OrtValue* input_Y = ort.KernelContext_GetInput(context, 1);

  OrtTensorDimensions dimensions(ort, input_X);
  OrtValue* output = ort.KernelContext_GetOutput(context, 0, dimensions.data(),
  dimensions.size());

  float* X = ort.GetTensorMutableData<float>(input_X);
  float* Y = ort.GetTensorMutableData<float>(input_Y);
  float* Z = ort.GetTensorMutableData<float>(output);

  size_t len = dimensions.Size();
  for (size_t i = 0; i < len; ++i) {
    Z[i] = X[i] + Y[i];
  }
}
*/

struct JaroWinklerOpKernel {
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

// legacy custom op registration
struct JaroWinklerOp : Ort::CustomOpBase<JaroWinklerOp, JaroWinklerOpKernel> {
  void* CreateKernel(const OrtApi& /* api */, const OrtKernelInfo* /* info */) const {
    return std::make_unique<JaroWinklerOpKernel>().release();
  };

  const char* GetName() const { return "JaroWinkler"; };

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
