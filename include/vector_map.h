/*****************************************************************************
 *
 * AM containers
 *
 * released under MIT license
 *
 * 2008-2017 André Müller
 *
 *****************************************************************************/

#ifndef AMLIB_VECTOR_MAP_H_
#define AMLIB_VECTOR_MAP_H_


#include <algorithm>
#include <utility>
#include <functional>
#include <vector>


namespace am {


/*************************************************************************//***
 *
 * @brief key -> value (multi)map
 *        guarantees that all keys are held in one contiguous chunk of memory
 *        thus enabling cache-friendly random access to key-value pairs
 *
 * @details
 *     random element access in O(1)
 *     element search in O(log(n))
 *     insert in O(n)
 *     erase  in O(n)
 *
 *
 *****************************************************************************/
template<
    class KeyT,
    class MappedT,
    class KeyCompare = std::less<KeyT>,
    class Allocator = std::allocator<std::pair<KeyT,MappedT> >
>
class vector_map
{
    using mem_t_ = std::vector<typename std::pair<KeyT,MappedT>,Allocator>;

public:
    //---------------------------------------------------------------
    // TYPES
    //---------------------------------------------------------------
    using key_type     = KeyT;
    using mapped_type  = MappedT;
    using key_compare  = KeyCompare;
    //-----------------------------------------------------
    using value_type   = typename mem_t_::value_type;
    using allocator_type = typename mem_t_::allocator_type;
    //-----------------------------------------------------
    using reference       = typename mem_t_::reference;
    using const_reference = typename mem_t_::const_reference;
    using pointer       = typename mem_t_::pointer;
    using const_pointer = typename mem_t_::const_pointer;
    //-----------------------------------------------------
    using size_type = typename mem_t_::size_type;
    using difference_type = typename mem_t_::difference_type;
    //-----------------------------------------------------
    using iterator = typename mem_t_::iterator;
    using const_iterator = typename mem_t_::const_iterator;
    //-----------------------------------------------------
    using reverse_iterator = typename mem_t_::reverse_iterator;
    using const_reverse_iterator = typename mem_t_::const_reverse_iterator;

    //-----------------------------------------------------
    struct value_compare
    {
        friend class vector_map;

        using result_type = bool;
        using first_argument_type = value_type;
        using second_argument_type = key_type;

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
    vector_map(
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type())
    :
        comp_(comp), mem_(alloc)
    {}
    //-----------------------------------------------------
    explicit
    vector_map(
        const allocator_type& alloc)
    :
        comp_(key_compare()), mem_(alloc)
    {}
    //-----------------------------------------------------
    explicit
    vector_map(
        std::initializer_list<value_type> il,
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type())
    :
        comp_(comp), mem_(alloc)
    {
        insert(il);
    }
    //-----------------------------------------------------
    template<class InputIterator, class =
        std::enable_if_t<!std::is_same<value_type,InputIterator>()> >
    vector_map(
        InputIterator first, InputIterator last,
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type())
    :
        comp_(comp), mem_(alloc)
    {
        insert(first,last);
    }


    //---------------------------------------------------------------
    // COPY / MOVE CONSTRUCTION
    //---------------------------------------------------------------
    vector_map(const vector_map& source):
        comp_(source.comp_), mem_(source.mem_)
    {}
    //-----------------------------------------------------
    vector_map(const vector_map& source, const allocator_type& alloc):
        comp_(source.comp_), mem_(source.mem_,alloc)
    {}
    //-----------------------------------------------------
    vector_map(vector_map&& source) noexcept :
        comp_(std::move(source.comp_)), mem_(std::move(source.mem_))
    {}
    //-----------------------------------------------------
    vector_map(vector_map&& source, const allocator_type& alloc) noexcept :
        comp_(std::move(source.comp_)), mem_(std::move(source.mem_), alloc)
    {}


    //---------------------------------------------------------------
    // aASSIGNMENT
    //---------------------------------------------------------------
    vector_map&
    operator = (const vector_map&) = default;
    //-----------------------------------------------------
    vector_map&
    operator = (const vector_map&& source) noexcept {
        comp_ = std::move(source.comp_);
        mem_ = std::move(source.mem_);
        return *this;
    }

    //-----------------------------------------------------
    template<class InputIterator>
    void assign(InputIterator first, InputIterator last) {
        mem_.clear();
        insert(first,last);
    }
    //-----------------------------------------------------
    void assign(std::initializer_list<value_type> il) {
        assign(il.begin(), il.end());
    }


    //---------------------------------------------------------------
    // ELEMENT ACCESS
    //---------------------------------------------------------------
    const value_type&
    operator [] (size_type index) const noexcept {
        return mem_[index];
    }

    //-----------------------------------------------------
    const value_type&
    at(size_type index) const {
        return mem_.at(index);
    }

    //-----------------------------------------------------
    const value_type&
    front() const noexcept {
        return mem_.front();
    }

    //-----------------------------------------------------
    const value_type&
    back() const noexcept {
        return mem_.back();
    }


    //---------------------------------------------------------------
    bool
    empty() const noexcept {
        return mem_.empty();
    }
    //-----------------------------------------------------
    size_type
    size() const noexcept {
        return mem_.size();
    }
    //-----------------------------------------------------
    size_type
    max_size() const noexcept {
        return mem_.max_size();
    }


    //---------------------------------------------------------------
    void
    reserve(size_type size) {
        mem_.reserve(size);
    }


    //---------------------------------------------------------------
    template<class... Args>
    const_iterator
    emplace(Args&&... args) {
        return insert(value_type(std::forward<Args>(args)...));
    }


    //---------------------------------------------------------------
    const_iterator
    insert(const value_type& val) {
        const auto pos = lower_bound(val.first);
        return mem_.insert(pos, val);
    }

    //-----------------------------------------------------
    const_iterator
    insert(value_type&& val) {
        const auto pos = lower_bound(val.first);
        return mem_.insert(pos, std::move(val));
    }

    //-----------------------------------------------------
    template <class InputIterator>
    const_iterator
    insert(InputIterator first, InputIterator last) {
        using std::distance;

        mem_.reserve(mem_.size() + distance(first,last));

        auto it = mem_.cend();
        for(; first != last; ++first) {
            it = insert(*first);
        }

        return it;
    }
    //-----------------------------------------------------
    const_iterator
    insert(std::initializer_list<value_type> il) {
        return insert(il.begin(), il.end());
    }


    //-----------------------------------------------------
    size_type
    erase(const key_type& key) {
        using std::distance;

        const auto er = equal_range(key);

        if(er.first < mem_.cend()) {
            mem_.erase(er.first, er.second);
            return distance(er.first, er.second);
        } else {
            return 0;
        }
    }
    //-----------------------------------------------------
    const_iterator
    erase(const_iterator pos) {
        return mem_.erase(pos);
    }
    //-----------------------------------------------------
    const_iterator
    erase(const_iterator first, const_iterator last) {
        return mem_.erase(first,last);
    }


    //---------------------------------------------------------------
    void
    clear() {
        mem_.clear();
    }


    //---------------------------------------------------------------
    const_iterator
    find(const key_type& k) const {
        const auto it = lower_bound(k);
        return (it->first == k) ? it : mem_.end();
    }


    //---------------------------------------------------------------
    const_iterator
    lower_bound(const key_type& k) const {
        return lower_bound( mem_.begin(), mem_.end(), k);
    }

    //-----------------------------------------------------
    const_iterator
    upper_bound(const key_type& k) const {
        return upper_bound(mem_.begin(), mem_.end(), k);
    }

    //-----------------------------------------------------
    std::pair<const_iterator,const_iterator>
    equal_range(const key_type& k) const {
        return equal_range(mem_.begin(), mem_.end(), k);
    }

    //-----------------------------------------------------
    size_type
    count(const key_type& k) const {
        using std::distance;
        const auto r = equal_range(k);
        return distance(r.first, r.second);
    }


    //---------------------------------------------------------------
    key_compare
    key_comp() const noexcept {
        return comp_.keyComp_;
    }
    //-----------------------------------------------------
    value_compare
    value_comp() const noexcept {
        return comp_;
    }

    //-----------------------------------------------------
    allocator_type
    get_allocator() const noexcept {
        return mem_.get_allocator();
    }


    //---------------------------------------------------------------
    void
    swap(vector_map& other) noexcept {
        using std::swap;

        swap(comp_, other.comp_);
        mem_.swap(other.mem_);
    }


    //---------------------------------------------------------------
    // ITERATORS
    //---------------------------------------------------------------
    const_iterator begin() const  noexcept {return mem_.begin(); }
    const_iterator end() const    noexcept {return mem_.end(); }
    //-----------------------------------------------------
    const_reverse_iterator rbegin() const  noexcept {return mem_.rbegin(); }
    const_reverse_iterator rend() const    noexcept {return mem_.rend(); }


private:
    //---------------------------------------------------------------
    //we need our own versions of upper_bound lower_bound etc.
    //we can't use the std:: algorithms because this would lead to
    //the requirement that the mapped type had to be default constructible
    template <class Iter>
    static Iter
    lower_bound(Iter first, Iter last, const key_type& key)
    {
        difference_type count = distance(first,last);
        difference_type step = 0;

        Iter it;
        while(count > 0) {
          it = first;
          step = count / 2;
          it += step;
          if(it->first < key) {
              first = ++it;
              count -= step + 1;
          }
          else count = step;
        }
        return first;
    }
    //-----------------------------------------------------
    template <class Iter, class T>
    static Iter
    upper_bound (Iter first, Iter last, const key_type& key)
    {
        difference_type count = distance(first,last);
        difference_type step = 0;

        Iter it;
        while(count > 0) {
            it = first;
            step = count / 2;
            it += step;
            if(!(key < it->first)) {
                first = ++it;
                count -= step + 1;
            }
            else count = step;
        }
        return first;
    }
    //-----------------------------------------------------
    template <class Iter, class T>
    static std::pair<Iter,Iter>
    equal_range(Iter first, Iter last, const key_type& key)
    {
        auto it = lower_bound(first,last,key);
        return std::make_pair(it, upper_bound(it,last,key) );
    }

    //---------------------------------------------------------------
    value_compare comp_;
    mem_t_ mem_;

};






/*****************************************************************************
 *
 * MODIFICATION
 *
 *****************************************************************************/
template<class K, class T, class C, class A>
inline void
swap(vector_map<K,T,C,A>& a, vector_map<K,T,C,A>& b) noexcept
{
    a.swap(b);
}






/*****************************************************************************
 *
 * RELATIONAL OPERATORS
 *
 *****************************************************************************/
template<class K, class T, class C, class A>
inline bool
operator == (const vector_map<K,T,C,A>& a, const vector_map<K,T,C,A>& b) noexcept
{
    return std::equal(a.begin(), a.end(), b.begin());
}

template<class K, class T, class C, class A>
inline bool
operator != (const vector_map<K,T,C,A>& a, const vector_map<K,T,C,A>& b) noexcept
{
    return !operator==(a,b);
}



template<class K, class T, class C, class A>
inline bool
operator < (const vector_map<K,T,C,A>& a, const vector_map<K,T,C,A>& b) noexcept
{
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

template<class K, class T, class C, class A>
inline bool
operator <= (const vector_map<K,T,C,A>& a, const vector_map<K,T,C,A>& b) noexcept
{
    return operator==(a,b) || operator<(a,b);
}

template<class K, class T, class C, class A>
inline bool
operator > (const vector_map<K,T,C,A>& a, const vector_map<K,T,C,A>& b) noexcept
{
    return !operator<(a,b);
}

template<class K, class T, class C, class A>
inline bool
operator >= (const vector_map<K,T,C,A>& a, const vector_map<K,T,C,A>& b) noexcept
{
    return operator==(a,b) || operator>(a,b);
}






/*****************************************************************************
 *
 * NON-MEMBER BEGIN/END
 *
 *****************************************************************************/
template<class K, class T, class C, class A>
inline auto
begin(vector_map<K,T,C,A>& m) noexcept {
    return m.begin();
}

template<class K, class T, class C, class A>
inline auto
begin(const vector_map<K,T,C,A>& m) noexcept {
    return m.begin();
}

template<class K, class T, class C, class A>
inline auto
cbegin(const vector_map<K,T,C,A>& m) noexcept {
    return m.cbegin();
}



//-------------------------------------------------------------------
template<class K, class T, class C, class A>
inline auto
end(vector_map<K,T,C,A>& m) noexcept {
    return m.end();
}

template<class K, class T, class C, class A>
inline auto
end(const vector_map<K,T,C,A>& m) noexcept {
    return m.end();
}

template<class K, class T, class C, class A>
inline auto
cend(const vector_map<K,T,C,A>& m) noexcept {
    return m.cend();
}


} //namespace am


#endif
