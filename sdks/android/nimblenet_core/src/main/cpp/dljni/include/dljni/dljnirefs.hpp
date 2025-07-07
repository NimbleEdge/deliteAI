/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <memory>
#include <type_traits>
#include <utility>

/**
 * @brief Creates a <code>DlJobjectLocalRef\<T\></code>.
 *
 * @param env JNIEnv to be used for jobject local reference deletion.
 * @param obj_local_ref jobject local reference to be managed via std::unique_ptr\<\>.
 * @return DlJobjectLocalRef\<T\>
 */
template <typename T,
          typename = std::enable_if_t<std::is_base_of_v<std::remove_pointer_t<jobject>, T>>>
auto make_jobjectlocalref(JNIEnv* env, T* obj_local_ref) {
  auto deleter = [env](T* obj_local_ref) { env->DeleteLocalRef(obj_local_ref); };
  return std::unique_ptr<T, decltype(deleter)>(obj_local_ref, std::move(deleter));
}

/**
 * @brief A <code>std::unique_ptr\<\></code> to manage a JNI <code>jobject</code> local reference.
 *
 * On destruction, it calls <code>env->DeleteLocalRef()</code> on the managed pointer.<br/>
 * For that, the deleter captures the corresponding <code>JNIEnv* env</code>.<br/>
 * As a result, <code>sizeof(DlJobjectLocalRef\<T\>) == 2 * sizeof(void*)</code>.
 */
template <typename T>
using DlJobjectLocalRef =
    decltype(make_jobjectlocalref(std::declval<JNIEnv*>(), std::declval<T*>()));

/**
 * @brief Creates a <code>DlJobjectGlobalRef\<T\></code>.
 *
 * @param env JNIEnv to be used for jobject global reference creation and deletion.
 * @param obj_local_ref jobject local reference to be converted to a global reference and then
 *                      managed via std::unique_ptr\<\>.
 * @return DlJobjectGlobalRef\<T\>
 */
template <typename T,
          typename = std::enable_if_t<std::is_base_of_v<std::remove_pointer_t<jobject>, T>>>
auto make_jobjectglobalref(JNIEnv* env, T* obj_local_ref) {
  auto deleter = [env](T* obj_global_ref) { env->DeleteGlobalRef(obj_global_ref); };
  return std::unique_ptr<T, decltype(deleter)>(dynamic_cast<T*>(env->NewGlobalRef(obj_local_ref)),
                                               std::move(deleter));
}

/**
 * @brief A <code>std::unique_ptr\<\></code> to manage a JNI <code>jobject</code> global reference.
 *
 * On destruction, it calls <code>env->DeleteGlobalRef()</code> on the managed pointer.<br/>
 * For that, the deleter captures the corresponding <code>JNIEnv* env</code>.<br/>
 * As a result, <code>sizeof(DlJobjectGlobalRef\<T\>) == 2 * sizeof(void*)</code>.
 */
template <typename T>
using DlJobjectGlobalRef =
    decltype(make_jobjectglobalref(std::declval<JNIEnv*>(), std::declval<T*>()));
