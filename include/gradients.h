#ifndef AMLIB_NUMERIC_MAP_GRADIENTS_H_
#define AMLIB_NUMERIC_MAP_GRADIENTS_H_

#include <type_traits>
#include <utility>

#include "interpolating_map.h"


namespace am {


/*************************************************************************//***
 *
 * @brief polymorphic interpolating function with numeric input
 *        (input is usually floating point in the range of [0,1])
 *        think 'gradient' as in 'color gradient'
 *
 *****************************************************************************/
template<class Argument, class Result>
class gradient
{
public:
    //---------------------------------------------------------------
    using argument_type = Argument;
    using result_type = Result;


    //---------------------------------------------------------------
    gradient() = default;
    gradient(const gradient&) = default;

    virtual ~gradient() {}


    //---------------------------------------------------------------
    virtual result_type operator () (argument_type) const = 0;
    virtual result_type min() const = 0;
    virtual result_type max() const = 0;

};




/*************************************************************************//***
 *
 * @brief augments an interpolating_map with a gradient interface
 *
 *****************************************************************************/
template<class Argument, class Result, class Interpolator>
class interpolating_gradient :
    public gradient<Argument,Result>
{
    using map_t = interpolating_map<Argument,Result,Interpolator>;

public:
    //---------------------------------------------------------------
    using argument_type = Argument;
    using result_type = Result;

    using const_iterator = typename map_t::const_iterator;
    using iterator       = typename map_t::iterator;
    using size_type      = typename map_t::size_type;


    //---------------------------------------------------------------
    interpolating_gradient() = default;

    template<class... Args>
    interpolating_gradient(Args&&... args):
        map_{std::forward<Args>(args)...}
    {}

    interpolating_gradient(const interpolating_gradient&) = default;
    interpolating_gradient(interpolating_gradient&&) = default;


    //---------------------------------------------------------------
    interpolating_gradient&
    operator = (const interpolating_gradient&) = default;

    interpolating_gradient&
    operator = (interpolating_gradient&&) = default;


    //---------------------------------------------------------------
    result_type
    operator () (argument_type p) const override {
        return map_(p);
    }


    //---------------------------------------------------------------
    result_type
    min() const override {
        return map_.begin()->second;
    }

    result_type
    max() const override {
        return map_.rbegin()->second;
    }


    //---------------------------------------------------------------
    size_type
    size() const {
        return map_.size();
    }


private:
    map_t map_;
};




/*****************************************************************************
 *
 *
 *****************************************************************************/
template<class Arg, class Res>
using linear_gradient =
    interpolating_gradient<Arg,Res,interpolator::piecewise_linear>;



/*****************************************************************************
 *
 *
 *****************************************************************************/
template<class Arg, class Res>
using step_gradient =
    interpolating_gradient<Arg,Res,interpolator::piecewise_constant>;


} //namespace am


#endif
