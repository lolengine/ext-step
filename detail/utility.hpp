// Andrew Naplavkov

#ifndef STEP_UTILITY_HPP
#define STEP_UTILITY_HPP

#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace step {

struct make_pair {
    template <class Lhs, class Rhs>
    constexpr auto operator()(Lhs&& lhs, Rhs&& rhs) const
    {
        return std::make_pair(std::forward<Lhs>(lhs), std::forward<Rhs>(rhs));
    }
};

struct make_reverse_pair {
    template <class Lhs, class Rhs>
    constexpr auto operator()(Lhs&& lhs, Rhs&& rhs) const
    {
        return std::make_pair(std::forward<Rhs>(rhs), std::forward<Lhs>(lhs));
    }
};

template <class Compare>
class equivalence {
    Compare cmp_;

public:
    template <class T>
    constexpr bool operator()(const T& lhs, const T& rhs) const
    {
        return !cmp_(lhs, rhs) && !cmp_(rhs, lhs);
    }
};

template <class T, size_t N>
class ring_table {
    std::array<std::vector<T>, N> rows_;

public:
    explicit ring_table(size_t cols)
    {
        for (auto& row : rows_)
            row.resize(cols);
    }

    auto& operator[](size_t row) { return rows_[row % N]; }
};

template <class It>
using iterator_value = typename std::iterator_traits<It>::value_type;

template <class Rng>
using range_value = iterator_value<decltype(std::declval<Rng>().begin())>;

template <class, class = std::void_t<>>
struct has_key_equal : std::false_type {
};

template <class T>
struct has_key_equal<T, std::void_t<typename T::key_equal>> : std::true_type {
};

template <class T>
struct key_equal {
    using type = typename T::key_equal;
};

template <class T>
struct key_equivalence {
    using type = equivalence<typename T::key_compare>;
};

template <class T>
using key_equal_or_equivalence =
    typename std::conditional_t<has_key_equal<T>::value,
                                key_equal<T>,
                                key_equivalence<T>>::type;

template <class T, class... It>
void append(T&& to, std::pair<It, It>... from)
{
    to.reserve(to.size() + (std::distance(from.first, from.second) + ...));
    (std::copy(from.first, from.second, std::back_inserter(to)), ...);
}

template <class F, class... It>
auto invoke(F&& f, std::pair<It, It>... args)
{
    auto size = (std::distance(args.first, args.second) + ...);
    if (size < std::numeric_limits<int8_t>::max())
        return f((uint8_t)size, args...);
    else if (size < std::numeric_limits<int16_t>::max())
        return f((uint16_t)size, args...);
    else if (size < std::numeric_limits<int32_t>::max())
        return f((uint32_t)size, args...);
    else
        return f((size_t)size, args...);
}

}  // namespace step

#endif  // STEP_UTILITY_HPPs
