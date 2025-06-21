/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nimblejson.hpp"

#include "list_data_variable.hpp"
#include "map_data_variable.hpp"
#include "nimble_net_util.hpp"
#include "nlohmann_json.hpp"
#include "single_variable.hpp"

namespace nimblejson {

struct JsonAllocator {
  std::vector<OpReturnType*> elements;
  std::vector<JsonIterator*> iterators;
  std::vector<JsonOutput*> jsonOutputs;

  JsonAllocator() {}

  ~JsonAllocator() {
    for (auto elem : elements) {
      delete elem;
    }
    for (auto iter : iterators) {
      delete iter;
    }
    for (auto out : jsonOutputs) {
      delete out;
    }
  }
};

// Json Allocator for storing context
void* create_json_allocator() {
  JsonAllocator* jsonAllocator = new JsonAllocator();
  return (void*)(jsonAllocator);
}

void deallocate_json_allocator(void* json_allocator) {
  JsonAllocator* jsonAllocator = static_cast<JsonAllocator*>(json_allocator);
  delete jsonAllocator;
}

// Create Json object and array
void* create_json_object(void* json_allocator) {
  OpReturnType* map = new OpReturnType(new MapDataVariable());
  JsonAllocator* jsonAllocator = static_cast<JsonAllocator*>(json_allocator);
  jsonAllocator->elements.push_back(map);
  return (void*)map;
}

void* create_json_array(void* json_allocator) {
  OpReturnType* list = new OpReturnType(new ListDataVariable());
  JsonAllocator* jsonAllocator = static_cast<JsonAllocator*>(json_allocator);
  jsonAllocator->elements.push_back(list);
  return (void*)list;
}

// Move elements to json array
bool move_json_object_or_array_to_array(void* jsonArray, void* json_object) {
  OpReturnType list = *static_cast<OpReturnType*>(jsonArray);
  OpReturnType map_or_list = *static_cast<OpReturnType*>(json_object);
  if (list->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  if (map_or_list->get_containerType() != CONTAINERTYPE::MAP &&
      map_or_list->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  list->append(map_or_list);
  return true;
}

bool move_int64_value_to_array(void* jsonArray, const int64_t value) {
  OpReturnType list = *static_cast<OpReturnType*>(jsonArray);
  if (list->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  list->append(OpReturnType(new SingleVariable<int64_t>(value)));
  return true;
}

bool move_double_value_to_array(void* jsonArray, const double value) {
  OpReturnType list = *static_cast<OpReturnType*>(jsonArray);
  if (list->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  list->append(OpReturnType(new SingleVariable<double>(value)));
  return true;
}

bool move_string_value_to_array(void* jsonArray, const char* value) {
  OpReturnType list = *static_cast<OpReturnType*>(jsonArray);
  if (list->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  list->append(OpReturnType(new SingleVariable<std::string>(value)));
  return true;
}

bool move_bool_value_to_array(void* jsonArray, const bool value) {
  OpReturnType list = *static_cast<OpReturnType*>(jsonArray);
  if (list->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  list->append(OpReturnType(new SingleVariable<bool>(value)));
  return true;
}

bool move_null_value_to_array(void* jsonArray) {
  OpReturnType list = *static_cast<OpReturnType*>(jsonArray);
  if (list->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  list->append(std::make_shared<NoneVariable>());
  return true;
}

// Add values to json object
bool add_string_value(const char* key, const char* value, void* json) {
  OpReturnType map = *static_cast<OpReturnType*>(json);
  if (map->get_containerType() != CONTAINERTYPE::MAP) {
    return false;
  }
  map->set_value_in_map(key, OpReturnType(new SingleVariable<std::string>(value)));
  return true;
}

bool add_int64_value(const char* key, const int64_t value, void* json) {
  OpReturnType map = *static_cast<OpReturnType*>(json);
  if (map->get_containerType() != CONTAINERTYPE::MAP) {
    return false;
  }
  map->set_value_in_map(key, OpReturnType(new SingleVariable<int64_t>(value)));
  return true;
}

bool add_double_value(const char* key, const double value, void* json) {
  OpReturnType map = *static_cast<OpReturnType*>(json);
  if (map->get_containerType() != CONTAINERTYPE::MAP) {
    return false;
  }
  map->set_value_in_map(key, OpReturnType(new SingleVariable<double>(value)));
  return true;
}

bool add_bool_value(const char* key, const bool value, void* json) {
  OpReturnType map = *static_cast<OpReturnType*>(json);
  if (map->get_containerType() != CONTAINERTYPE::MAP) {
    return false;
  }
  map->set_value_in_map(key, OpReturnType(new SingleVariable<bool>(value)));
  return true;
}

bool add_null_value(const char* key, void* json) {
  OpReturnType map = *static_cast<OpReturnType*>(json);
  if (map->get_containerType() != CONTAINERTYPE::MAP) {
    return false;
  }
  map->set_value_in_map(key, std::make_shared<NoneVariable>());
  return true;
}

// Add json object/array to json object
bool add_json_object_to_json(const char* key, void* value, void* json) {
  OpReturnType map = *static_cast<OpReturnType*>(json);
  OpReturnType new_map_object = *static_cast<OpReturnType*>(value);
  if (map->get_containerType() != CONTAINERTYPE::MAP) {
    return false;
  }
  if (new_map_object->get_containerType() != CONTAINERTYPE::MAP &&
      new_map_object->get_containerType() != CONTAINERTYPE::LIST) {
    return false;
  }
  map->set_value_in_map(key, new_map_object);
  return true;
}

void* create_json_iterator(void* json, void* json_allocator) {
  OpReturnType map_or_list = *static_cast<OpReturnType*>(json);
  JsonIterator* jsonIterator;
  if (map_or_list->get_containerType() == CONTAINERTYPE::MAP ||
      map_or_list->get_containerType() == CONTAINERTYPE::LIST) {
    jsonIterator = map_or_list->get_json_iterator();
  } else {
    return nullptr;
  }
  JsonAllocator* jsonAllocator = static_cast<JsonAllocator*>(json_allocator);
  jsonAllocator->iterators.push_back(jsonIterator);
  return (void*)(jsonIterator);
}

void* get_next_json_element(void* jsonIterator, void* jsonAllocator) {
  JsonIterator* iter = static_cast<JsonIterator*>(jsonIterator);
  JsonAllocator* json_allocator = static_cast<JsonAllocator*>(jsonAllocator);
  JsonOutput* jsonOutput = new JsonOutput();
  json_allocator->jsonOutputs.push_back(jsonOutput);
  OpReturnType value;

  if (!iter->isArray) {
    std::map<std::string, OpReturnType>::iterator beginIter = iter->map_begin;
    std::map<std::string, OpReturnType>::iterator endIter = iter->map_end;
    jsonOutput->isEnd = false;
    if (beginIter == endIter) {
      jsonOutput->isEnd = true;
      return jsonOutput;
    }
    assert(beginIter != endIter);
    jsonOutput->key = (beginIter->first).data();
    value = beginIter->second;
    iter->map_begin++;
  } else {
    std::vector<OpReturnType>::iterator beginIter = iter->list_begin;
    std::vector<OpReturnType>::iterator endIter = iter->list_end;
    jsonOutput->isEnd = false;
    if (beginIter == endIter) {
      jsonOutput->isEnd = true;
      return jsonOutput;
    }
    assert(beginIter != endIter);
    value = *beginIter;
    iter->list_begin++;
  }

  if (value->get_containerType() == CONTAINERTYPE::LIST) {
    jsonOutput->dataType = DATATYPE::JSON_ARRAY;
    JsonIterator* jsonArrayIterator = value->get_json_iterator();
    jsonOutput->value.obj = (void*)jsonArrayIterator;
    json_allocator->iterators.push_back(jsonArrayIterator);
  } else if (value->get_containerType() == CONTAINERTYPE::MAP) {
    jsonOutput->dataType = DATATYPE::JSON;
    JsonIterator* jsonMapIterator = value->get_json_iterator();
    jsonOutput->value.obj = (void*)jsonMapIterator;
    json_allocator->iterators.push_back(jsonMapIterator);
  } else if (value->get_containerType() == CONTAINERTYPE::SINGLE) {
    switch (value->get_dataType_enum()) {
      case DATATYPE::INT32:
      case DATATYPE::INT64: {
        jsonOutput->dataType = DATATYPE::INT64;
        jsonOutput->value.i = value->get_int64();
        break;
      }
      case DATATYPE::STRING: {
        jsonOutput->dataType = DATATYPE::STRING;
        auto stringValue = std::dynamic_pointer_cast<SingleVariable<std::string>>(value);
        assert(stringValue != nullptr);
        jsonOutput->value.s = stringValue->get_c_str();
        break;
      }
      case DATATYPE::BOOLEAN: {
        jsonOutput->dataType = DATATYPE::BOOLEAN;
        jsonOutput->value.b = value->get_bool();
        break;
      }
      case DATATYPE::FLOAT:
      case DATATYPE::DOUBLE: {
        jsonOutput->dataType = DATATYPE::DOUBLE;
        jsonOutput->value.d = value->get_double();
        break;
      }
      case DATATYPE::NONE: {
        jsonOutput->dataType = DATATYPE::NONE;
        jsonOutput->value.obj = nullptr;
        break;
      }
      default:
        break;
    }
  }
  return (void*)jsonOutput;
}

}  // namespace nimblejson
