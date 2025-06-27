#include "single_variable.hpp"

OpReturnType SingleVariable<std::string>::call_function(int memberFuncIndex,
                                                        const std::vector<OpReturnType>& arguments,
                                                        CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::STRING_UPPER: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::STRING_UPPER);
      std::string result = val;
      std::transform(result.begin(), result.end(), result.begin(), ::toupper);
      return OpReturnType(new SingleVariable<std::string>(result));
    }
    case MemberFuncType::STRING_LOWER: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::STRING_LOWER);
      std::string result = val;
      std::transform(result.begin(), result.end(), result.begin(), ::tolower);
      return OpReturnType(new SingleVariable<std::string>(result));
    }
    case MemberFuncType::STRING_STRIP: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::STRING_STRIP);
      std::string result = val;
      result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
                     return !std::isspace(ch);
                   }));
      // Base will point to the next element currently being pointed to by reverse iterator.
      result.erase(std::find_if(result.rbegin(), result.rend(),
                                [](unsigned char ch) { return !std::isspace(ch); })
                       .base(),
                   result.end());
      return OpReturnType(new SingleVariable<std::string>(result));
    }
    case MemberFuncType::STRING_JOIN: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::STRING_JOIN);
      // Tuple, list and tensor expected as arguments
      if (arguments[0]->get_containerType() != CONTAINERTYPE::LIST &&
          arguments[0]->get_containerType() != CONTAINERTYPE::TUPLE &&
          arguments[0]->get_containerType() != CONTAINERTYPE::VECTOR) {
        THROW("join expects argument of type tuple/list/tensor, provided : %s",
              arguments[0]->get_containerType_string());
      }
      std::string result;
      // If tensor then shape should be 1D and dtype of string
      if (arguments[0]->get_containerType() == CONTAINERTYPE::VECTOR) {
        if (arguments[0]->get_dataType_enum() != DATATYPE::STRING) {
          THROW("join when given argument of type vector it should contain strings, provided : %s",
                arguments[0]->get_dataType_enum());
        }
        if (arguments[0]->get_shape().size() != 1) {
          THROW(
              "join when given argument of type vector it should be 1 dimensional, provided has "
              "%d dimensions",
              arguments[0]->get_shape().size());
        }
        if (arguments[0]->get_numElements() == 0) {
          return OpReturnType(new SingleVariable<std::string>(""));
        }
        // Get all the strings
        int numElements = arguments[0]->get_numElements();
        for (int i = 0; i < numElements - 1; i++) {
          result += arguments[0]->get_int_subscript(i)->get_string() + val;
        }
        result += arguments[0]->get_int_subscript(numElements - 1)->get_string();
      } else {
        // If list/tuple then all members should be Single Variable and of datatype string
        int numElements = arguments[0]->get_size();
        if (numElements == 0) {
          return OpReturnType(new SingleVariable<std::string>(""));
        }
        for (int i = 0; i < numElements; i++) {
          if (arguments[0]->get_int_subscript(i)->get_containerType() != CONTAINERTYPE::SINGLE) {
            THROW(
                "join when given argument of type list/tuple expects all elements to be of type "
                "string, provided argument at index %d is %s",
                i, arguments[0]->get_int_subscript(i)->get_containerType_string());
          }
          if (arguments[0]->get_int_subscript(i)->get_dataType_enum() != DATATYPE::STRING) {
            THROW(
                "join when given argument of type list/tuple expects all elements to be of type "
                "string, provided argument at index %d is of type %s",
                i,
                util::get_string_from_enum(
                    arguments[0]->get_int_subscript(i)->get_dataType_enum()));
          }
          result += arguments[0]->get_int_subscript(i)->get_string() + val;
        }
        result.erase(result.size() - val.size());
      }
      return OpReturnType(new SingleVariable<std::string>(result));
    }
    case MemberFuncType::UNICODE: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::UNICODE);
      std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      return OpReturnType(new SingleVariable<std::wstring>(converter.from_bytes(val)));
    }
    default: {
      return DataVariable::call_function(memberFuncIndex, arguments, stack);
    }
  }
}

void SingleVariable<std::string>::build_char_to_byte_map() {
  // Clear any existing mapping
  char_to_byte_map.clear();

  // Count characters and build mapping at the same time
  cached_char_count = 0;

  // Pre-allocate with a reasonable capacity (most strings are short)
  char_to_byte_map.reserve(val.size() > 0 ? val.size() / 2 : 8);

  // Build the mapping
  const char* s = val.c_str();
  int byte_pos = 0;

  // Always start with position 0
  char_to_byte_map.push_back(0);

  while (*s) {
    if (!util::utf8::is_continuation_byte(*s)) {
      // This is the start of a new character
      cached_char_count++;
    }
    s++;
    byte_pos++;

    // Record the byte position for the next character
    if (!util::utf8::is_continuation_byte(*s) && *s) {
      char_to_byte_map.push_back(byte_pos);
    }
  }
}

SingleVariable<std::string>::SingleVariable(const std::string& v) {
  val = v;
  build_char_to_byte_map();
}

SingleVariable<std::string>::SingleVariable(const char* vPtr) {
  val = std::string(vPtr);
  build_char_to_byte_map();
}

char** SingleVariable<std::string>::get_string_ptr() {
  strPtr.clear();
  strPtr.push_back((char*)val.c_str());
  return strPtr.data();
}

int SingleVariable<std::string>::char_idx_to_byte_pos(int char_idx) const {
  int char_idx_ = char_idx;
  // Handle negative indices
  if (char_idx_ < 0) {
    char_idx_ += cached_char_count;
    if (char_idx_ < 0) {
      THROW("string index out of range: %d for string with %d characters", char_idx,
            cached_char_count);
    }
  }

  // Bounds checking
  if (char_idx_ >= static_cast<int>(char_to_byte_map.size())) {
    THROW("string index out of range: %d for string with %d characters", char_idx,
          cached_char_count);
  }
  return char_to_byte_map[char_idx_];
}

OpReturnType SingleVariable<std::string>::get_int_subscript(int argument) {
  // Find the byte position of the character at index 'argument'
  int byte_pos = char_idx_to_byte_pos(argument);
  // Extract the UTF-8 character
  std::string result = util::utf8::extract_char(val, byte_pos);
  return OpReturnType(new SingleVariable<std::string>(std::move(result)));
}

OpReturnType SingleVariable<std::string>::get_subscript(const OpReturnType& subscriptVal) {
  // If the subscript is a SliceVariable, handle slicing
  if (subscriptVal->get_containerType() == CONTAINERTYPE::SLICE) {
    return get_slice_subscript(subscriptVal);
  }
  // Otherwise, just delegate to the integer index handler
  return get_int_subscript(subscriptVal->get_int32());
}

OpReturnType SingleVariable<std::string>::get_slice_subscript(const OpReturnType& sliceObj) {
  // Cast to SliceVariable to use direct member access
  const ListSliceVariable* slice = static_cast<const ListSliceVariable*>(sliceObj.get());

  // Get character count (not byte count) - now already cached
  const int char_count = cached_char_count;

  // Extract slice parameters (start, stop, step)
  int start_char = slice->get_start(char_count);
  int stop_char = slice->get_stop(char_count);
  int step = slice->get_step();

  // For other step values, we need to iterate character by character
  std::string result = "";
  if (step > 0) {
    for (int i = start_char; i < stop_char; i += step) {
      int byte_pos = char_idx_to_byte_pos(i);
      result += util::utf8::extract_char(val, byte_pos);
    }
  } else {
    for (int i = start_char; i > stop_char; i += step) {
      int byte_pos = char_idx_to_byte_pos(i);
      result += util::utf8::extract_char(val, byte_pos);
    }
  }

  // Return a new string variable with the sliced string
  return OpReturnType(new SingleVariable<std::string>(result));
}

nlohmann::json SingleVariable<std::wstring>::to_json() const {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_converter;
  return utf8_converter.to_bytes(_val);
}

OpReturnType SingleVariable<std::wstring>::get_int_subscript(int argument) {
  std::wstring single_char(1, _val[argument]);
  return OpReturnType(new SingleVariable<std::wstring>(single_char));
}
