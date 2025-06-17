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

#include <atomic>
#include <memory>

namespace ne {

template <typename T>
class NullableAtomicPtr {
  std::shared_ptr<T> _sharedPtr;

 protected:
  // This is a constructor that default constructs the shared_ptr. It is only used by AtomicPtr
  // child class to do default construction
  enum class ConstructorType { Default };

  NullableAtomicPtr(ConstructorType) : _sharedPtr(std::make_shared<T>()) {}

 public:
  template <typename... Args>
  NullableAtomicPtr(Args&&... args)
      : _sharedPtr(std::make_shared<T>(std::forward<Args>(args)...)) {}

  // _sharedPtr will be nullptr in default construction
  NullableAtomicPtr() = default;

  auto load() const noexcept { return std::atomic_load(&_sharedPtr); }

  void store(std::shared_ptr<T> sharedPtr_) noexcept { std::atomic_store(&_sharedPtr, sharedPtr_); }

  NullableAtomicPtr(const NullableAtomicPtr&) = delete;
  NullableAtomicPtr& operator=(const NullableAtomicPtr&) = delete;
};

template <typename T>
class AtomicPtr : public NullableAtomicPtr<T> {
 public:
  template <typename... Args>
  AtomicPtr(Args&&... args) : NullableAtomicPtr<T>(std::forward<Args>(args)...) {}

  AtomicPtr() : NullableAtomicPtr<T>(NullableAtomicPtr<T>::ConstructorType::Default) {}
};

}  // namespace ne
