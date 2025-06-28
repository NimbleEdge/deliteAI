/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include "../utils/input_transformers.h"
#include "../utils/jni_string.h"
#include "../utils/output_transformers.h"
#include "data_variable.hpp"
#include "list_data_variable.hpp"
#include "proto_member_extender_shadow.h"

class ProtoDataVariable : public DataVariable {
    std::shared_ptr<ProtoMemberExtenderShadow> _protoMemberExtenderShadow;
    
    int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

    int get_dataType_enum() const override { return DATATYPE::FE_OBJ; }

    bool get_bool() override { return true; }

    int get_size() override { return _protoMemberExtenderShadow->size(_threadLocalEnv); }

public:
    ProtoDataVariable(JNIEnv *env, jobject obj)
            : _protoMemberExtenderShadow(std::make_shared<ProtoMemberExtenderShadow>(env, obj)) {

        static bool initializeExtenderOnce = ProtoMemberExtenderShadow::init(env);
        if (!ProtoMemberExtenderShadow::is_initialized()) {
            THROW("%s", "Kotlin Proto Support classes are missing");
        }
    }
    auto get_proto_shadow() const {
        return _protoMemberExtenderShadow;
    }
    OpReturnType get_string_subscript(const std::string &key) override {
        auto cStr = key.c_str();
        auto jkey = JniString::cStringToJstring(_threadLocalEnv, cStr);
        auto jobj = _protoMemberExtenderShadow->getValue(_threadLocalEnv, jkey);
        auto res = getOpReturnType(jobj);
        _threadLocalEnv->DeleteLocalRef(jkey);
        _threadLocalEnv->DeleteLocalRef(jobj);
        return res;
    }

    OpReturnType get_int_subscript(int index) override {
        auto jobj = _protoMemberExtenderShadow->getValue(_threadLocalEnv, index);
        auto res = getOpReturnType(jobj);
        _threadLocalEnv->DeleteLocalRef(jobj);
        return res;
    }

    void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) override {
        // currently only support object setting.
        auto nimblenetTensor = convertDataVariableToNimbleNetTensor(_threadLocalEnv,d);
        auto javaObject = nimbleNetTensorShadow.getData(_threadLocalEnv,nimblenetTensor);
        if(subscriptVal->is_string()){
            // convert d to a jobject
            auto key = subscriptVal->get_string();
            auto jkey = JniString::cStringToJstring(_threadLocalEnv, key.c_str());
            _protoMemberExtenderShadow->setValue(_threadLocalEnv,jkey, javaObject);
            _threadLocalEnv->DeleteLocalRef(jkey);
        } else {
            jint index = subscriptVal->get_int32();
            _protoMemberExtenderShadow->setValue(_threadLocalEnv,index, javaObject);
        }

        _threadLocalEnv->DeleteLocalRef(nimblenetTensor);
        _threadLocalEnv->DeleteLocalRef(javaObject);
    }

    bool in(const OpReturnType &elem) override {
        auto jkey = JniString::cStringToJstring(_threadLocalEnv, elem->get_string().c_str());
        auto res = _protoMemberExtenderShadow->contains(_threadLocalEnv, jkey);
        _threadLocalEnv->DeleteLocalRef(jkey);
        return res;
    }

    OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType> &arguments,
                               CallStack &stack) override {
        switch (memberFuncIndex) {
            case MemberFuncType::KEYS: {
                THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::KEYS);

                OpReturnType list = OpReturnType(new ListDataVariable());
                auto keys = _protoMemberExtenderShadow->getKeys(_threadLocalEnv);
                for (auto it: keys) {
                    list->append(OpReturnType(new SingleVariable<std::string>(it)));
                }
                return list;
            }
            case MemberFuncType::POP: {
                THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::POP);
                OpReturnType keyOrIndex = arguments[0];
                if (keyOrIndex->get_containerType() != CONTAINERTYPE::SINGLE) {
                    THROW("%s",
                          "pop expects either a string in case of removing an element from map or index in "
                          "case of a list.");
                }
                if (keyOrIndex->is_string()) {
                    auto key = keyOrIndex->get_string();
                    auto jkey = JniString::cStringToJstring(_threadLocalEnv, key.c_str());
                    auto jobj = _protoMemberExtenderShadow->pop(_threadLocalEnv, jkey);
                    auto res = getOpReturnType(jobj);
                    _threadLocalEnv->DeleteLocalRef(jkey);
                    _threadLocalEnv->DeleteLocalRef(jobj);
                    return res;
                } else {
                    jint index = keyOrIndex->get_int32();
                    auto jobj = _protoMemberExtenderShadow->pop(_threadLocalEnv, index);
                    auto res = getOpReturnType(jobj);
                    _threadLocalEnv->DeleteLocalRef(jobj);
                    return res;
                }
            }
            default: {
                return DataVariable::call_function(memberFuncIndex, arguments, stack);
            }
        }

        THROW("%s not implemented for proto data variable.",
              DataVariable::get_member_func_string(memberFuncIndex));
    }

    OpReturnType arrange(const OpReturnType argument) override {

        if (argument->get_containerType() != CONTAINERTYPE::VECTOR &&
            argument->get_containerType() != CONTAINERTYPE::LIST) {
            THROW("Argument of arrange should be a tensor/list, provided %s",
                  argument->get_containerType_string());
        }
        if (argument->get_containerType() == CONTAINERTYPE::VECTOR &&
            argument->get_shape().size() != 1) {
            THROW("Argument of arrange if tensor, should be of dimension 1, provided %d dimensions",
                  argument->get_shape().size());
        }
        auto shape = argument->get_shape();

        if (shape.size() != 1) {
            THROW("arrange expects tensor to be of 1 dimension. Given %d dimensions.",
                  shape.size());
        }
        int size = argument->get_size();
        std::vector<int64_t> indexedValues;

        for (int i = 0; i < size; i++) {
            OpReturnType index = argument->get_int_subscript(i);
            if (!index->is_integer()) {
                THROW("Element at index=%d should be of type int, provided %s", i,
                      util::get_string_from_enum(index->get_dataType_enum()));
            }

            int indexValue = index->get_int32();
            if (indexValue < 0 || indexValue >= shape[0]) {
                THROW("Tried to access %d index of the tensor.", indexValue);
            }

            indexedValues.push_back(indexValue);
        }


        jintArray kotlinIndexArray = _threadLocalEnv->NewIntArray(indexedValues.size());
        std::vector<jint> temp(indexedValues.begin(), indexedValues.end());
        _threadLocalEnv->SetIntArrayRegion(kotlinIndexArray, 0, temp.size(), temp.data());

        auto arrangedArray = _protoMemberExtenderShadow->arrange(_threadLocalEnv, kotlinIndexArray);
        auto res = OpReturnType(new ProtoDataVariable(_threadLocalEnv, arrangedArray));

        _threadLocalEnv->DeleteLocalRef(kotlinIndexArray);
        _threadLocalEnv->DeleteLocalRef(arrangedArray);
        return res;
    }

    std::string print() override {
        return _protoMemberExtenderShadow->print(_threadLocalEnv);
    }

    nlohmann::json to_json() const override {
        THROW("%s","Proto to json not implemented");
    }

    std::string to_json_str() const override {
        return _protoMemberExtenderShadow->print(_threadLocalEnv);
    }

    OpReturnType getOpReturnType(jobject obj) {
        auto childProtoMember = ProtoMemberExtenderShadow(_threadLocalEnv, obj);
        auto dataType = childProtoMember.getCoreType(_threadLocalEnv);

        if(dataType != DATATYPE::FE_OBJ) {
            obj = childProtoMember.get(_threadLocalEnv);
        }

        auto res = convertSingularKotlinDataToOpReturnType(_threadLocalEnv, obj, dataType);

        if(dataType != DATATYPE::FE_OBJ) {
            _threadLocalEnv->DeleteLocalRef(obj);
        }

        return res;
    }

    OpReturnType append(OpReturnType d) override {
        auto nimblenetTensor = convertDataVariableToNimbleNetTensor(_threadLocalEnv,d);
        auto javaObject = nimbleNetTensorShadow.getData(_threadLocalEnv,nimblenetTensor);
        _protoMemberExtenderShadow->append(_threadLocalEnv, javaObject);
        _threadLocalEnv->DeleteLocalRef(nimblenetTensor);
        _threadLocalEnv->DeleteLocalRef(javaObject);
        return shared_from_this();
    }
};
