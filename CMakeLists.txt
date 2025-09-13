cmake_minimum_required (VERSION 3.12)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

project("RAIIReader")

Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.0
)
FetchContent_MakeAvailable(Catch2)

# Add source to this project's executable.
add_executable(tests "conf-src/sanitizers_configure.cpp" "tests/test.cpp"  "src/raii_reader.cpp")

target_include_directories(tests PUBLIC src)

set_property(TARGET tests PROPERTY CXX_STANDARD 17)

string(TIMESTAMP TIMESTAMP %s)
add_compile_definitions(CONSTANT_VALUE=${TIMESTAMP})

# Enabling most errors
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(tests PRIVATE /W4 /permissive-)
    if(TREAT_WARNINGS_AS_ERRORS)
        target_compile_options(tests PRIVATE /WX)
    endif()
    target_compile_options(tests PRIVATE /wd4244 /wd4702)
    target_compile_definitions(tests PRIVATE -D_CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(tests PRIVATE -Wall -pedantic -Wextra)
    target_compile_options(tests PRIVATE -Wold-style-cast)
    target_compile_options(tests PRIVATE -Wextra-semi)
    target_compile_options(tests PRIVATE -Woverloaded-virtual)
    target_compile_options(tests PRIVATE -Wzero-as-null-pointer-constant)
    if(TREAT_WARNINGS_AS_ERRORS)
        target_compile_options(tests PRIVATE -Werror -pedantic-errors)
    endif()
endif()

# Compiler specific warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tests PRIVATE -Wshadow=compatible-local)
    target_compile_options(tests PRIVATE -Wduplicated-branches)
    target_compile_options(tests PRIVATE -Wduplicated-cond)
    # Disabled due to GCC bug
    # target_compile_options(tests PRIVATE -Wnull-dereference)
    target_compile_options(tests PRIVATE -Wno-maybe-uninitialized)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(tests PRIVATE -Wshadow-uncaptured-local)
    target_compile_options(tests PRIVATE -Wloop-analysis)
    target_compile_options(tests PRIVATE -Wno-self-assign-overloaded)
    target_compile_options(tests PRIVATE -Wno-missing-braces)
endif()

# Enabling debug standard library for Debug and Sanitized Debug on linux
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT WIN32)
    message(STATUS "Enabling _GLIBCXX_DEBUG")
    add_compile_definitions(_GLIBCXX_DEBUG)
endif()

option(USE_SANITIZERS "Enable to build with undefined, address and leak sanitizers" OFF)
if(USE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(STATUS "Enabling ASAN")
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")
        target_compile_options(tests PUBLIC /fsanitize=address)
        target_compile_definitions(tests PUBLIC _DISABLE_STRING_ANNOTATION=1 _DISABLE_VECTOR_ANNOTATION=1)
    else()
        message(STATUS "Enabling UBSan, ASan and LSan")
        target_compile_options(tests PUBLIC -fsanitize=undefined,address,pointer-compare,pointer-subtract,leak)
        target_link_options(tests PUBLIC -fsanitize=undefined,address,pointer-compare,pointer-subtract,leak)

        target_compile_options(tests PUBLIC -fno-sanitize-recover=all -fno-optimize-sibling-calls -fno-omit-frame-pointer)
    endif()
else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "EditAndContinue")
    endif()
endif()

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
