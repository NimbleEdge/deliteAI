#include "nimble_net_internal_data_variable.hpp"

OpReturnType NimbleNetInternalDataVariable::create_retriever(
    const std::vector<OpReturnType>& arguments, CallStack& stack) {
#ifdef GENAI
  return std::make_shared<RetrieverDataVariable>(_commandCenter, arguments);
#else   // GENAI
  THROW("%s", "Add GENAI flag to build Retriever");
#endif  // GENAI
}
