/**
 * @author Alexander Van Craen
 * @copyright 2024-today All Rights Reserved
 * @license This file is released under the MIT license.
 *          See the LICENSE.md file in the project root for full license information.
 *
 * @brief Tests for the backend-agnostic hws::detail helpers in include/hws/utility.hpp.
 * @details These are pure, header-only/free functions that compile into every configuration regardless of which
 *          hardware backends are enabled - no hardware, subprocess, or vendor SDK required.
 */

#include "hws/utility.hpp"  // hws::detail::{starts_with, trim, to_lower_case, split, is_integer, convert_to, split_as, map_entry_to_string, quote}

#include "doctest/doctest.h"

#include <map>          // std::map
#include <optional>     // std::optional, std::nullopt
#include <stdexcept>    // std::runtime_error, std::invalid_argument
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector

TEST_CASE("starts_with") {
    CHECK(hws::detail::starts_with("Architecture: x86_64", "Architecture"));
    CHECK_FALSE(hws::detail::starts_with("Architecture: x86_64", "architecture"));  // case-sensitive
    CHECK_FALSE(hws::detail::starts_with("ab", "abc"));                             // start longer than the full string
    CHECK(hws::detail::starts_with("abc", ""));                                     // every string starts with the empty string
    CHECK(hws::detail::starts_with("", ""));
}

TEST_CASE("trim") {
    SUBCASE("leading and trailing whitespace") {
        CHECK_EQ(hws::detail::trim("  value  "), "value");
        CHECK_EQ(hws::detail::trim("\t\v\r\n\f value \t\v\r\n\f"), "value");
    }
    SUBCASE("no whitespace to trim") {
        CHECK_EQ(hws::detail::trim("value"), "value");
    }
    SUBCASE("all-whitespace input trims to empty") {
        CHECK_EQ(hws::detail::trim("   "), "");
    }
    SUBCASE("empty input") {
        CHECK_EQ(hws::detail::trim(""), "");
    }
    SUBCASE("interior whitespace is preserved") {
        CHECK_EQ(hws::detail::trim("  a b  "), "a b");
    }
}

TEST_CASE("to_lower_case") {
    CHECK_EQ(hws::detail::to_lower_case("MiXeD Case"), "mixed case");
    CHECK_EQ(hws::detail::to_lower_case("already lower"), "already lower");
    CHECK_EQ(hws::detail::to_lower_case(""), "");
}

TEST_CASE("split") {
    SUBCASE("default delimiter") {
        const std::vector<std::string_view> tokens = hws::detail::split("a b c");
        REQUIRE_EQ(tokens.size(), 3);
        CHECK_EQ(tokens[0], "a");
        CHECK_EQ(tokens[1], "b");
        CHECK_EQ(tokens[2], "c");
    }
    SUBCASE("empty string returns an empty vector, not a vector with one empty token") {
        CHECK(hws::detail::split("").empty());
    }
    SUBCASE("no delimiter present returns the whole string as the only token") {
        const std::vector<std::string_view> tokens = hws::detail::split("value", ',');
        REQUIRE_EQ(tokens.size(), 1);
        CHECK_EQ(tokens[0], "value");
    }
    SUBCASE("consecutive delimiters produce an empty token in between") {
        const std::vector<std::string_view> tokens = hws::detail::split("a,,b", ',');
        REQUIRE_EQ(tokens.size(), 3);
        CHECK_EQ(tokens[0], "a");
        CHECK_EQ(tokens[1], "");
        CHECK_EQ(tokens[2], "b");
    }
    SUBCASE("a trailing delimiter produces a trailing empty token") {
        const std::vector<std::string_view> tokens = hws::detail::split("a,", ',');
        REQUIRE_EQ(tokens.size(), 2);
        CHECK_EQ(tokens[0], "a");
        CHECK_EQ(tokens[1], "");
    }
    SUBCASE("a leading delimiter produces a leading empty token") {
        const std::vector<std::string_view> tokens = hws::detail::split(",a", ',');
        REQUIRE_EQ(tokens.size(), 2);
        CHECK_EQ(tokens[0], "");
        CHECK_EQ(tokens[1], "a");
    }
}

TEST_CASE("is_integer") {
    CHECK(hws::detail::is_integer("12345"));
    CHECK_FALSE(hws::detail::is_integer("12.345"));
    CHECK_FALSE(hws::detail::is_integer("-5"));    // the sign character is not a digit
    CHECK_FALSE(hws::detail::is_integer("12a45"));
    // std::all_of over an empty range is vacuously true - document the actual (perhaps surprising) behavior
    CHECK(hws::detail::is_integer(""));
}

TEST_CASE("convert_to<T>") {
    SUBCASE("to std::string trims the value") {
        CHECK_EQ(hws::detail::convert_to<std::string>("  value  "), "value");
    }
    SUBCASE("to bool from the literal words, case-insensitively") {
        CHECK_EQ(hws::detail::convert_to<bool>("true"), true);
        CHECK_EQ(hws::detail::convert_to<bool>("TRUE"), true);
        CHECK_EQ(hws::detail::convert_to<bool>(" False "), false);
    }
    SUBCASE("to bool from a number: zero is false, anything else is true") {
        CHECK_EQ(hws::detail::convert_to<bool>("0"), false);
        CHECK_EQ(hws::detail::convert_to<bool>("1"), true);
        CHECK_EQ(hws::detail::convert_to<bool>("-3"), true);
    }
    SUBCASE("to char requires exactly one character after trimming") {
        CHECK_EQ(hws::detail::convert_to<char>(" x "), 'x');
        CHECK_THROWS_AS(static_cast<void>(hws::detail::convert_to<char>("ab")), std::runtime_error);
        CHECK_THROWS_AS(static_cast<void>(hws::detail::convert_to<char>("")), std::runtime_error);
    }
    SUBCASE("to a floating point type via std::stof/std::stod/std::stold") {
        CHECK_EQ(hws::detail::convert_to<float>("3.5"), doctest::Approx(3.5f));
        CHECK_EQ(hws::detail::convert_to<double>("2.25"), doctest::Approx(2.25));
        // std::stof throws std::invalid_argument on failure - a different exception type than the integral path below
        CHECK_THROWS_AS(static_cast<void>(hws::detail::convert_to<float>("not_a_number")), std::invalid_argument);
        // a valid numeric prefix followed by trailing garbage must be rejected, not silently truncated to the
        // prefix's value (regression check: std::stof("3.5x") alone would otherwise happily return 3.5)
        CHECK_THROWS_AS(static_cast<void>(hws::detail::convert_to<float>("3.5x")), std::runtime_error);
    }
    SUBCASE("to an integral type via std::from_chars, trimmed first") {
        CHECK_EQ(hws::detail::convert_to<int>("42"), 42);
        CHECK_EQ(hws::detail::convert_to<int>("  42  "), 42);
        CHECK_EQ(hws::detail::convert_to<unsigned long long>("18446744073709551615"), 18446744073709551615ull);
        // std::from_chars failure is translated into hws's own std::runtime_error, not left as an error code
        CHECK_THROWS_AS(static_cast<void>(hws::detail::convert_to<int>("not_a_number")), std::runtime_error);
        CHECK_THROWS_AS(static_cast<void>(hws::detail::convert_to<int>("")), std::runtime_error);
        // a valid numeric prefix followed by trailing garbage must be rejected, not silently truncated to the
        // prefix's value (regression check: std::from_chars("42x", ...) alone would otherwise happily return 42)
        CHECK_THROWS_AS(static_cast<void>(hws::detail::convert_to<int>("42x")), std::runtime_error);
    }
}

TEST_CASE("split_as<T>") {
    SUBCASE("splits and converts every token") {
        const std::vector<int> values = hws::detail::split_as<int>("1 2 3");
        CHECK_EQ(values, std::vector<int>{ 1, 2, 3 });
    }
    SUBCASE("a custom delimiter is honored") {
        const std::vector<int> values = hws::detail::split_as<int>("1,2,3", ',');
        CHECK_EQ(values, std::vector<int>{ 1, 2, 3 });
    }
    SUBCASE("empty input returns an empty vector") {
        CHECK(hws::detail::split_as<int>("").empty());
    }
    SUBCASE("a conversion failure on any token propagates as an exception") {
        CHECK_THROWS_AS(static_cast<void>(hws::detail::split_as<int>("1 x 3")), std::runtime_error);
    }
}

TEST_CASE("map_entry_to_string<MapType>") {
    SUBCASE("nullopt formats as an empty string") {
        CHECK_EQ(hws::detail::map_entry_to_string(std::optional<std::map<std::string, int>>{ std::nullopt }), "");
    }
    SUBCASE("scalar-valued map entries") {
        // std::map (not unordered_map) so iteration order - and thus the expected string - is deterministic
        const std::optional<std::map<std::string, int>> map{ std::map<std::string, int>{ { "a", 1 }, { "b", 2 } } };
        CHECK_EQ(hws::detail::map_entry_to_string(map), "{a, 1}, {b, 2}");
    }
    SUBCASE("vector-valued map entries are bracketed") {
        const std::optional<std::map<std::string, std::vector<int>>> map{
            std::map<std::string, std::vector<int>>{ { "a", { 1, 2, 3 } } }
        };
        CHECK_EQ(hws::detail::map_entry_to_string(map), "{a, [1, 2, 3]}");
    }
}

TEST_CASE("quote<T>") {
    SUBCASE("quotes every value") {
        const std::vector<std::string> quoted = hws::detail::quote(std::vector<int>{ 1, 2, 3 });
        CHECK_EQ(quoted, std::vector<std::string>{ "\"1\"", "\"2\"", "\"3\"" });
    }
    SUBCASE("works for strings too") {
        const std::vector<std::string> quoted = hws::detail::quote(std::vector<std::string>{ "a", "b" });
        CHECK_EQ(quoted, std::vector<std::string>{ "\"a\"", "\"b\"" });
    }
    SUBCASE("empty input returns an empty vector") {
        CHECK(hws::detail::quote(std::vector<int>{}).empty());
    }
}
