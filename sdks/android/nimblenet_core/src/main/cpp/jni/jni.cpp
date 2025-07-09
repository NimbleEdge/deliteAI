/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android/log.h>
#include <jni.h>

#include <string>
#include <vector>

#include "../impl/proto_data_variable.hpp"
#include "client.h"
#include "hardware_info_shadow.hpp"
#include "jni_common.h"
#include "logs_upload_scheduler_shadow.hpp"
#include "nimblenet.hpp"
#include "nlohmann/json.hpp"
#include "proto_member_extender_shadow.h"
#include "utils/input_transformers.h"
#include "utils/jni_string.h"
#include "utils/output_transformers.h"

#ifdef GEMINI
#include "../coreruntime/nimblenet/llm_executors/include/gemini_nano_executor.hpp"
#include "file_download_state_transition_shadow.h"
#include "mutable_map_shadow.h"
#include "nimble_net_error_shadow.h"
#include "nimble_net_result_shadow.h"
#include "user_event_data_shadow.h"
#endif  // GEMINI

NimbleNetTensorShadow nimbleNetTensorShadow = nullptr;
NimbleNetResultShadow nimbleNetResultShadow = nullptr;
MutableMapShadow mutableMapShadow = nullptr;
TypeCasterShadow typeCasterShadow = nullptr;
JSONObjectShadow jsonObjectShadow = nullptr;
JSONArrayShadow jsonArrayShadow = nullptr;
NimbleNetErrorShadow nimbleNetErrorShadow = nullptr;
UserEventDataShadow userEventDataShadow = nullptr;

jclass ProtoMemberExtenderShadow::jobjectExtenderClass;
jmethodID ProtoMemberExtenderShadow::getMethodId;
jmethodID ProtoMemberExtenderShadow::getValueByIndexMethodId;
jmethodID ProtoMemberExtenderShadow::getValueByKeyMethodId;
jmethodID ProtoMemberExtenderShadow::setValueByIndexMethodId;
jmethodID ProtoMemberExtenderShadow::setValueByKeyMethodId;
jmethodID ProtoMemberExtenderShadow::getKeysMethodId;
jmethodID ProtoMemberExtenderShadow::containsMethodId;
jmethodID ProtoMemberExtenderShadow::sizeMethodId;
jmethodID ProtoMemberExtenderShadow::arrangeMethodId;
jmethodID ProtoMemberExtenderShadow::printMethodId;
jmethodID ProtoMemberExtenderShadow::getCoreTypeMethodId;
jmethodID ProtoMemberExtenderShadow::popValueByIndexMethodId;
jmethodID ProtoMemberExtenderShadow::popValueByStringMethodId;
jmethodID ProtoMemberExtenderShadow::appendMethodId;
bool ProtoMemberExtenderShadow::isInitialized = false;

jclass FileDownloadStateTransitionShadow::fileDownloadStateTransitionClass;
jmethodID FileDownloadStateTransitionShadow::getCurrentStateMethodId;
jmethodID FileDownloadStateTransitionShadow::getPreviousStateMethodId;
jmethodID FileDownloadStateTransitionShadow::getTimeTakenMethodId;
jmethodID FileDownloadStateTransitionShadow::getCurrentStateReasonCodeMethodId;
jmethodID FileDownloadStateTransitionShadow::getDownloadManagerDownloadIdMethodId;

using namespace std;

// KOTLIN EXTERN FUNCTIONS
#pragma GCC visibility push(default)

void initContext(JNIEnv *env, jobject ctx) {
  env->GetJavaVM(&globalJvm);
  context = env->NewGlobalRef(ctx);
}

extern "C" {
void JNICALL Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_initializeNimbleNet(
    JNIEnv *env, jobject thiz, jobject ctx, jstring jnimble_net_config, jobject assetsJson,
    jstring jnimble_net_dir, jobject nimblenet_result_android) {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
  attachCrashReporter();

  try {
    initContext(env, ctx);
    DependencyContainerShadow::init(env);
    nimbleNetTensorShadow = NimbleNetTensorShadow(env);
    nimbleNetResultShadow = NimbleNetResultShadow(env);
    nimbleNetErrorShadow = NimbleNetErrorShadow(env);
    mutableMapShadow = MutableMapShadow(env);
    typeCasterShadow = TypeCasterShadow(env);
    jsonArrayShadow = JSONArrayShadow(env);
    jsonObjectShadow = JSONObjectShadow(env);
    userEventDataShadow = UserEventDataShadow(env);
    NetworkingShadow::init(env);
    FileDownloadStateTransitionShadow::init(env);
    HardwareInfoShadow::init(env);
    LogsUploadSchedulerShadow::init(env);
#ifdef GEMINI
    geminiNanoHandlerShadow = GeminiNanoHandlerShadow(env);
#endif  // GEMINI

    if (assetsJson != nullptr) {
        OpReturnType assets = convertJSONArrayToOpReturnType(env, assetsJson);
        NimbleNetStatus *loadModulesStatus =
            nimblenet::load_modules(assets, JniString::jstringToStdString(env, jnimble_net_dir));
        // In case of an error while loading modules return without initializing
        if (loadModulesStatus != nullptr) {
            populateNimbleNetResult(env, nimblenet_result_android, loadModulesStatus, nullptr, nullptr);
            checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "initNimbleNet()");
            return;
        }
    }

    NimbleNetStatus *nimbleNetStatus =
        nimblenet::initialize_nimblenet(JniString::jstringToStdString(env, jnimble_net_config),
                                        JniString::jstringToStdString(env, jnimble_net_dir));

    populateNimbleNetResult(env, nimblenet_result_android, nimbleNetStatus, nullptr, nullptr);
  } catch (...) {
    jstring errorMessage = (jstring) "Exception while initializing android classes in JNI";
    populateNimbleNetResult(env, nimblenet_result_android, nullptr, nullptr, errorMessage);
  }

  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "initNimbleNet()");
}
}

extern "C" {
void JNICALL Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_runMethod(
    JNIEnv *env, jobject thiz, jstring jTaskName, jobject kotlinInputMap,
    jobject nimbleNetResultAndroid) {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);

  try {
    attachCrashReporter();
    _threadLocalEnv = env;

    auto nativeInputMap = convertNimbleNetTensorMapToDataVariableMap(env, kotlinInputMap);
    auto nativeOutputMap = std::make_shared<MapDataVariable>();

    auto nimbleNetStatus = nimblenet::run_method(JniString::jstringToStdString(env, jTaskName),
                                                 nativeInputMap, nativeOutputMap);

    auto kotlinOutputMap = convertDataVariableMapToNimbleNetTensorMap(env, *nativeOutputMap);

    populateNimbleNetResult(env, nimbleNetResultAndroid, nimbleNetStatus, kotlinOutputMap, nullptr);
    env->DeleteLocalRef(kotlinOutputMap);
  } catch (const std::exception &e) {
    const char *errorMessage = e.what();
    jstring jErrorMessage = env->NewStringUTF(errorMessage);
    populateNimbleNetResult(env, nimbleNetResultAndroid, nullptr, nullptr, jErrorMessage);
    env->DeleteLocalRef(jErrorMessage);
  }
  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "runMethod()");
}
}

extern "C" JNIEXPORT void JNICALL Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_isReady(
    JNIEnv *env, jobject thiz, jobject nimblenet_result_android) {
  attachCrashReporter();
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
  try {
    auto nimbleNetStatus = nimblenet::is_ready();
    populateNimbleNetResult(env, nimblenet_result_android, nimbleNetStatus, nullptr, nullptr);
  } catch (...) {
    const char *errorMessage = "exception at is_ready";
    jstring jErrorMessage = env->NewStringUTF(errorMessage);
    populateNimbleNetResult(env, nimblenet_result_android, nullptr, nullptr, jErrorMessage);
  }
  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "isReady()");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_sendEvents(JNIEnv *env, jobject thiz,
                                                                     jstring jparams,
                                                                     jstring jhomeDir) {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
  try {
    initContext(env, context);
    NetworkingShadow::init(env);
    FileDownloadStateTransitionShadow::init(env);

    return nimblenet::send_events(JniString::jstringToStdString(env, jparams),
                                  JniString::jstringToStdString(env, jhomeDir));
  } catch (...) {
    return false;
  }
  // TODO: Control will never reach here
  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "sendEvents()");
}

extern "C" JNIEXPORT void JNICALL
Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_restartSession(JNIEnv *env, jobject thiz,
                                                                         jstring jsession_id) {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
  attachCrashReporter();

  try {
    nimblenet::update_session(JniString::jstringToStdString(env, jsession_id));
  } catch (...) {
    LOGE("Exception at restartSession()");
  }
  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "restartSession()");
}

extern "C" {
void JNICALL Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_addEvent(
    JNIEnv *env, jobject deliteAiController, jstring serializedEventMap, jstring tableName,
    jobject nimblenet_result_android) {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
  const char *events = env->GetStringUTFChars(serializedEventMap, nullptr);
  const char *name = env->GetStringUTFChars(tableName, nullptr);

  try {
    CUserEventsData cUserEventsData;
    auto userEventDataKotlin = nimbleNetResultShadow.getData(env, nimblenet_result_android);

    auto nimbleNetStatus = nimblenet::add_event(events, name, &cUserEventsData);

    if (nimbleNetStatus == nullptr) {
      auto eventTypeJString = JniString::cStringToJstring(env, cUserEventsData.eventType);
      auto eventJsonJString = JniString::cStringToJstring(env, cUserEventsData.eventJsonString);
      userEventDataShadow.setEventJsonString(env, userEventDataKotlin, eventJsonJString);
      userEventDataShadow.setEventType(env, userEventDataKotlin, eventTypeJString);
      populateNimbleNetResult(env, nimblenet_result_android, nimbleNetStatus, userEventDataKotlin,
                              nullptr);
    } else {
      populateNimbleNetResult(env, nimblenet_result_android, nimbleNetStatus, nullptr, nullptr);
    }

    deallocate_c_userevents_data(&cUserEventsData);
  } catch (...) {
    const char *errorMessage = "Exception at addEvent()";
    jstring jErrorMessage = env->NewStringUTF(errorMessage);
    populateNimbleNetResult(env, nimblenet_result_android, nullptr, nullptr, jErrorMessage);
  }

  env->ReleaseStringUTFChars(serializedEventMap, events);
  env->ReleaseStringUTFChars(tableName, name);
  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "addEvent()");
}
}

extern "C" JNIEXPORT void JNICALL
Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_addEventProto(
    JNIEnv *env, jobject thiz, jobject proto_event, jstring event_type,
    jobject nimblenet_result_android) {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);

  try {
    _threadLocalEnv = env;
    CUserEventsData cUserEventsData;
    auto userEventDataKotlin = nimbleNetResultShadow.getData(env, nimblenet_result_android);
    auto eventType = JniString::jstringToStdString(env, event_type);
    auto event = std::make_shared<ProtoDataVariable>(env, proto_event);

    auto nimbleNetStatus = nimblenet::add_event(event, eventType, &cUserEventsData);

    if (nimbleNetStatus == nullptr) {
      auto eventTypeJString = JniString::cStringToJstring(env, cUserEventsData.eventType);
      auto eventJsonJString = JniString::cStringToJstring(env, cUserEventsData.eventJsonString);

      userEventDataShadow.setEventJsonString(env, userEventDataKotlin, eventJsonJString);
      userEventDataShadow.setEventType(env, userEventDataKotlin, eventTypeJString);
      populateNimbleNetResult(env, nimblenet_result_android, nimbleNetStatus, userEventDataKotlin,
                              nullptr);
    } else {
      populateNimbleNetResult(env, nimblenet_result_android, nimbleNetStatus, nullptr, nullptr);
    }

    deallocate_c_userevents_data(&cUserEventsData);
  } catch (...) {
    const char *errorMessage = "Exception at addEventProto()";
    jstring jErrorMessage = env->NewStringUTF(errorMessage);
    populateNimbleNetResult(env, nimblenet_result_android, nullptr, nullptr, jErrorMessage);
  }

  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "addEventProto()");
}

extern "C" {
void JNICALL Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_writeMetric(
    JNIEnv *env, jobject deliteAiController, jstring jmetricType, jstring jmetric) {
  const char *metric = env->GetStringUTFChars(jmetric, nullptr);
  const char *metricType = env->GetStringUTFChars(jmetricType, nullptr);
  nimblenet::write_metric(metricType, metric);
  env->ReleaseStringUTFChars(jmetric, metric);
  env->ReleaseStringUTFChars(jmetricType, metricType);
}
}

extern "C" {
void JNICALL Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_writeRunMethodMetric(
    JNIEnv *env, jobject thiz, jstring jid, jlong jtotal_time_in_usecs) {
  const char *id = env->GetStringUTFChars(jid, nullptr);
  nimblenet::write_run_method_metric(id, jtotal_time_in_usecs);
  env->ReleaseStringUTFChars(jid, id);
}
}

extern "C" {
void JNICALL
Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_networkConnectionEstablishedCallback(
    JNIEnv *env, jobject deliteAiController) {
  nimblenet::internet_switched_on();
}
}

#ifdef GEMINI
extern "C" JNIEXPORT void JNICALL
Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_pushGeminiResponseToQueue(
    JNIEnv *env, jobject thiz, jstring text) noexcept {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
  attachCrashReporter();

  GeminiNanoExecutor::push_to_queue(JniString::jstringToStdString(env, text));

  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "pushGeminiResponseToQueue()");
}

extern "C" JNIEXPORT void JNICALL
Java_dev_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_closeGeminiResponseQueue(
    JNIEnv *env, jobject thiz) noexcept {
  auto initialLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
  attachCrashReporter();

  GeminiNanoExecutor::mark_end_of_stream();

  checkForUndeletedLocalReferences(env, initialLocalRefsAllowed, "closeGeminiResponseQueue()");
}
#endif  // GEMINI

#pragma GCC visibility pop
