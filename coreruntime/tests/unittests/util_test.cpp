/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "core_utils/atomic_ptr.hpp"

class UtilTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    /* Write your Setup for Test here*/
  };

  virtual void TearDown() override { /* Write your teardown for Test here*/ };
};

TEST(UtilTest, AtomicPtrDefaultConstruction) {
  struct A {
    int num = 2;
  };

  ne::AtomicPtr<A> atomicPtr;
  ASSERT_EQ(atomicPtr.load()->num, 2);

  ne::NullableAtomicPtr<A> nullablePtr;
  ASSERT_EQ(nullablePtr.load(), nullptr);
}

TEST(UtilTest, AtomicPtrIsNullable) {
  struct A {
    int num = 2;
  };

  ne::AtomicPtr<A> atomicPtr;
  ne::NullableAtomicPtr<A>& nullablePtr = atomicPtr;
  ASSERT_EQ(nullablePtr.load()->num, 2);
}