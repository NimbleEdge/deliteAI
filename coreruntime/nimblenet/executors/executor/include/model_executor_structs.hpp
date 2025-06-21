/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>

#include "nlohmann_json.hpp"
#include "user_events_struct.hpp"

using json = nlohmann::json;

/**
 * @brief Represents metadata about a tensor.
 */
struct TensorInfo {
  std::string name;           /**< Name of the tensor. */
  int dataType;               /**< Data type of the tensor (e.g., float, int64, etc.). */
  std::vector<int64_t> shape; /**< Shape of the tensor as a vector of dimensions. */
  int size;                   /**< Total number of elements in the tensor. */
  json preprocessorJson; /**< JSON description of any preprocessor configuration for the tensor. */
  bool toPreprocess =
      false; /**< Indicates whether preprocessing should be applied to this tensor. */
};

/**
 * @brief Describes how a preprocessor input maps to model inputs.
 */
struct PreProcessorInputInfo {
  std::string name; /**< Name of the preprocessor input. */
  std::vector<std::string>
      inputNames; /**< List of model input names this preprocessor input is mapped to. */
};

/**
 * @brief Stores the inputs and outputs of a machine learning model.
 */
struct ModelInfo {
  bool valid = false;              /**< Indicates whether struct is valid. */
  std::vector<TensorInfo> inputs;  /**< List of input tensor metadata. */
  std::vector<TensorInfo> outputs; /**< List of output tensor metadata. */
  std::vector<PreProcessorInputInfo>
      preprocessorInputs; /**< List of preprocessor input mappings. */
};

/**
 * @brief Holds a reference to a model input and its associated tensor information.
 */
struct SavedInputTensor {
  std::shared_ptr<ModelInput> modelInput; /**< Shared pointer to the model input. */
  TensorInfo* tensorInfoPtr;              /**< Pointer to the tensor metadata for the input. */

  /**
   * @brief Constructor for SavedInputTensor.
   *
   * @param modelInput_ Shared pointer to the model input.
   * @param tensorInfo_ Pointer to the corresponding tensor metadata.
   */
  SavedInputTensor(std::shared_ptr<ModelInput> modelInput_, TensorInfo* tensorInfo_) {
    tensorInfoPtr = tensorInfo_;
    modelInput = modelInput_;
  }
};

/**
 * @brief Deserialize JSON into a TensorInfo structure.
 *
 * @param j JSON object.
 * @param tensorInfo TensorInfo instance to populate.
 */
void from_json(const json& j, TensorInfo& tensorInfo);

/**
 * @brief Deserialize JSON into a PreProcessorInputInfo structure.
 *
 * @param j JSON object.
 * @param preProcessorInputInfo PreProcessorInputInfo instance to populate.
 */
void from_json(const json& j, PreProcessorInputInfo& preProcessorInputInfo);

/**
 * @brief Deserialize JSON into a ModelInfo structure.
 *
 * @param j JSON object.
 * @param modelInfo ModelInfo instance to populate.
 */
void from_json(const json& j, ModelInfo& modelInfo);

/**
 * @brief Serialize a TensorInfo structure into JSON.
 *
 * @param j JSON object to populate.
 * @param tensorInfo TensorInfo instance to serialize.
 */
void to_json(json& j, const TensorInfo& tensorInfo);
