/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ne::AtomicPtr : wrapper of std::shared_ptr that is to be loaded from and stored to atomically
 * This will only allow atomically doing these things and removes chance of accidentally directly
 * accessing the shared_ptr
 */

#pragma once

#include <atomic>
#include <memory>

namespace ne {

/**
 * @brief A wrapper class for std::shared_ptr that provides atomic operations
 *
 * @tparam T The type of object to be managed by the shared pointer
 *
 * This class provides atomic load and store operations for a shared pointer,
 * while allowing the pointer to be null. It is designed to be used as a base
 * class for AtomicPtr which enforces non-null semantics.
 */
template <typename T>
class NullableAtomicPtr {
  std::shared_ptr<T> _sharedPtr;

 protected:
  /**
   * @brief Internal constructor type for default construction
   *
   * It is only used by AtomicPtr child class to do default construction
   */
  enum class ConstructorType { Default };

  /**
   * @brief Protected constructor that creates a default-constructed object
   *
   * @param type Constructor type indicator
   */
  NullableAtomicPtr(ConstructorType) : _sharedPtr(std::make_shared<T>()) {}

 public:
  /**
   *@brief Constructs a NullableAtomicPtr with the given arguments
   *
   *@tparam Args Variadic template parameter pack for constructor arguments
   *@param args Arguments to forward to T's constructor
   */
  template <typename... Args>
  NullableAtomicPtr(Args&&... args)
      : _sharedPtr(std::make_shared<T>(std::forward<Args>(args)...)) {}

  /**
   * @brief Default constructor that initializes with a null pointer
   */
  NullableAtomicPtr() = default;

  /**
   * @brief Atomically loads the shared pointer
   *
   * @return std::shared_ptr<T> The atomically loaded shared pointer
   */
  auto load() const noexcept { return std::atomic_load(&_sharedPtr); }

  /**
   * @brief Atomically stores a new shared pointer
   *
   * @param sharedPtr_ The new shared pointer to store
   */
  void store(std::shared_ptr<T> sharedPtr_) noexcept { std::atomic_store(&_sharedPtr, sharedPtr_); }

  // Delete copy operations to prevent accidental copying
  NullableAtomicPtr(const NullableAtomicPtr&) = delete;
  NullableAtomicPtr& operator=(const NullableAtomicPtr&) = delete;
};

/**
 * @brief A wrapper class for std::shared_ptr that provides atomic operations and non-null guarantees
 *
 * @tparam T The type of object to be managed by the shared pointer
 *
 * This class extends NullableAtomicPtr to ensure the shared pointer is never null.
 * It provides the same atomic operations as NullableAtomicPtr but with the additional
 * guarantee that the pointer will always be valid.
 */
template <typename T>
class AtomicPtr : public NullableAtomicPtr<T> {
 public:
  /**
   * @brief Constructs an AtomicPtr with the given arguments
   *
   * @tparam Args Variadic template parameter pack for constructor arguments
   * @param args Arguments to forward to T's constructor
   */
  template <typename... Args>
  AtomicPtr(Args&&... args) : NullableAtomicPtr<T>(std::forward<Args>(args)...) {}

  /**
   *@brief Default constructor that creates a default-constructed object
   *
   * Unlike NullableAtomicPtr, this ensures the pointer is never null by
   * default-constructing the managed object.
   */
  AtomicPtr() : NullableAtomicPtr<T>(NullableAtomicPtr<T>::ConstructorType::Default) {}
};

}  // namespace ne
