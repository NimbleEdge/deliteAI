#pragma once

#include <utility>

#include "data_variable.hpp"

typedef OpReturnType (*CustomFuncPtr)(const std::vector<OpReturnType>&, CallStack&);
typedef std::function<OpReturnType(const std::vector<OpReturnType>&, CallStack&)> CustomStdFunction;

// TODO (puneet): Rename to ForeignFunctionDataVariable
/**
 * @brief A foreign function.
 *
 * A function not defined in the DelitePy language.
 * It can be:
 * <ul>
 *   <li>a built-in function, bundled with the DelitePy interpreter; typically written in C++</li>
 *   <li>a client provided function, fed into the interpreter; typically written in
 * Swift/Kotlin/Python/C++</li>
 * </ul>
 */
class CustomFuncDataVariable final : public DataVariable {
  CustomStdFunction _func;

  int get_dataType_enum() const final { return DATATYPE::FUNCTION; }

  int get_containerType() const final { return CONTAINERTYPE::FUNCTIONDEF; }

  bool get_bool() final { return true; }

  nlohmann::json to_json() const override { return "[Function]"; }

 public:
  explicit CustomFuncDataVariable(const CustomStdFunction& func) { _func = func; }

  explicit CustomFuncDataVariable(CustomStdFunction&& func) { _func = std::move(func); }

  virtual ~CustomFuncDataVariable() = default;

  OpReturnType execute_function(const std::vector<OpReturnType>& arguments,
                                CallStack& stack) override;

  std::string print() override { return fallback_print(); }
};
