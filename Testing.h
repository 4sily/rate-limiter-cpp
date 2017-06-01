#pragma once

#include <cassert>
#include <sstream>

struct AssertionException : public std::logic_error
{
	explicit AssertionException(const char* message) : std::logic_error(message) {}
};

template <typename A, typename B>
void AssertEqual(A&& a, B&& b, const char* fileName, int lineNumber)
{
	assert(a == b);
	if (a != b)
	{
		std::stringstream msg;
		msg << "Equality assumption is broken: file '" << fileName << "'; line " << lineNumber;
		throw AssertionException(msg.str().c_str());
	}
}

#define ASSERT_EQUAL(a, b) AssertEqual((a), (b), __FILE__, __LINE__)

