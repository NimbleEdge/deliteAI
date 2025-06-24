/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>

namespace ne {

template <typename T, typename U>
struct is_not_same : std::true_type {};

template <typename T>
struct is_not_same<T, T> : std::false_type {};

template <typename T, typename U>
inline constexpr bool is_not_same_v = is_not_same<T, U>::value;

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
struct not_type : std::false_type {};

template <>
struct not_type<std::false_type> : std::true_type {};

template <typename...>
struct is_one_of {
  static constexpr bool value = false;
};

template <typename F, typename S, typename... T>
struct is_one_of<F, S, T...> {
  static constexpr bool value = std::is_same<F, S>::value || is_one_of<F, T...>::value;
};

template <typename... Ts>
inline constexpr bool is_one_of_v = is_one_of<Ts...>::value;

template <typename LHS, typename RHS, typename = void>
struct is_addable : std::false_type {};

template <typename LHS, typename RHS>
struct is_addable<LHS, RHS, std::void_t<decltype(std::declval<LHS>() + std::declval<RHS>())>>
    : std::true_type {};

template <typename T, typename U>
using is_addable_t = typename is_addable<T, U>::type;

template <class...>
constexpr std::false_type always_false{};

}  // namespace ne