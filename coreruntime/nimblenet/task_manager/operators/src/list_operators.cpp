/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "binary_operators.hpp"
#include "data_variable.hpp"
#include "list_data_variable.hpp"

// List concatenation (binary add)
OpReturnType ListBinOp::add(OpReturnType val1, OpReturnType val2) const {
  // Ensure the operands are lists
  if (val1->get_containerType() != CONTAINERTYPE::LIST ||
      val2->get_containerType() != CONTAINERTYPE::LIST) {
    THROW("Cannot concatenate list with non-list type: (left)%s(%s) and (right)%s(%s)",
          val1->get_containerType_string(), util::get_string_from_enum(val1->get_dataType_enum()),
          val2->get_containerType_string(), util::get_string_from_enum(val2->get_dataType_enum()));
  }

  const int val1Size = val1->get_size();
  const int val2Size = val2->get_size();

  // Both lists have elements - do the concatenation
  std::vector<OpReturnType> newMembers;
  newMembers.reserve(val1Size + val2Size);

  // Copy elements from first list
  for (int i = 0; i < val1Size; i++) {
    newMembers.push_back(val1->get_int_subscript(i));
  }

  // Copy elements from the second list
  for (int i = 0; i < val2Size; i++) {
    newMembers.push_back(val2->get_int_subscript(i));
  }

  // Return a new list with the combined elements
  return OpReturnType(new ListDataVariable(std::move(newMembers)));
}

// List repetition (binary mult)
OpReturnType ListBinOp::mult(OpReturnType val1, OpReturnType val2) const {
  // Ensure either val1 is a list and val2 is an integer, or vice versa
  if (val1->get_containerType() == CONTAINERTYPE::LIST && val2->is_integer()) {
    // This is the standard case, list * int
  } else if (val2->get_containerType() == CONTAINERTYPE::LIST && val1->is_integer()) {
    // Handle the reversed case: int * list
    std::swap(val1, val2);
  } else {
    // Neither case is valid
    THROW("List repetition requires a list and integer count, got: (left)%s(%s) and (right)%s(%s)",
          val1->get_containerType_string(), util::get_string_from_enum(val1->get_dataType_enum()),
          val2->get_containerType_string(), util::get_string_from_enum(val2->get_dataType_enum()));
  }

  int count = val2->get_int32();
  const int val1Size = val1->get_size();

  // Optimization: Handle special cases efficiently
  if (count <= 0) {
    // Return an empty list for non-positive repetition count
    return OpReturnType(new ListDataVariable());
  }

  if (count == 1) {
    // If count is 1, just return a copy of this list
    std::vector<OpReturnType> newMembers;
    newMembers.reserve(val1Size);

    for (int i = 0; i < val1Size; i++) {
      newMembers.push_back(val1->get_int_subscript(i));
    }

    return OpReturnType(new ListDataVariable(std::move(newMembers)));
  }

  if (val1Size == 0) {
    // If this list is empty, multiplying by any number is still empty
    return OpReturnType(new ListDataVariable());
  }

  // Regular case: Create a new list with repeated elements
  const size_t finalSize = val1Size * count;
  std::vector<OpReturnType> newMembers;
  newMembers.reserve(finalSize);

  // Optimization: For large repetitions, use a more efficient algorithm
  if (count > 10 && val1Size > 1) {
    // First add the original list
    for (int i = 0; i < val1Size; i++) {
      newMembers.push_back(val1->get_int_subscript(i));
    }

    // Double the list repeatedly (exponential growth)
    size_t currentSize = val1Size;
    while (currentSize * 2 <= finalSize) {
      size_t oldSize = currentSize;
      for (size_t i = 0; i < oldSize; i++) {
        newMembers.push_back(newMembers[i]);
      }
      currentSize *= 2;
    }

    // Add any remaining elements needed
    for (size_t i = currentSize; i < finalSize; i++) {
      newMembers.push_back(val1->get_int_subscript(i % val1Size));
    }
  } else {
    // For small repetitions, just use the simple approach
    for (int i = 0; i < count; i++) {
      for (int j = 0; j < val1Size; j++) {
        newMembers.push_back(val1->get_int_subscript(j));
      }
    }
  }
  // Return a new list with the repeated elements
  return OpReturnType(new ListDataVariable(std::move(newMembers)));
}
