/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using json = nlohmann::json;

void to_json(json& j, const ModelData& modelData) {
  j = json{{"version", modelData.version}, {"model", modelData.model}};
}

void from_json(const json& j, ModelData& modelData) {
  j.at("model").get_to(modelData.model);
  j.at("version").get_to(modelData.version);
}
