/*****************************************************************************
 *
 * AM numeric facilities
 *
 * released under MIT license
 *
 * 2008-2017 André Müller
 *
 *****************************************************************************/

#ifndef AMLIB_INTERPOLATORS_H_
#define AMLIB_INTERPOLATORS_H_


#include <type_traits>
#include <algorithm>
#include <cmath>


namespace am {
namespace interpolator {


namespace detail {

///@brief perform conversion to floating point if necessary
template<class T, class MaxFpT = double, class =
    std::enable_if_t<std::is_arithmetic<T>::value>>
inline auto
make_fp(T x) {
    static_assert(std::is_floating_point<MaxFpT>(),
                  "expects floating point type as 2nd template parameter");

    return std::common_type_t<MaxFpT,T>(x);
}


///@brief forward, since we don't know enough about non-builtin types
template<class T, class MaxFpT = double, class =
    std::enable_if_t<!std::is_arithmetic<std::decay_t<T>>::value>>
inline auto
make_fp(T&& x) {
    static_assert(std::is_floating_point<MaxFpT>(),
                  "expects floating point type as 2nd template parameter");

    return std::forward<T>(x);
}

} //namespace detail



/*****************************************************************************
 *
 * @brief returns the value of a piece-wise constant function at position x
 *
 * @tparam Iterator  RandomAccessIterator (at least ForwardIterator)
 *                   to pairs of keys and mapped values (= nodes)
 *
 * @param begin  lower bound of node range
 * @param end    exclusive upper bound of node range
 * @param x      key for which to return the interpolated value
 *
 * @pre keys in range [begin,end) have to be sorted in ascending order
 *
 *****************************************************************************/
struct piecewise_constant
{
    template<class Iterator, class EndSentinel, class Value>
    auto operator () (const Iterator begin, const EndSentinel end,
                      const Value& x) const
    {
        using std::distance;
        using std::prev;
        using std::next;
        using std::lower_bound;

        using val_t = std::decay_t<decltype(*begin)>;
        using arg_t = std::decay_t<decltype(begin->first)>;
        using res_t = std::decay_t<decltype(begin->second)>;

        const auto n = distance(begin,end);
        if(n <  1) return res_t(0);
        if(n == 1) return begin->second;

        const auto p = lower_bound(begin,end,x,
            [](const val_t& a, const arg_t& x) {return a.first <= x;} );

        return (p != begin) ? prev(p)->second : p->second;
    }

};






/*****************************************************************************
 *
 * @brief returns the value of a piece-wise linearly interpolating function at
 *        position x
 *
 * @tparam Iterator  RandomAccessIterator (at least ForwardIterator)
 *                   to pairs of keys and mapped values (= nodes)
 *
 * @param begin  lower bound of node range
 * @param end    exclusive upper bound of node range
 * @param x      key for which to return the interpolated value
 *
 * @pre keys in range [begin,end) have to be sorted in ascending order
 *
 *****************************************************************************/
struct piecewise_linear
{
    template<class Iterator, class EndSentinel, class Value>
    auto operator () (const Iterator begin, const EndSentinel end,
                      const Value& x) const
    {
        using std::distance;
        using std::prev;
        using std::next;
        using std::lower_bound;

        using val_t = std::decay_t<decltype(*begin)>;
        using arg_t = std::decay_t<decltype(begin->first)>;
        using res_t = std::decay_t<decltype(begin->second)>;

        const auto n = distance(begin,end);
        if(n <  1) return detail::make_fp(res_t{});
        if(n == 1) return detail::make_fp(begin->second);

        auto p1 = lower_bound(begin,end,x,
            [](const val_t& a, const arg_t& x) {return a.first < x;} );

        //x smaller than left bound
        if(p1 == begin) {
            p1 = next(p1);
        }
        //x larger than right bound
        else if(p1 == end) {
            p1 = prev(p1);
        }

        const auto p0 = prev(p1);

        const auto slope = (p1->second - p0->second) /
                           detail::make_fp(p1->first - p0->first);

        return (p0->second + slope * (x - p0->first) );
    }

};






/*****************************************************************************
 *
 * @brief returns the value of a piece-wise linearly interpolating function at
 *        position log(x)
 *
 * @tparam Iterator  RandomAccessIterator (at least ForwardIterator)
 *                   to pairs of keys and mapped values (= nodes)
 *
 * @param begin  lower bound of node range
 * @param end    exclusive upper bound of node range
 * @param x      key for which to return the interpolated value
 *
 * @pre keys in range [begin,end) have to be sorted in ascending order
 *
 *****************************************************************************/
struct piecewise_log_linear
{
    template<class Iterator, class EndSentinel, class Value>
    auto operator () (const Iterator begin, const EndSentinel end,
                      const Value& x) const
    {
        using std::distance;
        using std::prev;
        using std::next;
        using std::lower_bound;

        using val_t = std::decay_t<decltype(*begin)>;
        using arg_t = std::decay_t<decltype(begin->first)>;
        using res_t = std::decay_t<decltype(begin->second)>;

        const auto n = distance(begin,end);
        if(n <  1) return detail::make_fp(res_t{});
        if(n == 1 || x <= 0) return detail::make_fp(begin->second);

        auto p1 = lower_bound(begin,end,x,
            [](const val_t& a, const arg_t& x) {return a.first < x;} );

        //x smaller than left bound
        if(p1 == begin) {
            p1 = next(p1);
        }
        //x larger than right bound
        else if(p1 == end) {
            p1 = prev(p1);
        }

        const auto p0 = prev(p1);

        const auto slope = (p1->second - p0->second) /
                           (log(p1->first / detail::make_fp(p0->first)) );

        return (p0->second + slope * (log(x / detail::make_fp(p0->first))) );
    }

};



} //namespace interpolator
} //namespace am

#endif
