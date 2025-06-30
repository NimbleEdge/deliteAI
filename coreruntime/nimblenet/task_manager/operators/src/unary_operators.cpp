#include "unary_operators.hpp"

std::map<std::string, UnaryOpFuncPtr> UnaryOperators::_unaryOpMap = {
    {"Not", UnaryOperators::inverse_bool},
    {"USub", UnaryOperators::unary_sub},
};

UnaryOpFuncPtr UnaryOperators::get_operator(const std::string& opType) {
  if (_unaryOpMap.find(opType) == _unaryOpMap.end()) {
    THROW("unaryOp=%s not found", opType.c_str());
  }
  return _unaryOpMap[opType];
}
