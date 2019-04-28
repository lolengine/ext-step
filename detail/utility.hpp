// Andrew Naplavkov

#ifndef STEP_UTILITY_HPP
#define STEP_UTILITY_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <stack>
#include <type_traits>
#include <utility>
#include <vector>

namespace step {

/// @see https://en.cppreference.com/w/cpp/utility/variant/visit
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

/// @see https://en.cppreference.com/w/cpp/iterator/iter_t
template <class It>
using iter_value_t = typename std::iterator_traits<It>::value_type;

/// @see https://en.cppreference.com/w/cpp/ranges/iterator_t
template <class Rng>
using iterator_t = decltype(std::begin(std::declval<Rng&>()));

template <class Rng>
using range_value_t = iter_value_t<iterator_t<Rng>>;

/// @see https://en.cppreference.com/w/cpp/experimental/is_detected
template <class Default, class, template <class...> class Op, class... Args>
struct detector {
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
    using type = Op<Args...>;
};

template <class Default, template <class...> class Op, class... Args>
using detected_or_t = typename detector<Default, void, Op, Args...>::type;

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

template <class T>
using key_equal_t = typename T::key_equal;

template <class T>
using key_equal_or_equivalence_t =
    detected_or_t<equivalence<typename T::key_compare>, key_equal_t, T>;

template <class T>
std::enable_if_t<std::is_unsigned_v<T>, T> flip(T n)
{
    return std::numeric_limits<T>::max() - n;
}

template <class T>
auto size(const std::pair<T, T>& pair)
{
    return pair.second - pair.first;
}

template <class T, class... It>
void append(T& dest, std::pair<It, It>... src)
{
    using size_type = decltype(dest.size());
    dest.reserve(dest.size() + ((size_type)size(src) + ...));
    (std::copy(src.first, src.second, std::back_inserter(dest)), ...);
}

template <class F, class... It>
auto invoke(F f, std::pair<It, It>... args)
{
    auto count = (size(args) + ...);
    if (count < std::numeric_limits<int8_t>::max())
        return f((uint8_t)count, args...);
    else if (count < std::numeric_limits<int16_t>::max())
        return f((uint16_t)count, args...);
    else if (count < std::numeric_limits<int32_t>::max())
        return f((uint32_t)count, args...);
    else
        return f((size_t)count, args...);
}

template <class T>
void move_backward(std::stack<T>& src, std::stack<T>& dest)
{
    for (; !src.empty(); src.pop())
        dest.push(std::move(src.top()));
}

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

}  // namespace step

#endif  // STEP_UTILITY_HPPs
