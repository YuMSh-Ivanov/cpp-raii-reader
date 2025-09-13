#include <cstdio>
#include <cstring>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "raii_reader.h"

TEST_CASE("default constructor")
{
    raii_reader reader;
    REQUIRE_FALSE(reader.is_opened());
    REQUIRE_FALSE(reader.read_char().has_value());

    REQUIRE_FALSE(reader.try_rewind());
}

TEST_CASE("no file")
{
    char non_existing_filename[L_tmpnam];
    CAPTURE(std::tmpnam(non_existing_filename));

    raii_reader reader(non_existing_filename);
    REQUIRE_FALSE(reader.is_opened());
    REQUIRE_FALSE(reader.read_char().has_value());

    REQUIRE_FALSE(reader.try_rewind());
}

TEST_CASE("open no file")
{
    char non_existing_filename[L_tmpnam];
    CAPTURE(std::tmpnam(non_existing_filename));

    raii_reader reader;
    REQUIRE_FALSE(reader.open(non_existing_filename));
    REQUIRE_FALSE(reader.is_opened());
    REQUIRE_FALSE(reader.read_char().has_value());

    REQUIRE_FALSE(reader.try_rewind());
}

static void write_file(const char* str, const char* name)
{
    std::FILE* file = std::fopen(name, "wb");
    if (file == nullptr)
    {
        FAIL("Could not create file for testing :(");
    }
    std::fwrite(str, 1, std::strlen(str), file);
    std::fclose(file);
}

static void read_char_check(raii_reader& reader, unsigned char expected_char)
{
    std::optional<unsigned char> read_char = reader.read_char();
    CAPTURE(expected_char);
    REQUIRE(read_char.has_value());
    REQUIRE(*read_char == expected_char);
}

TEST_CASE("simple read_char")
{
    write_file("Hello", "tmp.txt");

    raii_reader reader("tmp.txt");
    REQUIRE(reader.is_opened());
    read_char_check(reader, 'H');
    read_char_check(reader, 'e');
    read_char_check(reader, 'l');
    read_char_check(reader, 'l');
    read_char_check(reader, 'o');
    REQUIRE_FALSE(reader.read_char().has_value());
}

TEST_CASE("simple open")
{
    write_file("amen", "tmp5.txt");

    raii_reader reader;
    REQUIRE_FALSE(reader.is_opened());

    reader.open("tmp5.txt");
    REQUIRE(reader.is_opened());
    read_char_check(reader, 'a');
    read_char_check(reader, 'm');
    read_char_check(reader, 'e');
    read_char_check(reader, 'n');
    REQUIRE_FALSE(reader.read_char().has_value());
}

TEST_CASE("reopen no file")
{
    char non_existing_filename[L_tmpnam];
    CAPTURE(std::tmpnam(non_existing_filename));

    write_file("Reopen", "tmp13.txt");
    raii_reader reader("tmp13.txt");
    read_char_check(reader, 'R');
    read_char_check(reader, 'e');
    read_char_check(reader, 'o');
    REQUIRE_FALSE(reader.open(non_existing_filename));
    REQUIRE(reader.is_opened());
    read_char_check(reader, 'p');
    read_char_check(reader, 'e');
    read_char_check(reader, 'n');
}

TEST_CASE("simple reopen")
{
    write_file("Hello", "tmp6.txt");
    write_file("world", "tmp7.txt");

    raii_reader reader("tmp6.txt");
    REQUIRE(reader.is_opened());
    read_char_check(reader, 'H');
    read_char_check(reader, 'e');
    read_char_check(reader, 'l');
    read_char_check(reader, 'l');
    read_char_check(reader, 'o');
    REQUIRE_FALSE(reader.read_char().has_value());

    reader.open("tmp7.txt");
    REQUIRE(reader.is_opened());
    read_char_check(reader, 'w');
    read_char_check(reader, 'o');
    read_char_check(reader, 'r');
    read_char_check(reader, 'l');
    read_char_check(reader, 'd');
    REQUIRE_FALSE(reader.read_char().has_value());
}

TEST_CASE("non-ASCII read_char")
{
    const char string[] = {1, 2, 3, 4, -1, -2, -3, 127, -128, '\0'};
    write_file(string, "tmp2.txt");

    raii_reader reader("tmp2.txt");
    REQUIRE(reader.is_opened());
    for (std::size_t i = 0; i < sizeof(string) - 1; i++)
    {
        read_char_check(reader, static_cast<unsigned char>(string[i]));
    }
    REQUIRE_FALSE(reader.read_char().has_value());
}

TEST_CASE("close")
{
    write_file("Test", "tmp3.txt");

    {
        raii_reader reader("tmp3.txt");
    }

    if (std::remove("tmp3.txt") != 0)
    {
        FAIL("Error removing file. Maybe you forgot to close it in raii_file?");
    }
}

TEST_CASE("open, then close")
{
    write_file("Example", "tmp8.txt");

    {
        raii_reader reader;
        reader.open("tmp8.txt");
    }

    if (std::remove("tmp8.txt") != 0)
    {
        FAIL("Error removing file. Maybe you forgot to close it in raii_file?");
    }
}

TEST_CASE("constructor+open, then close")
{
    write_file("Ctor", "tmp9.txt");
    write_file("Open", "tmp10.txt");

    {
        raii_reader reader("tmp9.txt");
        reader.open("tmp10.txt");
        if (std::remove("tmp9.txt") != 0)
        {
            FAIL("Error removing file. Maybe you forgot to close it in raii_file?");
        }
        read_char_check(reader, 'O');
    }

    if (std::remove("tmp10.txt") != 0)
    {
        FAIL("Error removing file. Maybe you forgot to close it in raii_file?");
    }
}

TEST_CASE("open*2, then close")
{
    write_file("1Open", "tmp11.txt");
    write_file("2Open", "tmp12.txt");

    {
        raii_reader reader;
        reader.open("tmp11.txt");
        reader.open("tmp12.txt");

        if (std::remove("tmp11.txt") != 0)
        {
            FAIL("Error removing file. Maybe you forgot to close it in raii_file?");
        }
        read_char_check(reader, '2');
    }

    if (std::remove("tmp12.txt") != 0)
    {
        FAIL("Error removing file. Maybe you forgot to close it in raii_file?");
    }
}

TEST_CASE("rewind")
{
    write_file("Lorem\nipsum", "tmp4.txt");

    raii_reader reader("tmp4.txt");
    read_char_check(reader, 'L');
    read_char_check(reader, 'o');
    read_char_check(reader, 'r');
    read_char_check(reader, 'e');
    read_char_check(reader, 'm');
    read_char_check(reader, '\n');
    REQUIRE(reader.try_rewind());
    read_char_check(reader, 'L');
    read_char_check(reader, 'o');
    read_char_check(reader, 'r');
    read_char_check(reader, 'e');
    read_char_check(reader, 'm');
    read_char_check(reader, '\n');
    read_char_check(reader, 'i');
    read_char_check(reader, 'p');
    REQUIRE(reader.try_rewind());
    read_char_check(reader, 'L');
}

TEST_CASE("standard_input")
{
    std::ungetc('x', stdin);
    read_char_check(raii_reader::standard_input, 'x');
    REQUIRE(raii_reader::standard_input.is_opened());
}

#if defined(__unix__)
TEST_CASE("standard_input rewind")
{
    REQUIRE_FALSE(raii_reader::standard_input.try_rewind());
}
#endif

TEST_CASE("explicit constructor")
{
    REQUIRE_FALSE(std::is_convertible_v<const char*, raii_reader>);
}
