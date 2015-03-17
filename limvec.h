// Author: Divyansh Gupta

#ifndef LIMITED_VECTOR_H
#define LIMITED_VECTOR_H

#include <memory>
#include <stdexcept>
#include <algorithm>

// Base class as a wrapper around allocated memory (RAII)
template<class T, class Allocator = std::allocator<T>>
class vector_base {
public:
    Allocator alloc_;
    T* start_; //start of allocated memory
    T* finish_; //one past last element
    T* last_; //one past end of allocated memory

    vector_base(Allocator _a = Allocator()) : alloc_(_a) {
        null();
    }
    vector_base(typename Allocator::size_type _n, Allocator _a = Allocator())
    : alloc_(_a),
    start_(alloc_.allocate(_n)),
    finish_(start_),
    last_(start_ + _n) {}

    ~vector_base() {
        if(start_ != nullptr)
            alloc_.deallocate(start_, last_ - start_);
    }

    void swap_base(vector_base<T, Allocator>& _other) {
        std::swap(alloc_, _other.alloc_);
        std::swap(start_, _other.start_);
        std::swap(finish_, _other.finish_);
        std::swap(last_, _other.last_);
    }

    void null() {
        start_ = nullptr;
        finish_ = nullptr;
        last_ = nullptr;
    }
};

template<class T, class Allocator = std::allocator<T>>
class limited_vector : private vector_base<T, Allocator> {

    using vector_base<T, Allocator>::alloc_;
    using vector_base<T, Allocator>::start_;
    using vector_base<T, Allocator>::finish_;
    using vector_base<T, Allocator>::last_;

public:

    //////////////
    // TYPEDEFS //
    //////////////

    typedef Allocator allocator_type;
    typedef T value_type;
    typedef T*    pointer;
    typedef const T*    const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef typename Allocator::difference_type difference_type;
    typedef typename Allocator::size_type size_type;

    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    //////////////////////////////////
    // CONSTRUCTORS AND DESTRUCTORS //
    //////////////////////////////////

    // DEFAULT CONSTRUCTOR
    // Creates an empty vector of capacity cap
    limited_vector(size_type _cap = 0,
        const allocator_type& _a = allocator_type())
    : vector_base<T, Allocator>(_cap, _a) {}

    // INITIALIZER CONSTRUCTOR
    // Creates a vector of capacity cap and fills it with copies of _init
    limited_vector(size_type _cap, const value_type& _init,
        const allocator_type& _a = allocator_type())
    : vector_base<T, Allocator>(_cap, _a) {
        alloc_uninitialized_fill(start_, last_, _init);
        finish_ = start_ + _cap;
    }

    // COPY CONSTRUCTOR
    limited_vector(const limited_vector<T, Allocator>& _other)
    : vector_base<T, Allocator>(_other.capacity(), _other.get_allocator()) {
        alloc_uninitialized_copy(_other.begin(),  _other.end(), start_);
        finish_ = start_ + size_type(_other.end() - _other.begin());
    }

    // MOVE CONSTRUCTOR
    limited_vector(limited_vector<T, Allocator>&& _other) {
        alloc_ = _other.alloc_,
        start_ = _other.start_,
        finish_ = _other.finish_,
        last_ = _other.last_;

        _other.null();
    }

    // INITIALIZER LIST CONSTRUCTOR
    limited_vector(std::initializer_list<value_type> _il,
        const allocator_type& _a = allocator_type())
    : vector_base<T, Allocator>(_il.end() - _il.begin(), _a) {
        alloc_uninitialized_copy(_il.begin(),  _il.end(), start_);
        finish_ = start_ + size_type(_il.end() - _il.begin());
    }

    // DESTRUCTOR
    ~limited_vector() noexcept {
        clear();
    }

    ///////////////
    // ITERATORS //
    ///////////////

    iterator begin() noexcept { return start_; }
    const_iterator begin() const noexcept { return start_; }
    const_iterator cbegin() const noexcept { return start_; }

    iterator end() noexcept { return finish_; }
    const_iterator end() const noexcept { return finish_; }
    const_iterator cend() const noexcept { return finish_; }

    reverse_iterator rbegin() noexcept
    { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept
    { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept
    { return const_reverse_iterator(end()); }

    reverse_iterator rend() noexcept
    { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept
    { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept
    { return const_reverse_iterator(begin()); }

    //////////////
    // CAPACITY //
    //////////////

    size_type size() const noexcept { return (size_type(finish_ - start_)); }

    size_type capacity() const noexcept { return (size_type(last_ - start_)); }

    bool empty() const noexcept { return start_ == finish_; }

    ///////////////
    // OPERATORS //
    ///////////////

    // COPY ASSIGNMENT OPERATOR
    limited_vector& operator=(const limited_vector& _other) {
        if (this == &_other)
            return *this;

        if (capacity() < _other.size()) { // uses move assignment
            *this = limited_vector<T, Allocator>(_other);
            return *this;
        }

        if (size() > _other.size()) {
            while(size() != _other.size()) pop_back(); //destroy surplus
        }

        std::copy(_other.begin(), _other.begin() + size(), begin());
        alloc_uninitialized_copy(_other.begin() + size(), _other.end(), end());

        finish_ = start_ + _other.size();

        return *this;
    }

    // MOVE ASSIGNMENT OPERATOR
    limited_vector& operator=(limited_vector&& _other) noexcept {
        swap_base(_other);
        // _other's dtor will destroy the elements that *this previously had
        return *this;
    }

    //limited_vector(std::initializer_list<T> _il

    // No range checking
    reference operator[](size_type _n) noexcept
    { return *(start_ + _n); }

    const_reference operator[](size_type _n) const noexcept
    { return *(start_ + _n); }

    bool operator==(const limited_vector& _other) {
        return (size() == _other.size()
            && std::equal(begin(), end(), _other.begin()));
    }

    bool operator!=(const limited_vector& _other) noexcept{
        return !(*this == _other);
    }

    ///////////////
    // ACCESSORS //
    ///////////////

    // Strong exception guarantee
    reference at(size_type _n) {
        if (_n >= size())
            throw std::out_of_range("Out of vector bounds");

        return *(start_ + _n);
    }

    // Strong exception guarantee
    const_reference at(size_type _n) const {
        if (_n >= size())
            throw std::out_of_range("Out of vector bounds");

        return *(start_ + _n);
    }

    // Undefined behavior if called on an empty container
    reference front() noexcept { return *start_; }
    const_reference front() const noexcept { return *start_; }

    // Undefined behavior if called on an empty container
    reference back() noexcept { return *(finish_ - 1); }
    const_reference back() const noexcept { return *(finish_ - 1); }

    pointer data() noexcept { return start_; }
    const_pointer data() const noexcept{ return start_; }

    Allocator get_allocator() const noexcept { return alloc_; }

    ///////////////
    // MODIFIERS //
    ///////////////

    // Strong exception guarantee
    void push_back(const value_type& _value) {
        if (size() == capacity())
            throw std::overflow_error("Vector capacity reached");

        alloc_.construct(finish_, _value);
        // finish will not be updated if construct throws
        ++finish_;
    }

    // Strong exception guarantee
    void push_back(value_type&& _rvalue) {
        if (size() == capacity())
            throw std::overflow_error("Vector capacity reached");

        alloc_.construct(finish_, std::move(_rvalue));
        // Will utilize the move constructor of T, if available

        // finish will not be updated if construct throws
        ++finish_;
    }

    // Undefined behavior if called on an empty container
    void pop_back() noexcept {
        alloc_.destroy(finish_ - 1);
        --finish_;
    }

    void clear() noexcept {
        while(!empty()) pop_back();
    }

    // set_capacity can be used to explicitly set the capacity 
    // of the container. If the current size is greater than the 
    // requested capacity, then the remaining elements are destroyed
    // Strong exception guarantee
    void set_capacity(size_type _cap) {

        vector_base<T, Allocator> temp(_cap, alloc_);

        if (_cap > size()) {
            alloc_uninitialized_copy(begin(), end(), temp.start_);
            temp.finish_ = temp.start_ + size();
        }
        else {
            alloc_uninitialized_copy(begin(), begin() + _cap, temp.start_);
            temp.finish_ = temp.start_ + _cap;
        }

        clear();
        swap_base(temp);
    }

    void shrink_to_fit() {
        set_capacity(size());
    }

private:

    // Rationale: std::uninitialized_fill not used because it's not 
    // allocator aware, ie, it uses placement new and not alloc_.construct
    template<class... Args>
    void alloc_uninitialized_fill(iterator _begin, iterator _end,
        Args&&... _args) {

        iterator current;
        try {
            for(current = _begin; current != _end; ++current)
                alloc_.construct(current, _args...);
        }
        catch(...) {
            for(; _begin != current; ++_begin)
                alloc_.destroy(_begin);
            throw;
        }
    }

    template<class InputIterator>
    void alloc_uninitialized_copy(InputIterator _begin, InputIterator _end, 
        iterator _storage) {

        iterator current = _storage;
        try {
            for(; _begin != _end; ++_begin, ++current) {
                alloc_.construct(current, *_begin);
            }
        }
        catch(...) {
            for(; _storage != current; ++_storage) {
                alloc_.destroy(_storage);
            }
            throw;
        }
    }
};

#endif // LIMITED_VECTOR_H
