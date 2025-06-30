#pragma once

#include <functional>

#include "custom_func_data_variable.hpp"
#include "range_data_variable.hpp"
#include "raw_event_store_data_variable.hpp"
#include "single_variable.hpp"
#include "variable_scope.hpp"

class CustomFunctions {
  static std::map<std::string, CustomFuncPtr> _customFuncMap;
  friend class InbuiltFunctionsStatement;

 public:
/*
DELITEPY_DOC_BLOCK_BEGIN
- `print()` function
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType print(const std::vector<OpReturnType>& args, CallStack& stack) {
    std::string printVar = "";
    for (auto d : args) {
      printVar += d->print() + " ";
    }
    LOG_TO_CLIENT_DEBUG("%s", printVar.c_str());
    return OpReturnType(new NoneVariable());
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `range()` function
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType range(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("%s", "range expects only a single argument");
    }
    int size = args[0]->get_int32();
    if (size < 0) {
      THROW("range should be +ve got %d", size);
    }
    return OpReturnType(new RangeDataVariable(size));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `not` operator
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType inverse_bool(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("%s", "not expects only a single argument");
    }
    return OpReturnType(new SingleVariable<bool>(!args[0]->get_bool()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `str()` constructor
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType str(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("str expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<std::string>(args[0]->print()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `int()` constructor
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType cast_int(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("int expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<int32_t>(args[0]->cast_int32()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `float()` constructor
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType cast_float(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("float expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<float>(args[0]->cast_float()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `bool()` constructor
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType cast_bool(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("bool expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<bool>(args[0]->get_bool()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `len()` function
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType len(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("len expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<int32_t>(args[0]->get_size()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `Exception` class
DELITEPY_DOC_BLOCK_END
*/
  static OpReturnType create_exception(const std::vector<OpReturnType>& arguments,
                                       CallStack& stack);

  // this is a decorator function. This should return a callable function datavariable
  static OpReturnType add_event(const std::vector<OpReturnType>& rawStoreDataVariables,
                                CallStack& stack);

/*
DELITEPY_DOC_BLOCK_BEGIN
- `concurrent()` decorator
DELITEPY_DOC_BLOCK_END
*/
  // this is a decorator function. This should return a callable function datavariable
  static OpReturnType concurrent(const std::vector<OpReturnType>& arguments, CallStack& stack);

  static OpReturnType pre_add_event_hook(const std::vector<OpReturnType>& typesDataVariable,
                                         CallStack& stack);
};
