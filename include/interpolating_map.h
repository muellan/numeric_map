/*****************************************************************************
 *
 * AM numeric facilities
 *
 * released under MIT license
 *
 * 2008-2016 André Müller
 *
 *****************************************************************************/

#ifndef AMLIB_INTERPOLATING_MAP_H_
#define AMLIB_INTERPOLATING_MAP_H_

#include <numeric>

#include "interpolators.h"
#include "vector_map.h"


namespace am {


/*****************************************************************************
 *
 * @brief Interpolation function with a std::map like interface.
 *        Each {key,value} pair is a node (in the mathematical sense)
 *        of {domain,co-domain} values.
 *
 * @tparam KeyT          domain value type
 * @tparam MappedT       co-domain value type
 * @tparam Interpolator  function class that interpolates in-between nodes
 * @tparam KeyCompare    domain value comparison function class
 * @tparam Allocator     node allocator
 *
 *****************************************************************************/
template<
    class KeyT,
    class MappedT,
    class Interpolator,
    class KeyCompare = std::less<KeyT>,
    class Allocator = std::allocator<std::pair<const KeyT, MappedT> >
>
class interpolating_map
{
    using nodes_t_ = vector_map<KeyT,MappedT,KeyCompare,Allocator>;

public:
    //---------------------------------------------------------------
    // TYPES
    //---------------------------------------------------------------
    using key_type     = KeyT;
    using mapped_type  = MappedT;
    //-----------------------------------------------------
    using interpolator_type = Interpolator;
    using key_compare  = KeyCompare;
    //-----------------------------------------------------
    using value_type   = typename nodes_t_::value_type;
    using allocator_type = typename nodes_t_::allocator_type;
    //-----------------------------------------------------
    using reference       = typename nodes_t_::reference;
    using const_reference = typename nodes_t_::const_reference;
    using pointer       = typename nodes_t_::pointer;
    using const_pointer = typename nodes_t_::const_pointer;
    //-----------------------------------------------------
    using size_type = typename nodes_t_::size_type;
    using difference_type = typename nodes_t_::difference_type;
    //-----------------------------------------------------
    using iterator = typename nodes_t_::iterator;
    using const_iterator = typename nodes_t_::const_iterator;
    //-----------------------------------------------------
    using reverse_iterator = typename nodes_t_::reverse_iterator;
    using const_reverse_iterator = typename nodes_t_::const_reverse_iterator;

    //-----------------------------------------------------
    struct value_compare
    {
        friend class interpolating_map;

        using result_type = bool;
        using first_argument_type = value_type;
        using second_argument_type = value_type;

        bool operator () (const value_type a, const value_type& b) const {
            return keyComp_(a.first, b.first);
        }

    protected:
        value_compare(const key_compare& keyComp): keyComp_(keyComp) {}

    private:
        key_compare keyComp_;
    };


    //---------------------------------------------------------------
    // CONSTRUCTION / DESTRUCTION
    //---------------------------------------------------------------
    explicit
    interpolating_map(
        const interpolator_type& ipl = interpolator_type(),
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type())
    :
        ipl_(ipl), nodes_(comp,alloc)
    {}
    //-----------------------------------------------------
    explicit
    interpolating_map(
        const key_compare& comp,
        const allocator_type& alloc = allocator_type())
    :
        ipl_(), nodes_(comp,alloc)
    {}
    //-----------------------------------------------------
    explicit
    interpolating_map(
        const allocator_type& alloc)
    :
        ipl_(), nodes_(alloc)
    {}
    //-----------------------------------------------------
    explicit
    interpolating_map(
        std::initializer_list<value_type> il,
        const interpolator_type& ipl = interpolator_type(),
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type())
    :
        ipl_(ipl), nodes_(il,comp,alloc)
    {}
    //-----------------------------------------------------
    template <class InputIterator, class =
        std::enable_if_t<!std::is_same<value_type,InputIterator>()> >
    interpolating_map(
        InputIterator first, InputIterator last,
        const interpolator_type& ipl = interpolator_type(),
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type())
    :
        ipl_(ipl), nodes_(first,last,comp,alloc)
    {}


    //---------------------------------------------------------------
    // COPY / MOVE CONSTRUCTION
    //---------------------------------------------------------------
    interpolating_map(const interpolating_map& source):
        ipl_(source.ipl_), nodes_(source.nodes_)
    {}
    //-----------------------------------------------------
    interpolating_map(
        const interpolating_map& source, const allocator_type& alloc)
    :
        ipl_(source.ipl_), nodes_(source.nodes_,alloc)
    {}
    //-----------------------------------------------------
    interpolating_map(interpolating_map&& source) noexcept :
        ipl_(std::move(source.ipl_)), nodes_(std::move(source.nodes_))
    {}
    //-----------------------------------------------------
    interpolating_map(interpolating_map&& source, const allocator_type& alloc) noexcept :
        ipl_(std::move(source.ipl_)), nodes_(std::move(source.nodes_), alloc)
    {}


    //---------------------------------------------------------------
    // ASSIGNMENT
    //---------------------------------------------------------------
    interpolating_map&
    operator = (const interpolating_map&) = default;
    //-----------------------------------------------------
    interpolating_map&
    operator = (const interpolating_map&& source) noexcept {
        ipl_ = std::move(source.ipl_);
        nodes_ = std::move(source.nodes_);
        return *this;
    }

    //-----------------------------------------------------
    template<class InputIterator>
    void assign(InputIterator first, InputIterator last) {
        nodes_.assign(first,last);
    }
    //-----------------------------------------------------
    void assign(std::initializer_list<value_type> il) {
        nodes_.assign(il.begin(), il.end());
    }


    //---------------------------------------------------------------
    // INTERPOLATION
    //---------------------------------------------------------------
    mapped_type
    operator () (const key_type& x) const {
        return ipl_(nodes_.begin(), nodes_.end(), x);
    }
    //-----------------------------------------------------
    template<class Arg1, class Arg2, class... Args>
    mapped_type
    operator () (Arg1&& arg1, Arg2&& arg2, Args... args) const {
        return ipl_(nodes_.begin(), nodes_.end(),
                    key_type(std::forward<Arg1>(arg1),
                             std::forward<Arg2>(arg2),
                             std::forward<Args>(args)...) );
    }


    //---------------------------------------------------------------
    // ELEMENT ACCESS
    //---------------------------------------------------------------
    value_type&
    operator [] (size_type index) {
        return nodes_[index];
    }
    //-----------------------------------------------------
    const value_type&
    operator [] (size_type index) const {
        return nodes_[index];
    }

    //-----------------------------------------------------
    value_type&
    at(size_type index) {
        return nodes_.at(index);
    }
    //-----------------------------------------------------
    const value_type&
    at(size_type index) const {
        return nodes_.at(index);
    }


    //---------------------------------------------------------------
    bool
    empty() const {
        return nodes_.empty();
    }
    //-----------------------------------------------------
    size_type
    size() const {
        return nodes_.size();
    }
    //-----------------------------------------------------
    size_type
    max_size() const {
        return nodes_.max_size();
    }


    //---------------------------------------------------------------
    template<class... Args>
    iterator
    emplace(Args&&... args) {
        return nodes_.emplace(std::forward<Args>(args)...);
    }


    //---------------------------------------------------------------
    iterator
    insert(const value_type& val) {
        return nodes_.insert(val);
    }

    //-----------------------------------------------------
    template<class V>
    iterator
    insert(V&& val) {
        return nodes_.insert(std::forward<V>(val));
    }

    //-----------------------------------------------------
    template <class InputIterator>
    iterator
    insert(InputIterator first, InputIterator last) {
        return nodes_.insert(first,last);
    }
    //-----------------------------------------------------
    iterator
    insert(std::initializer_list<value_type> il) {
        return nodes_.insert(il);
    }


    //-----------------------------------------------------
    size_type
    erase(const key_type& key) {
        return nodes_.erase(key);
    }
    //-----------------------------------------------------
    iterator
    erase(const_iterator pos) {
        return nodes_.erase(pos);
    }
    //-----------------------------------------------------
    iterator
    erase(const_iterator first, const_iterator last) {
        return nodes_.erase(first,last);
    }


    //---------------------------------------------------------------
    void
    clear() {
        nodes_.clear();
    }


    //---------------------------------------------------------------
    iterator
    find(const key_type& k) {
        return nodes_.find(k);
    }
    //-----------------------------------------------------
    const_iterator
    find(const key_type& k) const {
        return nodes_.find(k);
    }


    //---------------------------------------------------------------
    iterator
    lower_bound(const key_type& k) {
        return nodes_.lower_bound(k);
    }
    //-----------------------------------------------------
    const_iterator
    lower_bound(const key_type& k) const {
        return nodes_.lower_bound(k);
    }

    //-----------------------------------------------------
    iterator
    upper_bound(const key_type& k) {
        return nodes_.upper_bound(k);
    }
    //-----------------------------------------------------
    const_iterator
    upper_bound(const key_type& k) const {
        return nodes_.upper_bound(k);
    }

    //-----------------------------------------------------
    std::pair<iterator,iterator>
    equal_range(const key_type& k) {
        return nodes_.equal_range(k);
    }
    //-----------------------------------------------------
    std::pair<const_iterator,const_iterator>
    equal_range(const key_type& k) const {
        return nodes_.equal_range(k);
    }

    //-----------------------------------------------------
    size_type
    count(const key_type& k) const {
        return nodes_.count(k);
    }


    //---------------------------------------------------------------
    const interpolator_type&
    interpolator() const {
        return ipl_;
    }
    //-----------------------------------------------------
    key_compare
    key_comp() const {
        return nodes_.key_comp();
    }
    //-----------------------------------------------------
    value_compare
    value_comp() const {
        return nodes_.value_comp();
    }

    //-----------------------------------------------------
    allocator_type
    get_allocator() const noexcept {
        return nodes_.get_allocator();
    }


    //---------------------------------------------------------------
    void
    swap(interpolating_map& other) {
        using std::swap;

        swap(ipl_, other.ipl_);
        nodes_.swap(other.nodes_);
    }


    //---------------------------------------------------------------
    // ITERATORS
    //---------------------------------------------------------------
    iterator       begin() noexcept        {return nodes_.begin(); }
    const_iterator begin() const noexcept  {return nodes_.begin(); }
    const_iterator cbegin() const noexcept {return nodes_.cbegin(); }
    //-----------------------------------------------------
    iterator       end() noexcept        {return nodes_.end(); }
    const_iterator end() const noexcept  {return nodes_.end(); }
    const_iterator cend() const noexcept {return nodes_.cend(); }
    //-----------------------------------------------------
    reverse_iterator       rbegin() noexcept        {return nodes_.rbegin(); }
    const_reverse_iterator rbegin() const noexcept  {return nodes_.rbegin(); }
    const_reverse_iterator crbegin() const noexcept {return nodes_.crbegin(); }
    //-----------------------------------------------------
    reverse_iterator       rend() noexcept        {return nodes_.rend(); }
    const_reverse_iterator rend() const noexcept  {return nodes_.rend(); }
    const_reverse_iterator crend() const noexcept {return nodes_.crend(); }


private:
    //---------------------------------------------------------------
    interpolator_type ipl_;
    nodes_t_ nodes_;

};






/*****************************************************************************
 *
 *
 *****************************************************************************/
template<
    class Key,
    class Value,
    class KeyCompare = std::less<Key>,
    class Allocator = std::allocator<std::pair<const Key,Value>>
>
using piecewise_constant_map =
        interpolating_map<Key,Value,interpolator::piecewise_constant,
                          KeyCompare,Allocator>;



/*****************************************************************************
 *
 *
 *****************************************************************************/
template<
    class Key,
    class Value,
    class KeyCompare = std::less<Key>,
    class Allocator = std::allocator<std::pair<const Key,Value>>
>
using piecewise_linear_map =
        interpolating_map<Key,Value,interpolator::piecewise_linear,
                          KeyCompare,Allocator>;



/*****************************************************************************
 *
 *
 *****************************************************************************/
template<
    class Key,
    class Value,
    class KeyCompare = std::less<Key>,
    class Allocator = std::allocator<std::pair<const Key,Value>>
>
using piecewise_log_linear_map =
        interpolating_map<Key,Value,interpolator::piecewise_log_linear,
                          KeyCompare,Allocator>;






/*****************************************************************************
 *
 * @brief free-standing swap of 2 interpolating maps
 *
 *****************************************************************************/
template<class K, class T, class I, class C, class A>
inline void
swap(interpolating_map<K,T,I,C,A>& a, interpolating_map<K,T,I,C,A>& b)
{
    a.swap(b);
}






/*****************************************************************************
 *
 * RELATIONAL OPERATORS
 *
 *****************************************************************************/
template<class K, class T, class I, class C, class A>
inline bool
operator == (const interpolating_map<K,T,I,C,A>& a,
             const interpolating_map<K,T,I,C,A>& b)
{
    return std::equal(a.begin(), a.end(), b.begin());
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline bool
operator != (const interpolating_map<K,T,I,C,A>& a,
             const interpolating_map<K,T,I,C,A>& b)
{
    return !operator==(a,b);
}



//-------------------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline bool
operator < (const interpolating_map<K,T,I,C,A>& a,
            const interpolating_map<K,T,I,C,A>& b)
{
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

//-------------------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline bool
operator <= (const interpolating_map<K,T,I,C,A>& a,
             const interpolating_map<K,T,I,C,A>& b)
{

    return operator==(a,b) || operator<(a,b);
}

//-------------------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline bool
operator > (const interpolating_map<K,T,I,C,A>& a,
            const interpolating_map<K,T,I,C,A>& b)
{

    return !operator<(a,b);
}

//-------------------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline bool
operator >= (const interpolating_map<K,T,I,C,A>& a,
             const interpolating_map<K,T,I,C,A>& b)
{

    return operator==(a,b) || operator>(a,b);
}






/*****************************************************************************
 *
 * NON-MEMBER BEGIN/END
 *
 *****************************************************************************/
template<class K, class T, class I, class C, class A>
inline decltype(auto)
begin(interpolating_map<K,T,I,C,A>& m)
{
    return m.begin();
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline decltype(auto)
begin(const interpolating_map<K,T,I,C,A>& m)
{
    return m.begin();
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline decltype(auto)
cbegin(const interpolating_map<K,T,I,C,A>& m)
{
    return m.cbegin();
}



//-------------------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline decltype(auto)
end(interpolating_map<K,T,I,C,A>& m)
{
    return m.end();
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline decltype(auto)
end(const interpolating_map<K,T,I,C,A>& m)
{
    return m.end();
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline decltype(auto)
cend(const interpolating_map<K,T,I,C,A>& m)
{
    return m.cend();
}






/*****************************************************************************
 *
 * STATISTICS
 *
 *****************************************************************************/
template<class K, class T, class I, class C, class A>
inline auto
min(const interpolating_map<K,T,I,C,A>& in)
{
    return *std::min_element(in.begin(), in.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline auto
max(const interpolating_map<K,T,I,C,A>& in)
{
    return *std::max_element(in.begin(), in.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline auto
total(const interpolating_map<K,T,I,C,A>& in)
{
    return *std::accumulate(in.begin(), in.end(), T(0),
        [](const auto& a, const auto& b) { return a.second + b.second; });
}

//---------------------------------------------------------
template<class K, class T, class I, class C, class A>
inline auto
mean(const interpolating_map<K,T,I,C,A>& in)
{
    return total(in) /
        typename interpolating_map<K,T,I,C,A>::mapped_type(in.size());
}


} //namespace am


#endif
