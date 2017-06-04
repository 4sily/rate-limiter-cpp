# Simple rate limiter

# What can be improved here

+ Turn this console application into the static/dynamic library
+ Move tests to the separate library/application
+ Use some of the existing testing frameworks in that library/applicaton
  + e.g. Catch/Googletest/Boost.Test/doctest/whatever
+ (Arguable) use [Boost.Circular buffer](http://www.boost.org/doc/libs/1_64_0/doc/html/circular_buffer.html) instead of the self-implemented inside the class HitQueue
+ Abstract away the interface of Limiter via either abstract base class or template trait class
  + Also de-couple the tests from the specific impelemntation of Limiter
+ Add several other implementations, especially for the standard rate limiting algorithms
  + [Token bucket](https://en.m.wikipedia.org/wiki/Token_bucket)
  + [Leaky bucket](https://en.m.wikipedia.org/wiki/Leaky_bucket)
    + [as a meter](https://en.m.wikipedia.org/wiki/Leaky_bucket#The_Leaky_Bucket_Algorithm_as_a_Meter)
    + [as a queue](https://en.m.wikipedia.org/wiki/Leaky_bucket#The_Leaky_Bucket_Algorithm_as_a_Queue)