/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "retriever.hpp"

#include "asset_load_job.hpp"
#include "list_data_variable.hpp"
#include "tensor_data_variable.hpp"
#include "tuple_data_variable.hpp"

OpReturnType RetrieverDataVariable::topk(const std::vector<OpReturnType>& arguments,
                                         CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::TOPK);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::TOPK);
  int k = arguments[1]->get_int32();
  std::vector<OpReturnType> embeddingModelArgs;
  embeddingModelArgs.push_back(OpReturnType(new StringTensorVariable({arguments[0]}, 1)));
  auto embedding =
      _embeddingModel->call_function(MemberFuncType::RUNMODEL, embeddingModelArgs, stack);

  if (!embedding->get_bool()) {
    THROW("%s", "embedding could not be created for query");
  }
  std::vector<OpReturnType> embeddingStoreModelArgs;
  embeddingStoreModelArgs.push_back(embedding->get_int_subscript(0));
  auto output =
      _embeddingStoreModel->call_function(MemberFuncType::RUNMODEL, embeddingStoreModelArgs, stack);
  if (!output->get_bool()) {
    THROW("%s", "Ranks could not be fetched from embeddingStore");
  }
  auto scores = output->get_int_subscript(0);
  auto indices = output->get_int_subscript(1);
  int total = indices->get_size();
  OpReturnType documents = OpReturnType(new ListDataVariable(std::vector<OpReturnType>()));
  OpReturnType docScores = OpReturnType(new ListDataVariable(std::vector<OpReturnType>()));
  for (int i = 0; i < k && i < total; i++) {
    int index = indices->get_int_subscript(i)->get_int32();
    auto val = _documentStore->get_int_subscript(index);
    documents->append(val);
    auto score = scores->get_int_subscript(i);
    docScores->append(score);
  }
  return OpReturnType(new TupleDataVariable({docScores, documents}));
}

OpReturnType RetrieverDataVariable::call_function(int memberFuncIndex,
                                                  const std::vector<OpReturnType>& arguments,
                                                  CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::TOPK:
      return topk(arguments, stack);
  }
  THROW("%s not implemented for Retriever", DataVariable::get_member_func_string(memberFuncIndex));
}

RetrieverDataVariable::RetrieverDataVariable(CommandCenter* commandCenter_,
                                             const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 3, MemberFuncType::RETRIEVER);
  _commandCenter = commandCenter_;
  _embeddingModel = arguments[0];
  _embeddingStoreModel = arguments[1];
  _documentStore = arguments[2];
}
