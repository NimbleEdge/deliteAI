/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <functional>

#include "char_stream.hpp"
#include "json_stream.hpp"

TEST(StreamTest, JSONStringStreamTest) {
  auto charStream = CharStream::construct();
  JSONStringStream jsonStream{charStream};
  charStream->set_subscriber(std::bind(&JSONStringStream::parse_ahead, std::ref(jsonStream)));

  ASSERT_EQ(jsonStream.to_string(), "");

  charStream->push("     ");
  ASSERT_EQ(jsonStream.to_string(), "");

  charStream->push("  \" hello ");
  ASSERT_EQ(jsonStream.to_string(), " hello ");

  charStream->push("world\" haha");
  ASSERT_EQ(jsonStream.to_string(), " hello world");

  ASSERT_EQ(charStream->empty(), false);
  ASSERT_EQ(charStream->pop(), ' ');
  ASSERT_EQ(charStream->pop(), 'h');
  ASSERT_EQ(charStream->pop(), 'a');
}

TEST(StreamTest, JSONNumberStreamTest) {
  auto charStream = CharStream::construct();
  JSONNumberStream jsonStream{charStream};
  charStream->set_subscriber(std::bind(&JSONNumberStream::parse_ahead, std::ref(jsonStream)));

  charStream->push("123");
  ASSERT_EQ(jsonStream.get_number<int>(), 123);

  charStream->push(".45");
  ASSERT_EQ(jsonStream.get_number<double>(), 123.45);

  charStream->push("78E-2");
  ASSERT_EQ(jsonStream.get_number<double>(), 1.234578);
}

TEST(StreamTest, SimpleJSONStreamTest) {
  auto charStream = CharStream::construct();
  JSONStream jsonStream{charStream};
  charStream->set_subscriber(std::bind(&JSONStream::parse_ahead, std::ref(jsonStream)));

  ASSERT_EQ(jsonStream.to_json_string(), R"({
})");

  charStream->push(R"({"A": "B)");
  ASSERT_EQ(jsonStream.to_json_string(), R"({
    "A": "B",
})");

  charStream->push(R"(", "he)");
  ASSERT_EQ(jsonStream.to_json_string(), R"({
    "A": "B",
})");

  charStream->push(R"(llo": "wor)");
  ASSERT_EQ(jsonStream.to_json_string(), R"({
    "hello": "wor",
    "A": "B",
})");

  charStream->push(R"(ld"})");
  ASSERT_EQ(jsonStream.to_json_string(), R"({
    "hello": "world",
    "A": "B",
})");
}

TEST(StreamTest, NestedSimpleJSONTest) {
  auto charStream = CharStream::construct();
  JSONStream jsonStream{charStream};
  charStream->set_subscriber(std::bind(&JSONStream::parse_ahead, std::ref(jsonStream)));

  charStream->push(R"({"A": {"nest": "o)");
  ASSERT_EQ(jsonStream.to_json_string(), R"({
    "A": {
        "nest": "o",
    },
})");

  charStream->push(R"(k"}, "B": "C"})");
  ASSERT_EQ(jsonStream.to_json_string(), R"({
    "B": "C",
    "A": {
        "nest": "ok",
    },
})");
}

TEST(StreamTest, JSONArrayStreamTest) {
  auto charStream = CharStream::construct();
  JSONArrayStream arrayStream{charStream};
  charStream->set_subscriber(std::bind(&JSONArrayStream::parse_ahead, std::ref(arrayStream)));

  charStream->push(R"([{"a": "b", "c": "d"}, {"w" : "x", "y" : "z"}])");
  ASSERT_EQ(arrayStream.to_json_string(), R"([ {
    "c": "d",
    "a": "b",
}, {
    "y": "z",
    "w": "x",
}, ])");
}
