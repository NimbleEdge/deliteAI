/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "custom_functions.hpp"

#include "enumerate_data_variable.hpp"
#include "exception_data_variable.hpp"
#include "statements.hpp"

std::map<std::string, CustomFuncPtr> CustomFunctions::_customFuncMap = {
    {"print", CustomFunctions::print},
    {"range", CustomFunctions::range},
    {"str", CustomFunctions::str},
    {"not", CustomFunctions::inverse_bool},
    {"float", CustomFunctions::cast_float},
    {"bool", CustomFunctions::cast_bool},
    {"int", CustomFunctions::cast_int},
    {"len", CustomFunctions::len},
    {"concurrent", CustomFunctions::concurrent},
    {"add_event", CustomFunctions::add_event},
    {"pre_add_event", CustomFunctions::pre_add_event_hook},
    {"Exception", CustomFunctions::create_exception},
    {"next", CustomFunctions::next},
    {"enumerate", CustomFunctions::enumerate},
};

OpReturnType CustomFunctions::concurrent(const std::vector<OpReturnType>& arguments,
                                         CallStack& stack) {
  if (arguments.size() != 1)
    THROW("decorator function args should have size 1 given %d.", arguments.size());
  auto functionDataVariable = static_cast<FunctionDataVariable*>(arguments[0].get());
  functionDataVariable->set_static();
  return arguments[0];
}

OpReturnType CustomFunctions::create_exception(const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) {
  if (arguments.size() != 1)
    THROW("Exception constructor should have size 1 given %d.", arguments.size());
  return std::make_shared<ExceptionDataVariable>(arguments[0]->get_string());
}

OpReturnType CustomFunctions::pre_add_event_hook(const std::vector<OpReturnType>& typesDataVariable,
                                                 CallStack& stack) {
  CustomStdFunction myLambda = [typesDataVariable](const std::vector<OpReturnType>& arguments,
                                                   CallStack& stack) -> OpReturnType {
    THROW_ARGUMENTS_MISMATCH_FUNCTION_NAME(typesDataVariable.size(), 1, "pre_add_event");
    THROW_ARGUMENTS_MISMATCH_FUNCTION_NAME(arguments.size(), 1, "@pre_add_event_hook");

    auto data = typesDataVariable[0];
    if (data->get_containerType() != CONTAINERTYPE::LIST) {
      THROW("pre_add_event decorator accepts argument of the type list. Provided: %s",
            data->get_containerType_string());
    }
    std::vector<std::string> types;

    int listSize = data->get_size();
    for (int i = 0; i < listSize; i++) {
      auto s = data->get_int_subscript(i);
      if (s->get_dataType_enum() != DATATYPE::STRING) {
        THROW("Only string data type variables can be defined as types for preAddEvent hook got %s",
              util::get_string_from_enum(s->get_dataType_enum()));
      }
      types.push_back(s->get_string());
    }

    auto commandCenter = stack.command_center();
    commandCenter->get_userEventsManager().add_pre_event_hook(arguments[0], std::move(types));
    return arguments[0];
  };
  return OpReturnType(new CustomFuncDataVariable(std::move(myLambda)));
}

OpReturnType CustomFunctions::add_event(const std::vector<OpReturnType>& rawStoreDataVariables,
                                        CallStack& stack) {
  // Define a lambda function matching the signature
  CustomStdFunction myLambda = [rawStoreDataVariables](const std::vector<OpReturnType>& arguments,
                                                       CallStack& stack) -> OpReturnType {
    if (arguments.size() != 1)
      THROW("decorator function args should have size 1 given %d.", arguments.size());
    auto functionDataVariable = arguments[0];
    for (auto& rawStoreDataVar : rawStoreDataVariables) {
      if (rawStoreDataVar->get_dataType_enum() != DATATYPE::RAW_EVENTS_STORE) {
        THROW("RawEventStore required for add_event decorator dataType=%s given",
              util::get_string_from_enum(rawStoreDataVar->get_dataType_enum()));
      }
      RawEventStoreDataVariable* var =
          static_cast<RawEventStoreDataVariable*>(rawStoreDataVar.get());
      var->set_add_event_hook(functionDataVariable);
    }
    return arguments[0];
  };
  return OpReturnType(new CustomFuncDataVariable(std::move(myLambda)));
}

OpReturnType CustomFunctions::next(const std::vector<OpReturnType>& args, CallStack& stack) {
  THROW_ARGUMENTS_MISMATCH_FUNCTION_NAME(args.size(), 1, "next");

  if (args[0]->is_iterable()) {
    try {
      return args[0]->next(stack);
    } catch (const std::runtime_error& e) {
      if (std::string(e.what()) == "StopIteration") return nullptr;
      throw;
    }
  }
  THROW("next expects an iterable argument, provided %s", args[0]->get_containerType_string());
}

OpReturnType CustomFunctions::enumerate(const std::vector<OpReturnType>& args, CallStack& stack) {
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH_FUNCTION_NAME(args.size(), 1, 2, "enumerate");

  OpReturnType iterable = args[0];

  int startIndex = 0;
  if (args.size() == 2) {
    startIndex = args[1]->get_int32();
  }

  return OpReturnType(new EnumerateDataVariable(iterable, startIndex));
}
