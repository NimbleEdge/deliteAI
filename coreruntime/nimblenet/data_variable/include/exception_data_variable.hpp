#include "data_variable.hpp"

class ExceptionDataVariable final : public DataVariable {
  std::string _errorMessage;

 public:
  ExceptionDataVariable(const std::string& errorMessage) : _errorMessage(errorMessage) {}

  int get_dataType_enum() const override { return DATATYPE::EXCEPTION; }

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  std::string print() override { return _errorMessage; }

  nlohmann::json to_json() const override { return "[Exception]"; }
};