#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>

TEST_CASE("Record is assignable") {
    edfio::Record<char> a(10);
    a()[0] = 'X';
    edfio::Record<char> b(10);
    b = a;  // Must compile and work
    CHECK(b()[0] == 'X');
    CHECK(b.Size() == 10);
}

TEST_CASE("Record move-assignment works") {
    edfio::Record<char> a(5);
    a()[0] = 42;
    edfio::Record<char> b(5);
    b = std::move(a);
    CHECK(b()[0] == 42);
}

TEST_CASE("Record concatenation") {
    edfio::Record<char> a(3);
    edfio::Record<char> b(2);
    a()[0] = 1; a()[1] = 2; a()[2] = 3;
    b()[0] = 4; b()[1] = 5;
    auto c = a + b;
    CHECK(c.Size() == 5);
    CHECK(c()[3] == 4);
}

TEST_CASE("Record Size derives from vector") {
    edfio::Record<char> r(7);
    CHECK(r.Size() == 7);
    CHECK(r.Size() == r().size());
}

TEST_CASE("Record copy construction") {
    edfio::Record<char> a(3);
    a()[0] = 'A'; a()[1] = 'B'; a()[2] = 'C';
    edfio::Record<char> b(a);
    CHECK(b.Size() == 3);
    CHECK(b()[0] == 'A');
    CHECK(b()[2] == 'C');
}
