// Andrew Naplavkov

#ifndef STEP_TEST_SUFFIX_HPP
#define STEP_TEST_SUFFIX_HPP

#include <chrono>
#include <cstdint>
#include <iterator>
#include <map>
#include <step/suffix_array.hpp>
#include <step/suffix_tree.hpp>
#include <step/test/utility.hpp>
#include <string_view>

using namespace std::literals;
using namespace std::chrono;

TEST_CASE("suffix_array_hello_world")
{
    auto str = "how can I quickly search for text within a document?"sv;
    step::suffix_array tree{str};
    CHECK(tree.find("quick"sv) == 10);
}

TEST_CASE("suffix_tree_hello_world")
{
    auto str = "use the quick find feature to search for a text"sv;
    step::suffix_tree tree{};
    std::copy(str.begin(), str.end(), std::back_inserter(tree));
    CHECK(tree.find("quick"sv) == 8);
}

template <typename SuffixTree>
std::string tree_topology(const SuffixTree& tree)
{
    std::ostringstream os;
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            os << std::setw(len) << std::setfill(' ')
               << std::string_view{tree.begin(str), tree.size(str)};
            if (tree.suffix(str))
                os << " [" << str.second - len << "]";
            os << "\n";
        },
        [](auto&&...) {});
    return os.str();
}

template <typename SuffixTree>
auto tree_to_array(const SuffixTree& tree)
{
    std::vector<typename SuffixTree::size_type> result;
    result.reserve(tree.size());
    tree.visit(
        [&](const auto& str, const auto&, auto len) {
            if (tree.suffix(str))
                result.push_back(str.second - len);
        },
        [](auto&&...) {});
    return result;
}

TEST_CASE("suffix_tree_topology")
{
    struct {
        std::string_view str;
        std::string_view expect;
    } tests[] = {

        {"", ""},

        {"abcabxabcd$", R"(
$ [10]
ab
  c
   abxabcd$ [0]
   d$ [6]
  xabcd$ [3]
b
 c
  abxabcd$ [1]
  d$ [7]
 xabcd$ [4]
c
 abxabcd$ [2]
 d$ [8]
d$ [9]
xabcd$ [5]
)"},

        {"BANANA$", R"(
$ [6]
A
 $ [5]
 NA
   $ [3]
   NA$ [1]
BANANA$ [0]
NA
  $ [4]
  NA$ [2]
)"},

        {"VVuVVVOm$", R"(
$ [8]
Om$ [6]
V
 Om$ [5]
 V
  Om$ [4]
  VOm$ [3]
  uVVVOm$ [0]
 uVVVOm$ [1]
m$ [7]
uVVVOm$ [2]
)"},

        {"wwwJwww$", R"(
$ [7]
Jwww$ [3]
w
 $ [6]
 Jwww$ [2]
 w
  $ [5]
  Jwww$ [1]
  w
   $ [4]
   Jwww$ [0]
)"},
    };

    for (auto& [str, expect] : tests) {
        step::suffix_tree<char, size_t, std::map> tree{};
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(tree_topology(tree) == expect);
    }
}

TEST_CASE("suffix_array_n_tree_find")
{
    struct {
        std::string_view str;
        std::string_view pattern;
        std::initializer_list<size_t> expect;
    } tests[] = {
        {"GEEKSFORGEEKS$", "GEEKS", {0, 8}},
        {"GEEKSFORGEEKS$", "GEEK1", {}},
        {"GEEKSFORGEEKS$", "FOR", {5}},
        {"AABAACAADAABAAABAA$", "AABA", {0, 9, 13}},
        {"AABAACAADAABAAABAA$", "AA", {0, 3, 6, 9, 12, 13, 16}},
        {"AABAACAADAABAAABAA$", "AAE", {}},
        {"AAAAAAAAA$", "AAAA", {0, 1, 2, 3, 4, 5}},
        {"AAAAAAAAA$", "AA", {0, 1, 2, 3, 4, 5, 6, 7}},
        {"AAAAAAAAA$", "A", {0, 1, 2, 3, 4, 5, 6, 7, 8}},
        {"AAAAAAAAA$", "AB", {}},
    };
    for (auto& [str, pattern, expect] : tests) {
        step::suffix_array arr{str};
        CHECK(arr.find(str) == 0);
        CHECK(arr.find("not found"sv) == arr.size());
        auto arr_all = arr.find_all(pattern);
        CHECK(std::is_permutation(
            arr_all.first, arr_all.second, expect.begin(), expect.end()));

        step::suffix_tree tree{};
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(tree.find(str) == 0);
        CHECK(tree.find(""sv) == 0);
        CHECK(tree.find("not found"sv) == tree.size());
        auto tree_all = tree.find_all(pattern);
        CHECK(std::is_permutation(
            tree_all.begin(), tree_all.end(), expect.begin(), expect.end()));
    }
}

TEST_CASE("suffix_array_n_tree_cross_check")
{
    for (size_t i = 0; i < 1000; ++i) {
        auto str = make_random_string(100);
        str += str;
        str.back() = '$';

        step::suffix_array<char, uint16_t> arr{str};
        step::suffix_tree<char, uint16_t, std::map> tree{};
        tree.reserve(str.size());
        std::copy(str.begin(), str.end(), std::back_inserter(tree));
        CHECK(arr.index() == tree_to_array(tree));

        auto pattern = make_random_string(4);
        auto arr_all = arr.find_all(pattern);
        auto tree_all = tree.find_all(pattern);
        CHECK(std::is_permutation(
            arr_all.first, arr_all.second, tree_all.begin(), tree_all.end()));
    }
}
/*
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>

template <typename Key, typename T>
using flat_map = boost::container::flat_map<
    Key,
    T,
    std::less<>,
    boost::container::small_vector<std::pair<Key, T>, 8>>;
*/
inline auto benchmark_tree(std::string_view str)
{
    step::suffix_tree<char, uint32_t /*, flat_map*/> tree{};
    tree.reserve(str.size());
    auto start = high_resolution_clock::now();
    std::copy(str.begin(), str.end(), std::back_inserter(tree));
    return duration_cast<milliseconds>(high_resolution_clock::now() - start)
        .count();
}

inline auto benchmark_array(std::string_view str)
{
    auto start = high_resolution_clock::now();
    step::suffix_array<char, uint32_t> arr{str /*, std::execution::par_unseq*/};
    return duration_cast<milliseconds>(high_resolution_clock::now() - start)
        .count();
}

inline void benchmark(size_t len)
{
    auto str = make_random_string(len);
    str.back() = '$';
    auto rnd_tree = benchmark_tree(str);
    auto rnd_arr = benchmark_array(str);

    auto mid = str.begin() + str.size() / 2;
    std::copy(str.begin(), mid, mid);
    str.back() = '$';
    auto dual_tree = benchmark_tree(str);
    auto dual_arr = benchmark_array(str);

    std::cout << " " << right(str.size()) << " | " << right(rnd_tree) << " | "
              << right(dual_tree) << " | " << right(rnd_arr) << " | "
              << right(dual_arr) << " |\n";
}

/*
 Intel(R) Core(TM) i7-6600U CPU @ 2.60GHz

              | suffix tree  | suffix tree  | suffix arr   | suffix arr   |
 text (chars) | random (ms)  | doubled (ms) | random (ms)  | doubled (ms) |
       262144 |           76 |           39 |           27 |          102 |
       524288 |          166 |           94 |           55 |          224 |
      1048576 |          357 |          194 |          132 |          546 |
      2097152 |          784 |          385 |          310 |         1403 |
      4194304 |         1964 |          891 |          692 |         3780 |
      8388608 |         5101 |         2373 |         1807 |         7681 |
     16777216 |        10385 |         4805 |         3251 |        17202 |
*/
TEST_CASE("suffix_array_n_tree_complexity")
{
    std::cout << " " << left("") << " | " << left("suffix tree") << " | "
              << left("suffix tree") << " | " << left("suffix array") << " | "
              << left("suffix array") << " |\n"
              << " " << left("text (chars)") << " | " << left("random (ms)")
              << " | " << left("doubled (ms)") << " | " << left("random (ms)")
              << " | " << left("doubled (ms)") << " |\n";
    for (size_t i = 18; i < 22; ++i)
        benchmark(std::exp2(i));
}

#endif  // STEP_TEST_SUFFIX_HPP
