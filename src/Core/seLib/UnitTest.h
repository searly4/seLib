#pragma once

namespace seLib {
namespace UnitTest {

template <typename T, T V1, T V2>
struct CheckEquality {
	static constexpr bool value{ false };
};

template <typename T, T V>
struct CheckEquality<T, V, V> {
	static constexpr bool value{ true };
};

}} // seLib::UnitTest

#define SELIB_UNITTEST_EQUALITY(TESTNAME, TEST_EXPRESSION, COMPARE_VALUE) \
	static_assert(seLib::UnitTest::CheckEquality<decltype(COMPARE_VALUE), static_cast<decltype(COMPARE_VALUE)>(TEST_EXPRESSION), COMPARE_VALUE>::value, \
	"Unit test failed: " TESTNAME " (" #TEST_EXPRESSION " != " #COMPARE_VALUE ")");

#define SELIB_UNITTEST_TRUE(TESTNAME, TEST_EXPRESSION) \
	static_assert(TEST_EXPRESSION, \
	"Unit test failed: " TESTNAME " (" #TEST_EXPRESSION " != true)");
