#pragma once

#include <cassert>
#include <sstream>

struct AssertionException : public std::logic_error {
  explicit inline AssertionException(const char *message)
      : std::logic_error(message) {}
};

template <typename Actual, typename Expected>
void AssertEqual(Actual &&actual, Expected &&expected, const char *fileName,
                 int lineNumber) {
  if (expected != actual) {
    std::stringstream msg;
    msg << "Equality assumption is broken: file '" << fileName << "'; line "
        << lineNumber;
    msg << "; Expected: " << expected << "; Actual: " << actual;
    throw AssertionException(msg.str().c_str());
  }
}

#define ASSERT_EQUAL(a, b) AssertEqual((a), (b), __FILE__, __LINE__)
