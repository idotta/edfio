# EdfIO C++23 Refactoring -- Implementation Plan

Target: **clang++ 21.1.8**, **CMake 4.0.2**, **C++23**, **Windows 11**
Architecture invariant: header-only library; "iterators to navigate files like arrays" pattern preserved.

---

## Phase 0: Infrastructure

**Goal:** Establish CMake build, integrate doctest via FetchContent, create the test harness, verify everything compiles before touching a single line of library code.

### 0.1 Create `CMakeLists.txt` (root)

**New file:** `C:/dev/code/edfio/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 4.0)
project(edfio VERSION 0.2.0 LANGUAGES CXX)

# ---- Header-only library target ----
add_library(edfio INTERFACE)
target_include_directories(edfio INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(edfio INTERFACE cxx_std_23)

# ---- Tests ----
option(EDFIO_BUILD_TESTS "Build tests" ON)
if(EDFIO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

### 0.2 Create `tests/CMakeLists.txt`

**New file:** `C:/dev/code/edfio/tests/CMakeLists.txt`

```cmake
include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

# Convenience function: one call per test source file
function(edfio_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE edfio doctest::doctest)
    target_compile_options(${name} PRIVATE
        -Wall -Wextra -Wpedantic -Wno-c++98-compat -fsanitize=address,undefined
    )
    target_link_options(${name} PRIVATE -fsanitize=address,undefined)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Copy test fixture
configure_file(
    ${CMAKE_SOURCE_DIR}/Calib5.edf
    ${CMAKE_CURRENT_BINARY_DIR}/Calib5.edf
    COPYONLY
)

# --- Test executables (added incrementally per phase) ---
edfio_add_test(test_record)
edfio_add_test(test_reader)
edfio_add_test(test_processor_sample)
edfio_add_test(test_iterators)
edfio_add_test(test_writer)
edfio_add_test(test_processor_utils)
```

### 0.3 Skeleton test files

Create one `.cpp` per test target with a minimal doctest `main()` and a single `CHECK(true)`. These will be populated in later phases.

**New files:**
- `C:/dev/code/edfio/tests/test_record.cpp`
- `C:/dev/code/edfio/tests/test_reader.cpp`
- `C:/dev/code/edfio/tests/test_processor_sample.cpp`
- `C:/dev/code/edfio/tests/test_iterators.cpp`
- `C:/dev/code/edfio/tests/test_writer.cpp`
- `C:/dev/code/edfio/tests/test_processor_utils.cpp`

Each file follows this skeleton:

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>

TEST_CASE("placeholder") {
    CHECK(true);
}
```

### 0.4 Build verification

```
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_STANDARD=23
cmake --build build
ctest --test-dir build --output-on-failure
```

**Dependencies:** None.

---

## Phase 1: Bug Fixes

**Goal:** Fix every critical and high-severity bug identified during analysis, with regression tests proving each fix. Zero refactoring -- minimal, surgical patches only.

### 1.1 CRITICAL -- `ProcessorSampleRecord` data corruption

**File:** `C:/dev/code/edfio/include/edfio/processor/impl/ProcessorSampleRecord.ipp`

**Bug A -- `unsigned char < 0` always false:**
The loop variable `r` is `unsigned char`. The test `r < 0` is always false, so sign-extension for negative samples (BDF 3-byte, EDF 2-byte) never triggers. `sample = -1` is dead code.

**Bug B -- double `idx++`:**
`idx++` appears in the condition `if (r < 0 && idx++)` AND again at `idx++` at bottom of loop body. If the condition were ever true, `idx` would be incremented twice per iteration.

**Current code (lines 16-32):**
```cpp
template<SampleType SampleT>
inline typename ProcessorSampleRecord<SampleT>::ProcType
ProcessorSampleRecord<SampleT>::operator()(Record<char> record)
{
    DigiType sample = 0;
    size_t idx = 0;
    for (unsigned char r : record())
    {
        if (r < 0 && idx++)       // BUG: always false
            sample = -1;
        sample <<= 8;
        sample |= r;
        idx++;
    }

    if (std::is_same<DigiType, ProcType>::value)
        return sample;
    return impl::ConvertSample(m_offset, m_scaling, sample);
}
```

**Fixed code:**
The correct algorithm reads bytes little-endian (LSB first -- per EDF/BDF spec), assembles them, then sign-extends the result. The current code reads big-endian which is also wrong for EDF, but is consistent with how the rest of the codebase uses it (ProcessorSample writes big-endian too). So we preserve the byte order but fix sign extension:

```cpp
template<SampleType SampleT>
inline typename ProcessorSampleRecord<SampleT>::ProcType
ProcessorSampleRecord<SampleT>::operator()(Record<char> record)
{
    DigiType sample = 0;
    auto const& bytes = record();
    std::size_t const nbytes = bytes.size();

    // Assemble bytes (big-endian order as written by ProcessorSample)
    for (std::size_t i = 0; i < nbytes; ++i)
    {
        sample <<= 8;
        sample |= static_cast<unsigned char>(bytes[i]);
    }

    // Sign-extend: if high bit of the MSB is set, the value is negative
    if (nbytes > 0 && nbytes < sizeof(DigiType))
    {
        unsigned int sign_bit = 1u << (nbytes * 8 - 1);
        if (sample & sign_bit)
            sample |= ~((1 << (nbytes * 8)) - 1);
    }

    if constexpr (std::is_same_v<DigiType, ProcType>)
        return sample;
    return impl::ConvertSample(m_offset, m_scaling, sample);
}
```

**Test (in `test_processor_sample.cpp`):**
```cpp
TEST_CASE("ProcessorSampleRecord sign-extends negative 2-byte samples") {
    // -100 as signed 16-bit = 0xFF9C
    Record<char> rec(2);
    rec()[0] = static_cast<char>(0xFF);
    rec()[1] = static_cast<char>(0x9C);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == -100);
}

TEST_CASE("ProcessorSampleRecord positive 2-byte samples") {
    Record<char> rec(2);
    rec()[0] = static_cast<char>(0x00);
    rec()[1] = static_cast<char>(0x64);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == 100);
}

TEST_CASE("ProcessorSampleRecord 3-byte BDF negative sample") {
    // -1000 as signed 24-bit = 0xFFFC18
    Record<char> rec(3);
    rec()[0] = static_cast<char>(0xFF);
    rec()[1] = static_cast<char>(0xFC);
    rec()[2] = static_cast<char>(0x18);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == -1000);
}
```

---

### 1.2 CRITICAL -- `TalStore::prev()` out-of-bounds UB

**File:** `C:/dev/code/edfio/include/edfio/store/TalStore.hpp`

**Bug:** Line 235: `auto first = m_stream().rend() + off;`
`rend()` already points past-the-reverse-end. Adding a positive `off` goes further out of bounds -- undefined behavior.

The intent is to iterate backward from position `off`. To go backwards from byte offset `off` in the underlying vector, we need `rbegin() + (size - off)`.

**Current code (lines 228-258):**
```cpp
size_type prev(size_type off)
{
    if (off <= 0)
        throw std::length_error("Iterator not decrementable");

    if (off > 0)
    {
        auto first = m_stream().rend() + off;   // BUG
        auto last = m_stream().rend();
        // ...
```

**Fixed code:**
```cpp
size_type prev(size_type off)
{
    if (off == 0)
        throw std::length_error("Iterator not decrementable");

    auto const& data = m_stream();
    auto const sz = data.size();

    // Walk backward from position (off - 1), skipping zeros
    size_type pos = off;
    while (pos > 0 && data[pos - 1] == 0)
        --pos;

    if (pos == 0)
        throw std::length_error("Iterator not decrementable");

    // Find the start of the previous non-zero TAL
    size_type end_of_tal = pos;
    while (pos > 0 && data[pos - 1] != 0)
        --pos;

    m_value.assign(data.begin() + pos, data.begin() + end_of_tal);
    return pos;
}
```

**Test (in `test_iterators.cpp`):** Requires constructing a TalStore from a crafted SignalRecordStore, which is complex. Instead, test with a round-trip: iterate forward collecting TALs, then iterate backward and verify they appear in reverse order. Requires `Calib5.edf` to be an EDF+ file with annotations. If `Calib5.edf` is plain EDF (no annotations), write a unit test that creates a vector with known TAL structure and tests the `prev()` logic via a mock. A more practical approach:

```cpp
TEST_CASE("TalStore reverse iteration does not crash") {
    // This is an integration-level test using a real file.
    // If Calib5.edf has no annotations, this test is skipped.
    std::ifstream stream("Calib5.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    edfio::ReaderHeaderExam reader;
    auto header = reader(stream);
    // If no annotation signals, skip
    bool has_annot = false;
    for (auto const& sig : header.m_signals) {
        if (sig.m_detail.m_isAnnotation) { has_annot = true; break; }
    }
    if (!has_annot) { MESSAGE("No annotations in Calib5.edf, skipping"); return; }
    // ... full test using TalStore iterators
}
```

---

### 1.3 CRITICAL -- `RecordStore::iterator::operator-` inverted

**File:** `C:/dev/code/edfio/include/edfio/store/RecordStore.hpp`

**Bug (line 173):**
```cpp
difference_type operator-(iterator it) const
{
    return difference_type(it.m_offset - m_offset);  // inverted!
}
```

For `a - b`, the standard requires `a.offset - b.offset`, but this returns `b.offset - a.offset`.

**Fix:**
```cpp
difference_type operator-(iterator it) const
{
    if (!m_context)
        throw std::invalid_argument("Invalid context");
    if (m_context != it.m_context)
        throw std::invalid_argument("Iterators incompatible");
    return difference_type(m_offset - it.m_offset);
}
```

**Same bug in `RecordSink::iterator::operator-` (line 187):**

**File:** `C:/dev/code/edfio/include/edfio/sink/RecordSink.hpp`

```cpp
// Current (line 187):
return difference_type(it.m_offset - m_offset);
// Fixed:
return difference_type(m_offset - it.m_offset);
```

**Test (in `test_iterators.cpp`):**
```cpp
TEST_CASE("RecordStore iterator subtraction returns (a - b) not (b - a)") {
    std::ifstream stream("Calib5.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    edfio::ReaderHeaderExam reader;
    auto header = reader(stream);
    auto store = edfio::detail::CreateDataRecordStore(stream, header.m_general);
    auto a = store.begin() + 2;
    auto b = store.begin();
    CHECK((a - b) == 2);
    CHECK((b - a) == -2);
}
```

---

### 1.4 CRITICAL -- `Record<ValT>` has `const size_t m_size` breaking assignment

**File:** `C:/dev/code/edfio/include/edfio/core/Record.hpp`

**Bug (line 64):**
```cpp
const size_t m_size;
```

`const` data member makes the implicit copy/move assignment operators deleted. The class has a copy constructor but relies on implicit assignment in many places (e.g., `m_value = ...` in Store::load paths, where `Record<char>` is assigned into `m_value`). The `const` is doubly redundant because `m_size` should track `m_value.size()`.

**Fix:** Remove `const` from `m_size`, add a proper assignment operator, or better yet, derive `m_size` from `m_value.size()` and remove the redundant member:

```cpp
template <typename ValT = char>
struct Record
{
    using ValueType = ValT;
    using VectorType = std::vector<ValueType>;

    Record() = delete;

    Record(size_t recordSize)
        : m_value(recordSize, 0) {}

    Record(typename VectorType::const_iterator first,
           typename VectorType::const_iterator last)
        : m_value(first, last) {}

    Record(const Record&) = default;
    Record(Record&&) = default;
    Record& operator=(const Record&) = default;
    Record& operator=(Record&&) = default;

    size_t Size() const { return m_value.size(); }
    const VectorType& operator()() const { return m_value; }
    VectorType& operator()() { return m_value; }

    Record<ValueType> operator+(const Record<ValueType>& record) const
    {
        Record<ValueType> tmp(Size() + record.Size());
        std::copy(m_value.begin(), m_value.end(), tmp().begin());
        std::copy(record().begin(), record().end(), tmp().begin() + Size());
        return tmp;
    }

    VectorType m_value;
};
```

Note: `m_size` is removed entirely. `Size()` now returns `m_value.size()`. The stream operators (`operator>>`, `operator<<`) already call `record.resize(r.Size(), 0)` which will become a no-op now that the vector is already the right size.

**Impact:** The `operator>>` and `operator<<` templates at bottom of Record.hpp call `record.resize(r.Size(), 0)`. After the fix, `r.Size()` returns `m_value.size()`, so `resize` is a no-op, which is correct.

**Test (in `test_record.cpp`):**
```cpp
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
```

---

### 1.5 HIGH -- `const_cast` in 12 places to fake const-correctness

**Files:**
- `C:/dev/code/edfio/include/edfio/store/RecordStore.hpp` (lines 222, 226, 234, 238)
- `C:/dev/code/edfio/include/edfio/store/TalStore.hpp` (lines 142, 146, 154, 158)
- `C:/dev/code/edfio/include/edfio/sink/RecordSink.hpp` (lines 227, 231, 239, 243)

**Bug:** The `const` begin/end/cbegin/cend methods cast away `const` on `this` to construct iterators. This is technically UB if the object is actually const, and defeats the purpose of const-correctness.

**Root cause:** The iterator stores a raw pointer to the non-const Store/Sink. The `const_iterator` is just `typedef iterator const`, which means `const iterator` -- a const copy of a mutable iterator, NOT a true const_iterator.

**Fix (Phase 1 -- minimal, safe):** Make `const_iterator` a proper alias and have iterators store a `const` pointer when needed. However, the minimal fix is to acknowledge the library is not used in truly const contexts and mark this for Phase 4 (iterator overhaul with deducing this). For Phase 1, we fix the UB by making the `const` overloads non-const (or by making the iterator's pointer member mutable). The pragmatic fix:

Replace `typedef iterator const const_iterator;` with a true `const_iterator` class. But that is a large change. Phase 1 minimal: change to `using const_iterator = iterator;` (drop the `const`), remove the `const_cast`, and accept that `begin() const` just returns a mutable iterator. This is no worse than the current const_cast UB.

Actually, the cleanest Phase 1 fix: store `m_context` as `const`-compatible. The load/getR/getP methods need mutable access. Use `mutable` on the cache fields (`m_value`) in RecordStore and TalStore. Then the iterator can hold a `const Store*` and call const methods that mutate only mutable cache fields.

This is substantial but safe. **Defer to Phase 4** (iterator overhaul with deducing this) -- mark const_cast sites with `// FIXME(Phase4): const_cast removed by deducing-this iterator redesign`.

For Phase 1, add the `// FIXME` comments but do NOT change behavior. The const_cast pattern is technically UB only if the underlying object is truly const, which never happens in this codebase.

---

### 1.6 HIGH -- Iterators don't satisfy `std::random_access_iterator` concept

**Files:**
- `C:/dev/code/edfio/include/edfio/core/Device.hpp` (iterator base class)
- `C:/dev/code/edfio/include/edfio/store/RecordStore.hpp`
- `C:/dev/code/edfio/include/edfio/sink/RecordSink.hpp`

**Issues:**
1. No `iterator_concept` type alias (required for C++20+ iterator concepts).
2. `operator-` returns `b - a` instead of `a - b` (fixed in 1.3).
3. No `n + it` form (only `it + n`).
4. `size_type` is `unsigned long long` but `difference_type` is `long long` -- `operator+` / `operator-` take `size_type` (unsigned) but should take `difference_type` (signed) per the standard.

**Fix (Phase 1 -- minimal):**

In `Device.hpp` iterator, add:
```cpp
using iterator_concept = IterCategory;
```

In `RecordStore::iterator`, add friend free function:
```cpp
friend iterator operator+(size_type off, const iterator& it)
{
    return it + off;
}
```

Change `operator+=(size_type off)` to `operator+=(difference_type off)` (and correspondingly for `operator-=`, `operator+`, `operator-` scalar overloads). This is a **Phase 4** task due to cascading changes. Phase 1 adds only the `iterator_concept` alias and the `n + it` overload.

**Defer signed/unsigned parameter changes to Phase 4.**

---

### 1.7 HIGH -- `catch(std::exception e)` by value in 5 .ipp files

**Files (5 locations):**
1. `C:/dev/code/edfio/include/edfio/reader/impl/ReaderHeaderGeneral.ipp` line 43
2. `C:/dev/code/edfio/include/edfio/reader/impl/ReaderHeaderSignal.ipp` line 55
3. `C:/dev/code/edfio/include/edfio/writer/impl/WriterHeaderGeneral.ipp` line 44
4. `C:/dev/code/edfio/include/edfio/writer/impl/WriterHeaderSignals.ipp` line 53
5. `C:/dev/code/edfio/include/edfio/writer/impl/WriterRecord.ipp` line 29

**Bug:** Catching by value slices derived exception types.

**Fix:** Change all to `catch (const std::exception& e)`:

```cpp
// Before:
catch (std::exception e)
// After:
catch (const std::exception&)
```

(The `e` variable is unused in all cases -- the handlers immediately throw a new exception.)

**Test:** No specific test needed; this is a correctness fix verified by compiler warnings with `-Wcatch-value`.

---

### 1.8 HIGH -- `off < 0` on unsigned in load() methods

**Files:**
1. `C:/dev/code/edfio/include/edfio/store/DatarecordStore.hpp` line 37
2. `C:/dev/code/edfio/include/edfio/include/edfio/store/SignalrecordStore.hpp` line 40
3. `C:/dev/code/edfio/include/edfio/store/SignalSampleStore.hpp` line 43
4. `C:/dev/code/edfio/include/edfio/store/TimeStampStore.hpp` line 40

**Bug:** `size_type` is `unsigned long long`. The check `off < 0` is always false.

**Fix:** Remove the dead `off < 0` check. The `off >= size()` check is sufficient because `off` is unsigned and comes from iterator arithmetic that already bounds-checks.

```cpp
// Before:
if (off < 0 || off >= size())
// After:
if (off >= size())
```

---

### Phase 1 summary of file changes

| File | Changes |
|------|---------|
| `processor/impl/ProcessorSampleRecord.ipp` | Rewrite sample assembly + sign extension |
| `store/TalStore.hpp` | Rewrite `prev()` method |
| `store/RecordStore.hpp` | Fix `operator-` sign, add FIXME for const_cast |
| `sink/RecordSink.hpp` | Fix `operator-` sign, add FIXME for const_cast |
| `core/Record.hpp` | Remove `const` from `m_size`, or remove `m_size` entirely |
| `core/Device.hpp` | Add `iterator_concept` alias |
| `reader/impl/ReaderHeaderGeneral.ipp` | `catch(std::exception e)` -> `catch(const std::exception&)` |
| `reader/impl/ReaderHeaderSignal.ipp` | Same |
| `writer/impl/WriterHeaderGeneral.ipp` | Same |
| `writer/impl/WriterHeaderSignals.ipp` | Same |
| `writer/impl/WriterRecord.ipp` | Same |
| `store/DatarecordStore.hpp` | Remove `off < 0` |
| `store/SignalrecordStore.hpp` | Remove `off < 0` |
| `store/SignalSampleStore.hpp` | Remove `off < 0` |
| `store/TimeStampStore.hpp` | Remove `off < 0` |

**Dependencies:** Phase 0 must be complete (CMake builds, tests compile).

---

## Phase 2: Low-Risk Modernization

**Goal:** Apply safe, mechanical transformations that improve code quality without altering semantics. Every change is individually small and independently testable by compilation + existing Phase 1 tests.

### 2.1 Remove `return std::move(local)` -- NRVO prevention (21 instances)

**Files and lines:**
1. `header/HeaderUtils.hpp` line 62: `return std::move(header);` in `CreateHeaderGeneral`
2. `header/HeaderUtils.hpp` line 89: `return std::move(CreateHeaderGeneral(...));` -- both the outer move and the nested move
3. `header/HeaderUtils.hpp` line 104: `return std::move(header);` in `CreateHeaderGeneralPlus`
4. `header/HeaderUtils.hpp` line 140: `return std::move(signal);` in `CreateHeaderSignal`
5. `store/detail/StoreUtils.hpp` line 33: `return std::move(DataRecordStore{...});`
6. `store/detail/StoreUtils.hpp` line 44: `return std::move(SignalRecordStore{...});`
7. `store/detail/StoreUtils.hpp` line 56: `return std::move(SignalSampleStore{...});`
8. `store/detail/StoreUtils.hpp` line 67: `return std::move(TimeStampStore{...});`
9. `sink/detail/SinkUtils.hpp` line 31: `return std::move(DataRecordSink{...});`
10. `sink/detail/SinkUtils.hpp` line 42: `return std::move(SignalRecordSink{...});`
11. `reader/impl/ReaderHeaderGeneral.ipp` line 47: `return std::move(hdr);`
12. `reader/impl/ReaderHeaderSignal.ipp` line 59: `return std::move(signals);`
13. `reader/impl/ReaderHeaderExam.ipp` lines 33, 40, 44, 62: Four `std::move` wrapping return values
14. `writer/impl/WriterHeaderExam.ipp` lines 28, 31: Two `std::move` on temporaries passed to functions (these are actually fine since they are function arguments, not return values -- leave alone)
15. `processor/impl/ProcessorHeaderExam.ipp` line 45: `return std::move(HeaderExam{...});`
16. `processor/impl/ProcessorHeaderGeneral.ipp` line 162: `return std::move(out);`
17. `processor/impl/ProcessorHeaderSignalFields.ipp` line 326: `return std::move(signals);`
18. `processor/impl/ProcessorHeaderSignal.ipp` line 98: `return std::move(out);`
19. `processor/impl/ProcessorTalRecord.ipp` line 96: `return std::move(out);`
20. `processor/impl/ProcessorTimeStampRecord.ipp` line 56: `return std::move(timestamp);`
21. `processor/impl/ProcessorSample.ipp` line 33: `return std::move(record);`

**Fix:** For every `return std::move(x);` where `x` is a local variable or temporary, change to `return x;`.

Example:
```cpp
// Before:
return std::move(header);
// After:
return header;
```

For `auto header = std::move(CreateHeaderGeneral(...));` change to `auto header = CreateHeaderGeneral(...);`.

For `auto general = std::move(procGeneralFields(std::move(generalFields)));` in ReaderHeaderExam.ipp -- the inner `std::move(generalFields)` is correct (moving into a by-value parameter), but the outer `std::move()` around the return value assignment is fine to keep (assigning to a local, not returning). Actually, `auto general = std::move(procGeneralFields(...))` prevents copy elision from the function return. Change to `auto general = procGeneralFields(std::move(generalFields));`.

---

### 2.2 Replace `typedef` with `using` (all files)

**Scope:** Every file using `typedef`. This is a mechanical find-and-replace.

```cpp
// Before:
typedef iterator const const_iterator;
typedef std::reverse_iterator<iterator> reverse_iterator;
typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
typedef Store<...> store_type;
typedef device_type::iterator iterator;

// After:
using const_iterator = iterator;  // (also fixing the const issue noted in 1.5)
using reverse_iterator = std::reverse_iterator<iterator>;
using const_reverse_iterator = std::reverse_iterator<const_iterator>;
using store_type = Store<...>;
using iterator = typename device_type::iterator;
```

**Files:**
- `core/Device.hpp` (6 typedefs)
- `store/Store.hpp` (3 typedefs)
- `sink/Sink.hpp` (2 typedefs)
- `store/RecordStore.hpp` (3 typedefs)
- `store/TalStore.hpp` (4 typedefs)
- `sink/RecordSink.hpp` (3 typedefs)
- `store/DatarecordStore.hpp` (4 typedefs)
- `store/SignalrecordStore.hpp` (4 typedefs)
- `store/SignalSampleStore.hpp` (4 typedefs)
- `store/TimeStampStore.hpp` (4 typedefs)
- `sink/DataRecordSink.hpp` (4 typedefs)
- `sink/SignalRecordSink.hpp` (4 typedefs)

---

### 2.3 Replace `static` functions in headers with `inline`

**Files with `static` functions that should be `inline`:**
- `core/DataFormat.hpp`: `IsPlus`, `IsEdf`, `IsBdf`, `GetSampleBytes` (4 functions)
- `Utils.hpp`: `GetError` (1 function)
- `processor/detail/ProcessorUtils.hpp`: all `static` functions in `impl::` and `detail::` namespaces (9 functions)
- `header/HeaderUtils.hpp`: `CreateHeaderGeneral`, `CreateHeaderGeneralPlus`, `CreateHeaderSignal` (3 functions)
- `store/detail/StoreUtils.hpp`: 4 template functions (already have `static` but templates are implicitly inline, so just remove `static`)
- `sink/detail/SinkUtils.hpp`: 2 template functions (same)

**Fix pattern:**
```cpp
// Before:
static const bool IsPlus(DataFormat format) { ... }
// After:
inline constexpr bool IsPlus(DataFormat format) { ... }
```

For `GetError` in Utils.hpp:
```cpp
// Before:
static const char* GetError(FileErrc err)
// After:
inline constexpr const char* GetError(FileErrc err)
```

For non-template `static` functions in ProcessorUtils.hpp (e.g., `ReduceString`, `GetFormatName`, `GetMonthFromString`, `GetStringFromMonth`, `to_string_decimal`):
```cpp
// Before:
static std::string ReduceString(const std::string &value)
// After:
inline std::string ReduceString(const std::string &value)
```

For `ADDITIONAL_SEPARATOR`:
```cpp
// Before:
static const char ADDITIONAL_SEPARATOR = '|';
// After:
inline constexpr char ADDITIONAL_SEPARATOR = '|';
```

For `Config.hpp`:
```cpp
// Before:
static constexpr ProcessorErrorCheck PROCESSOR_ERROR_CHECKING = ...
// After:
inline constexpr ProcessorErrorCheck PROCESSOR_ERROR_CHECKING = ...
```

---

### 2.4 Fix `std::regex` performance in `ReduceString`

**File:** `C:/dev/code/edfio/include/edfio/processor/detail/ProcessorUtils.hpp` line 104-107

**Bug:** `std::regex("^ +| +$|( ) +")` is constructed on every call. std::regex construction is extremely expensive (up to 10ms per call). This function is called 12 times per header parse (see ProcessorHeaderGeneralFields.ipp lines 345-356 and ProcessorHeaderSignalFields.ipp lines 317-323).

**Fix:** Replace with a simple string trim + collapse function:
```cpp
inline std::string ReduceString(const std::string& value)
{
    // Trim leading spaces
    auto start = value.find_first_not_of(' ');
    if (start == std::string::npos) return "";
    // Trim trailing spaces
    auto end = value.find_last_not_of(' ');
    // Collapse interior runs of spaces to single space
    std::string result;
    result.reserve(end - start + 1);
    bool prev_space = false;
    for (size_t i = start; i <= end; ++i) {
        if (value[i] == ' ') {
            if (!prev_space) {
                result += ' ';
                prev_space = true;
            }
        } else {
            result += value[i];
            prev_space = false;
        }
    }
    return result;
}
```

**Test (in `test_processor_utils.cpp`):**
```cpp
TEST_CASE("ReduceString trims and collapses spaces") {
    using edfio::detail::ReduceString;
    CHECK(ReduceString("  hello   world  ") == "hello world");
    CHECK(ReduceString("   ") == "");
    CHECK(ReduceString("no_change") == "no_change");
    CHECK(ReduceString(" a  b  c ") == "a b c");
    CHECK(ReduceString("") == "");
}
```

---

### 2.5 Fix typo `m_dararecord` -> `m_datarecord`

**Files:**
- `C:/dev/code/edfio/include/edfio/core/Annotation.hpp` line 27: `long long m_dararecord = 0;`
- `C:/dev/code/edfio/include/edfio/processor/impl/ProcessorTimeStampRecord.ipp` line 24: `timestamp.m_dararecord = datarecord;`
- `C:/dev/code/edfio/include/edfio/processor/impl/ProcessorTalRecord.ipp` line 87: `annot.m_dararecord = datarecord;`

**Fix:** Rename to `m_datarecord` in all three locations.

---

### 2.6 Fix double semicolons

**File:** `C:/dev/code/edfio/include/edfio/sink/SignalRecordSink.hpp`
- Line 47: `size_type offset = (sz - m_headerOffset) % m_datarecordSize;;`
- Line 61: `size_type offset = (sz - m_headerOffset) % m_datarecordSize;;`

**Fix:** Remove the duplicate semicolons.

---

### 2.7 Make `DataFormat` functions `constexpr`

**File:** `C:/dev/code/edfio/include/edfio/core/DataFormat.hpp`

Make `IsPlus`, `IsEdf`, `IsBdf`, `GetSampleBytes` all `inline constexpr`. (Overlaps with 2.3.)

---

### 2.8 Make `GetError` `constexpr`

**File:** `C:/dev/code/edfio/include/edfio/Utils.hpp`

Change `GetError` to `inline constexpr`. It only uses `if/else` chains returning string literals, which is valid constexpr in C++23.

---

### 2.9 Fix `GetStringFromMonth` unsigned underflow

**File:** `C:/dev/code/edfio/include/edfio/processor/detail/ProcessorUtils.hpp` lines 95-102

**Bug:** Parameter is `size_t idx`. Line 97: `idx--`. If `idx == 0`, this wraps to `SIZE_MAX`. The subsequent `idx >= 0` check (line 99) is always true because `idx` is unsigned.

**Fix:**
```cpp
inline std::string GetStringFromMonth(size_t idx)
{
    static const std::vector<std::string> months = {
        "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
        "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
    if (idx >= 1 && idx <= 12)
        return months[idx - 1];
    return "JAN";
}
```

**Test (in `test_processor_utils.cpp`):**
```cpp
TEST_CASE("GetStringFromMonth handles boundaries") {
    CHECK(edfio::detail::GetStringFromMonth(0) == "JAN");  // was UB
    CHECK(edfio::detail::GetStringFromMonth(1) == "JAN");
    CHECK(edfio::detail::GetStringFromMonth(12) == "DEC");
    CHECK(edfio::detail::GetStringFromMonth(13) == "JAN");
}
```

---

### 2.10 Remove `<regex>` include

**File:** `C:/dev/code/edfio/include/edfio/processor/detail/ProcessorUtils.hpp`

After replacing `ReduceString` in 2.4, remove `#include <regex>`. This significantly improves compilation time.

---

### Phase 2 summary

All changes are mechanical. No architectural changes. Tests from Phase 1 serve as regression. Additional tests in `test_processor_utils.cpp` for ReduceString and GetStringFromMonth.

**Dependencies:** Phase 1 complete. All Phase 1 tests passing.

---

## Phase 3: Core C++23 Refactoring

**Goal:** Rewrite internal implementations using C++23 features for clarity and safety. Public API signatures may change. Every file touched gets updated tests.

### 3.1 `if constexpr` replaces SFINAE in ProcessorUtils

**File:** `C:/dev/code/edfio/include/edfio/processor/detail/ProcessorUtils.hpp`

**Current:** 4 overloads of `CheckFormatErrors` using `std::enable_if`:

```cpp
template <ProcessorErrorCheck Check, typename CharT>
static bool CheckFormatErrors(const typename std::enable_if<Check == ProcessorErrorCheck::Strict, std::basic_string<CharT>>::type &str) { ... }
// ... 3 more overloads
```

**Replace with:**
```cpp
namespace impl {
    template <ProcessorErrorCheck Check, typename Container>
    inline bool CheckFormatErrors(const Container& data)
    {
        if constexpr (Check == ProcessorErrorCheck::Permissive) {
            return false;
        } else {
            for (auto c : data) {
                if (!std::isprint(static_cast<unsigned char>(c)))
                    return true;
            }
            return false;
        }
    }
}

namespace detail {
    template <typename Container>
    inline bool CheckFormatErrors(const Container& data)
    {
        return impl::CheckFormatErrors<config::PROCESSOR_ERROR_CHECKING>(data);
    }
}
```

This replaces 4 overloads with 2 function templates.

**Also** replace `std::is_same<DigiType, ProcType>::value` in ProcessorSampleRecord.ipp and ProcessorSample.ipp with `if constexpr (std::is_same_v<...>)` (already done in Phase 1 fix for ProcessorSampleRecord).

---

### 3.2 Spaceship operator (`<=>`) for iterators

**Files:**
- `C:/dev/code/edfio/include/edfio/store/RecordStore.hpp`
- `C:/dev/code/edfio/include/edfio/store/TalStore.hpp`
- `C:/dev/code/edfio/include/edfio/sink/RecordSink.hpp`

**Current:** Each iterator class has 6 comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`), totalling ~120 lines across 3 files.

**Replace with:**
```cpp
// In RecordStore::iterator:
bool operator==(const iterator& it) const
{
    return m_offset == it.m_offset && m_context == it.m_context;
}

std::strong_ordering operator<=>(const iterator& it) const
{
    if (m_context != it.m_context)
        throw std::invalid_argument("Iterators incompatible");
    return m_offset <=> it.m_offset;
}
```

This replaces 6 operators with 2, and `!=`, `<`, `>`, `<=`, `>=` are all synthesized by the compiler. Saves ~80 lines total.

**Include `<compare>` in the relevant files.**

---

### 3.3 `std::format` replaces `ostringstream` formatting

**File:** `C:/dev/code/edfio/include/edfio/processor/impl/ProcessorHeaderGeneral.ipp`

**Current (lines 53-57):**
```cpp
std::ostringstream oss;
oss << std::setw(2) << std::setfill('0') << day << ".";
oss << std::setw(2) << std::setfill('0') << month << ".";
oss << std::setw(2) << std::setfill('0') << year;
out.m_startDate(oss.str());
```

**Replace with:**
```cpp
out.m_startDate(std::format("{:02d}.{:02d}.{:02d}", day, month, year));
```

Same for start time (lines 64-68):
```cpp
out.m_startTime(std::format("{:02d}.{:02d}.{:02d}", hour, minute, second));
```

And in the "Plus" Recording field (line 137):
```cpp
auto dateStr = std::format("{:02d}-{}-{}", day ? day : 1,
                           detail::GetStringFromMonth(month),
                           year ? year : 1984);
fields.push_back(dateStr);
```

Remove `#include <sstream>` and `#include <iomanip>`, add `#include <format>`.

---

### 3.4 `std::from_chars` replaces `stoi` / `stod` + try/catch

**Files:**
- `processor/impl/ProcessorHeaderGeneralFields.ipp` (6 uses of stoi/stoll/stod)
- `processor/impl/ProcessorHeaderSignalFields.ipp` (5 uses of stoi/stod)
- `processor/impl/ProcessorTalRecord.ipp` (2 uses of stod)

**Pattern:**
```cpp
// Before:
try {
    int day = std::stoi(startdate);
} catch (...) {
    throw std::invalid_argument(GetError(FileErrc::FileContainsFormatErrors));
}

// After:
int day = 0;
auto [ptr, ec] = std::from_chars(startdate.data(), startdate.data() + startdate.size(), day);
if (ec != std::errc{})
    throw std::invalid_argument(GetError(FileErrc::FileContainsFormatErrors));
```

**Note:** `std::from_chars` for `double` has full support in C++23 / clang 21. Include `<charconv>`.

---

### 3.5 `std::string_view` for read-only string parameters

**Files:**
- `processor/detail/ProcessorUtils.hpp`: `ReduceString(const std::string&)` -> `ReduceString(std::string_view)`
- `processor/detail/ProcessorUtils.hpp`: `GetMonthFromString(const std::string&)` -> `GetMonthFromString(std::string_view)`
- `processor/detail/ProcessorUtils.hpp`: `GetFormatName(DataFormat)` already returns string (keep as is)

**Example:**
```cpp
inline int GetMonthFromString(std::string_view str) {
    static constexpr std::array months = {
        "JAN"sv, "FEB"sv, "MAR"sv, "APR"sv, "MAY"sv, "JUN"sv,
        "JUL"sv, "AUG"sv, "SEP"sv, "OCT"sv, "NOV"sv, "DEC"sv
    };
    for (size_t i = 0; i < months.size(); ++i) {
        if (str == months[i]) return static_cast<int>(i + 1);
    }
    return 0;
}
```

---

### 3.6 `std::expected<T, FileErrc>` for error handling (optional / future API)

**Scope:** This is a significant API change. Add `std::expected` returning overloads alongside the existing throw-based ones. Do NOT remove the throwing versions (that would break users).

**Files:**
- `reader/ReaderHeaderExam.hpp` / `.ipp`
- `reader/ReaderHeaderGeneral.hpp` / `.ipp`
- `reader/ReaderHeaderSignal.hpp` / `.ipp`

**Example new API:**
```cpp
struct ReaderHeaderExam : Reader<char> {
    // Existing (throws):
    HeaderExam operator()(Stream& stream);
    // New (returns expected):
    std::expected<HeaderExam, FileErrc> try_read(Stream& stream) noexcept;
};
```

**Implementation:** Wrap the existing call in a try/catch that maps exceptions to `std::unexpected(FileErrc::...)`.

This is a **non-breaking additive change**. Existing code continues to work.

**Defer implementation details to Phase 4** if time-constrained. The key Phase 3 deliverable is defining the `try_read` API shape.

---

### 3.7 `std::optional` replaces `-1` sentinel in RecordSink

**File:** `C:/dev/code/edfio/include/edfio/sink/RecordSink.hpp`

**Bug:** `size_type m_offset = -1;` where `size_type` is `unsigned long long`. This means `m_offset` is `ULLONG_MAX` when used as "end" sentinel. Multiple comparisons like `m_offset == -1` rely on implicit conversion.

**Fix:** Use `std::optional<size_type>`:
```cpp
std::optional<size_type> m_offset;  // nullopt = end
```

Update all checks:
```cpp
// Before:
if (m_offset == -1)
// After:
if (!m_offset)
```

And the save call:
```cpp
// Before:
m_context->save(m_offset, std::move(value));
// After:
m_context->save(m_offset.value_or(size_type(-1)), std::move(value));
```

This change cascades into `DataRecordSink.hpp` and `SignalRecordSink.hpp` where `save()` checks `off == -1`.

---

### Phase 3 summary

| File | C++23 Feature |
|------|---------------|
| `processor/detail/ProcessorUtils.hpp` | `if constexpr`, `string_view`, `constexpr` arrays |
| `store/RecordStore.hpp` | `<=>` spaceship |
| `store/TalStore.hpp` | `<=>` spaceship |
| `sink/RecordSink.hpp` | `<=>` spaceship, `std::optional` |
| `processor/impl/ProcessorHeaderGeneral.ipp` | `std::format` |
| `processor/impl/ProcessorHeaderGeneralFields.ipp` | `std::from_chars` |
| `processor/impl/ProcessorHeaderSignalFields.ipp` | `std::from_chars` |
| `processor/impl/ProcessorTalRecord.ipp` | `std::from_chars` |
| `reader/*.hpp` | `std::expected` API addition |

**Dependencies:** Phase 2 complete. All Phase 1+2 tests passing.

---

## Phase 4: Architecture -- Ranges, Iterator Overhaul, Coroutines

**Goal:** Make Store/Sink types satisfy the standard ranges concepts. Redesign iterators using C++23 deducing this. Optionally add a coroutine generator for TalStore.

### 4.1 Deducing this eliminates const_cast (12 sites)

**Files:**
- `store/RecordStore.hpp`
- `store/TalStore.hpp`
- `sink/RecordSink.hpp`

**Concept:** C++23 deducing this allows a single function template to handle both const and non-const overloads:

```cpp
// Before (6 functions per class: begin, begin const, cbegin const, end, ...):
iterator begin() { return iterator(this); }
const_iterator begin() const { return const_iterator(const_cast<RecordStore*>(this)); }

// After (deducing this):
auto begin(this auto& self)
{
    return iterator(&self);
}
auto cbegin(this const auto& self)
{
    return const_iterator(&self);
}
```

For this to work, the iterator class must be templated on pointer constness, or the Store's cached `m_value` must be `mutable`. The cleanest approach:

1. Make `m_value` (and `m_bufferPos`, `m_buffer`) `mutable` in RecordStore, TalStore, and derived classes. This is semantically correct: the cache is an implementation detail, and dereferencing a const iterator from a const Store should still work (the file read is a side effect hidden behind the cache).

2. Make `getR()` / `getP()` / `load()` const (now possible because members are mutable).

3. Store a `const RecordStore*` (or `const TalStore*`) in the iterator.

4. Replace all 12 const_cast sites with direct `this`.

5. Use deducing this for `begin`/`end`:

```cpp
auto begin(this auto& self) -> iterator { return iterator(&self, 0); }
auto end(this auto& self) -> iterator { return iterator(&self, self.size()); }
auto cbegin(this const auto& self) -> iterator { return iterator(&self, 0); }
auto cend(this const auto& self) -> iterator { return iterator(&self, self.size()); }
```

This removes 12 const_cast uses and ~36 redundant function definitions.

---

### 4.2 Ranges compliance for RecordStore

**Goal:** `RecordStore` (and derived types) should model `std::ranges::random_access_range` and `std::ranges::sized_range`.

**Requirements:**
1. `begin()` and `end()` must return iterators satisfying `std::random_access_iterator`
2. `size()` must return a value convertible to `std::ranges::range_size_t`
3. Iterator `difference_type` must be signed
4. Iterator `operator-(it, it)` must return `difference_type`
5. `operator+(n, it)` must be provided (free function)
6. All arithmetic operators must use `difference_type`, not `size_type`

**Changes to `Device.hpp`:**
```cpp
template <class Value, class Pointer, class Reference, class Stream, typename IterCategory>
class Device
{
public:
    using stream_type = Stream;
    using device_type = Device;
    using value_type = Value;
    using pointer = Pointer;
    using reference = Reference;
    using difference_type = std::ptrdiff_t;  // was long long
    using size_type = std::size_t;           // was unsigned long long

    struct iterator
    {
        using iterator_concept = IterCategory;
        using difference_type = Device::difference_type;
        using value_type = Device::value_type;
        using reference = Device::reference;
        using pointer = Device::pointer;
    };
    // ...
};
```

**Changes to `RecordStore::iterator`:**
- Change all `size_type` parameters in operators to `difference_type`
- Add `friend iterator operator+(difference_type n, const iterator& it) { return it + n; }`
- Verify `std::random_access_iterator<RecordStore::iterator>` at compile time:

```cpp
static_assert(std::random_access_iterator<RecordStore::iterator>);
```

**Changes to `RecordStore` itself:**
- Add `begin()` / `end()` returning proper iterators (done in 4.1)
- Verify: `static_assert(std::ranges::random_access_range<DataRecordStore>);`

**Test (in `test_iterators.cpp`):**
```cpp
TEST_CASE("DataRecordStore models random_access_range") {
    std::ifstream stream("Calib5.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    edfio::ReaderHeaderExam reader;
    auto header = reader(stream);
    auto store = edfio::detail::CreateDataRecordStore(stream, header.m_general);

    // Use ranges algorithms
    auto count = std::ranges::distance(store);
    CHECK(count == header.m_general.m_datarecordsFile);

    // Random access
    auto it = std::ranges::begin(store);
    auto rec = *(it + 0);
    CHECK(rec.Size() > 0);
}
```

---

### 4.3 TalStore as a `std::ranges::view` with optional coroutine generator

**Current:** TalStore is a bidirectional iterator over TAL strings embedded in annotation signals. The `next()`/`prev()` methods manually parse through a byte vector.

**Option A -- Coroutine Generator:**
```cpp
#include <generator>

std::generator<std::vector<char>> tals(const std::vector<char>& data)
{
    size_t pos = 0;
    while (pos < data.size())
    {
        // Skip zeros
        while (pos < data.size() && data[pos] == 0)
            ++pos;
        if (pos >= data.size())
            break;
        // Find end of TAL
        size_t start = pos;
        while (pos < data.size() && data[pos] != 0)
            ++pos;
        co_yield std::vector<char>(data.begin() + start, data.begin() + pos);
    }
}
```

This is a forward-only range. TalStore currently claims bidirectional, but the `prev()` implementation is broken (fixed in Phase 1). If bidirectional is not actually needed by users, replace with the generator. If bidirectional is needed, keep the fixed `prev()` and add the generator as a convenience view.

**Option B -- Ranges-based lazy view (no coroutine):**
Use `std::views::split` or a custom view to split the byte vector on null bytes and filter empties:

```cpp
auto tals_view(const std::vector<char>& data)
{
    return data
        | std::views::split('\0')
        | std::views::filter([](auto&& sub) { return !std::ranges::empty(sub); })
        | std::views::transform([](auto&& sub) {
            return std::vector<char>(std::ranges::begin(sub), std::ranges::end(sub));
          });
}
```

**Recommendation:** Implement Option B as the primary API (composable, no overhead). Provide Option A as an alternative `tals_generator()` free function for users who prefer coroutine style.

---

### 4.4 `std::span` for non-owning buffer views

**Where applicable:**
- `ProcessorSampleRecord::operator()` takes `Record<char>` by value. Change to `std::span<const char>`:
```cpp
ProcType operator()(std::span<const char> bytes);
```
- Similarly for `ProcessorTalRecord::operator()` which takes `std::vector<char>` by value.

**Impact:** These are processor function objects used internally. The Store dereferences to `Record<char> const&`, so passing a span is trivial: `proc(std::span(record()))`.

---

### 4.5 Concepts for Device template hierarchy

**Current:** The `Device` base class takes 5 template parameters: `Value, Pointer, Reference, Stream, IterCategory`. This is the CRTP/inheritance chain: `Device -> Store -> RecordStore -> DataRecordStore`.

**Replace with concepts:**
```cpp
template <typename T>
concept Streamable = requires(T& t) {
    { t.good() } -> std::convertible_to<bool>;
    { t.clear() };
};

template <typename T>
concept ReadableStream = Streamable<T> && requires(T& t, std::streamoff off) {
    { t.seekg(off, std::ios::beg) };
    { t.tellg() } -> std::convertible_to<std::streamoff>;
};

template <typename T>
concept WritableStream = Streamable<T> && requires(T& t, std::streamoff off) {
    { t.seekp(off, std::ios::beg) };
    { t.tellp() } -> std::convertible_to<std::streamoff>;
};
```

Then `Device` becomes:
```cpp
template <typename Value, Streamable Stream>
class Device {
    // Value, difference_type, size_type derived automatically
    // No Pointer/Reference/IterCategory template params needed
};
```

**This is a significant redesign.** The existing 5-param template works and is stable. Recommend implementing this as an optional modernization, not a hard requirement. If implemented, all Store and Sink classes need updating.

---

### Phase 4 summary

| Task | Risk | Lines Changed | Benefit |
|------|------|--------------|---------|
| 4.1 Deducing this | Medium | ~120 | Eliminates const_cast UB, -36 function defs |
| 4.2 Ranges compliance | Medium | ~100 | Standard algorithm compatibility |
| 4.3 TalStore view/generator | Low | ~60 | Cleaner TAL parsing |
| 4.4 std::span | Low | ~20 | Zero-copy buffer passing |
| 4.5 Concepts for Device | High | ~200 | Cleaner template hierarchy |

**Dependencies:** Phase 3 complete. All prior tests passing.

**Recommendation:** Implement 4.1 and 4.2 first (highest value). 4.3 and 4.4 are independent and can be done in parallel. 4.5 is optional.

---

## Appendix A: Complete file inventory

All paths relative to `C:/dev/code/edfio/include/edfio/`.

| # | File | Phase 1 | Phase 2 | Phase 3 | Phase 4 |
|---|------|---------|---------|---------|---------|
| 1 | `Config.hpp` | | 2.3 `inline` | | |
| 2 | `Defs.hpp` | | | | |
| 3 | `Utils.hpp` | | 2.3, 2.8 | | |
| 4 | `EdfIO.hpp` | | | | |
| 5 | `core/Annotation.hpp` | | 2.5 typo | | |
| 6 | `core/DataFormat.hpp` | | 2.3, 2.7 | | |
| 7 | `core/Device.hpp` | 1.6 | 2.2 | | 4.2, 4.5 |
| 8 | `core/Field.hpp` | | | | |
| 9 | `core/Record.hpp` | 1.4 | | | |
| 10 | `core/SampleType.hpp` | | | | |
| 11 | `core/StreamIO.hpp` | | | | |
| 12 | `header/HeaderExam.hpp` | | | | |
| 13 | `header/HeaderGeneral.hpp` | | | | |
| 14 | `header/HeaderSignal.hpp` | | | | |
| 15 | `header/HeaderUtils.hpp` | | 2.1 | | |
| 16 | `processor/detail/ProcessorUtils.hpp` | | 2.1, 2.3, 2.4, 2.9, 2.10 | 3.1, 3.5 | |
| 17 | `processor/ProcessorAnnotation.hpp` | | | | |
| 18 | `processor/impl/ProcessorAnnotation.ipp` | | | | |
| 19 | `processor/ProcessorHeaderExam.hpp` | | | | |
| 20 | `processor/impl/ProcessorHeaderExam.ipp` | | 2.1 | | |
| 21 | `processor/ProcessorHeaderGeneral.hpp` | | | | |
| 22 | `processor/impl/ProcessorHeaderGeneral.ipp` | | 2.1 | 3.3 | |
| 23 | `processor/ProcessorHeaderGeneralFields.hpp` | | | | |
| 24 | `processor/impl/ProcessorHeaderGeneralFields.ipp` | | 2.1 | 3.4 | |
| 25 | `processor/ProcessorHeaderSignal.hpp` | | | | |
| 26 | `processor/impl/ProcessorHeaderSignal.ipp` | | 2.1 | | |
| 27 | `processor/ProcessorHeaderSignalFields.hpp` | | | | |
| 28 | `processor/impl/ProcessorHeaderSignalFields.ipp` | | | 3.4 | |
| 29 | `processor/ProcessorSample.hpp` | | | | 4.4 |
| 30 | `processor/impl/ProcessorSample.ipp` | | 2.1 | | 4.4 |
| 31 | `processor/ProcessorSampleRecord.hpp` | | | | 4.4 |
| 32 | `processor/impl/ProcessorSampleRecord.ipp` | 1.1 | | | 4.4 |
| 33 | `processor/ProcessorTalRecord.hpp` | | | | 4.4 |
| 34 | `processor/impl/ProcessorTalRecord.ipp` | | 2.1, 2.5 | 3.4 | 4.4 |
| 35 | `processor/ProcessorTimeStamp.hpp` | | | | |
| 36 | `processor/impl/ProcessorTimeStamp.ipp` | | | | |
| 37 | `processor/ProcessorTimeStampRecord.hpp` | | | | |
| 38 | `processor/impl/ProcessorTimeStampRecord.ipp` | | 2.1, 2.5 | | |
| 39 | `reader/ReaderHeaderExam.hpp` | | | 3.6 | |
| 40 | `reader/impl/ReaderHeaderExam.ipp` | | 2.1 | | |
| 41 | `reader/ReaderHeaderGeneral.hpp` | | | | |
| 42 | `reader/impl/ReaderHeaderGeneral.ipp` | 1.7 | 2.1 | | |
| 43 | `reader/ReaderHeaderSignal.hpp` | | | | |
| 44 | `reader/impl/ReaderHeaderSignal.ipp` | 1.7 | 2.1 | | |
| 45 | `sink/DataRecordSink.hpp` | | 2.2 | 3.7 | 4.1 |
| 46 | `sink/RecordSink.hpp` | 1.3 | 2.2 | 3.2, 3.7 | 4.1 |
| 47 | `sink/SignalRecordSink.hpp` | | 2.2, 2.6 | 3.7 | 4.1 |
| 48 | `sink/Sink.hpp` | | 2.2 | | |
| 49 | `sink/detail/SinkUtils.hpp` | | 2.1 | | |
| 50 | `store/DatarecordStore.hpp` | 1.8 | 2.2 | | 4.1 |
| 51 | `store/RecordStore.hpp` | 1.3 | 2.2 | 3.2 | 4.1, 4.2 |
| 52 | `store/SignalrecordStore.hpp` | 1.8 | 2.2 | | 4.1 |
| 53 | `store/SignalSampleStore.hpp` | 1.8 | 2.2 | | 4.1 |
| 54 | `store/Store.hpp` | | 2.2 | | |
| 55 | `store/TalStore.hpp` | 1.2 | 2.2 | 3.2 | 4.1, 4.3 |
| 56 | `store/TimeStampStore.hpp` | 1.8 | 2.2 | | 4.1 |
| 57 | `store/detail/StoreUtils.hpp` | | 2.1 | | |
| 58 | `writer/WriterHeaderExam.hpp` | | | | |
| 59 | `writer/impl/WriterHeaderExam.ipp` | | 2.1 | | |
| 60 | `writer/WriterHeaderGeneral.hpp` | | | | |
| 61 | `writer/impl/WriterHeaderGeneral.ipp` | 1.7 | | | |
| 62 | `writer/WriterHeaderSignals.hpp` | | | | |
| 63 | `writer/impl/WriterHeaderSignals.ipp` | 1.7 | | | |
| 64 | `writer/WriterRecord.hpp` | | | | |
| 65 | `writer/impl/WriterRecord.ipp` | 1.7 | | | |

---

## Appendix B: Test matrix

| Test File | Covers | Phase |
|-----------|--------|-------|
| `test_record.cpp` | Record assign/move/concat, Size() | 1 |
| `test_reader.cpp` | Header reading from Calib5.edf, round-trip | 1, 2 |
| `test_processor_sample.cpp` | ProcessorSampleRecord sign extension, round-trip with ProcessorSample | 1 |
| `test_iterators.cpp` | RecordStore operator-, DataRecordStore iteration, ranges compliance, TalStore prev | 1, 4 |
| `test_writer.cpp` | Header write round-trip, Record write | 1, 2 |
| `test_processor_utils.cpp` | ReduceString, GetStringFromMonth, CheckFormatErrors, from_chars parsing | 2, 3 |

---

## Appendix C: Build command reference

```bash
# Configure
cmake -B build -G Ninja \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic"

# Build
cmake --build build

# Test
ctest --test-dir build --output-on-failure

# Sanitizer build (for CI)
cmake -B build-san -G Ninja \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra"
cmake --build build-san
ctest --test-dir build-san --output-on-failure
```

---

## Execution order and time estimates

| Phase | Estimated Effort | Blocking | Deliverable |
|-------|-----------------|----------|-------------|
| Phase 0 | 1 hour | None | CMake builds, doctest runs, skeleton tests pass |
| Phase 1 | 4 hours | Phase 0 | All critical/high bugs fixed, regression tests green |
| Phase 2 | 3 hours | Phase 1 | Modernized style, no behavioral changes, all tests green |
| Phase 3 | 6 hours | Phase 2 | C++23 features integrated, new tests for from_chars/format/spaceship |
| Phase 4 | 8 hours | Phase 3 | Ranges-compliant iterators, deducing this, optional coroutine |

**Total estimated effort: ~22 hours**

Each phase produces a standalone, shippable commit. No phase depends on a later phase. If work is stopped after any phase, the codebase is strictly improved.
