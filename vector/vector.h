#ifndef MY_VECTOR_H
#define MY_VECTOR_H

#include <cstddef>
#include <algorithm>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;

    vector();                               // O(1) nothrow
    vector(vector const&);                  // O(N) strong
    vector& operator=(vector const&);       // O(N) strong

    ~vector();                              // O(N) nothrow

    T& operator[](size_t i);                // O(1) nothrow
    T const& operator[](size_t) const;      // O(1) nothrow

    T* data();                              // O(1) nothrow
    T const* data() const;                  // O(1) nothrow
    size_t size() const;                    // O(1) nothrow

    T& front();                             // O(1) nothrow
    T const& front() const;                 // O(1) nothrow

    T& back();                              // O(1) nothrow
    T const& back() const;                  // O(1) nothrow
    void push_back(T const&);               // O(1)* strong
    void pop_back();                        // O(1) nothrow

    bool empty() const;                     // O(1) nothrow

    size_t capacity() const;                // O(1) nothrow
    void reserve(size_t);                   // O(N) strong
    void shrink_to_fit();                   // O(N) strong

    void clear();                           // O(N) nothrow

    void swap(vector&);                     // O(1) nothrow

    iterator begin();                       // O(1) nothrow
    iterator end();                         // O(1) nothrow

    const_iterator begin() const;           // O(1) nothrow
    const_iterator end() const;             // O(1) nothrow

    // iterator insert(iterator pos, T const&); // O(N) weak
    iterator insert(const_iterator, T const&);  // O(N) weak

    // iterator erase(iterator pos);           // O(N) weak
    iterator erase(const_iterator);         // O(N) weak

    // iterator erase(iterator first, iterator last);  // O(N) weak
    iterator erase(const_iterator first, const_iterator last);  // O(N) weak

private:
    void new_cap(size_t);
    static iterator copy_data(const_iterator, size_t, size_t);
    static void destroy_data(const_iterator, size_t, size_t);
    size_t get_new_cap();

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};

template <typename T>
vector<T>::vector() : data_(nullptr), size_(0), capacity_(0) {}

template <typename T>
vector<T>::vector(vector const& other) {
    data_ = nullptr;
    if (!other.empty()) {
        data_ = copy_data(other.data_, other.size_, other.size_);
    }
    size_ = other.size_;
    capacity_ = other.size_;
}

template <typename T>
vector<T>& vector<T>::operator=(vector<T> const& other) {
    if (this != &other) {
        vector(other).swap(*this);
    }
    return *this;
}

template <typename T>
vector<T>::~vector() {
    clear();
    operator delete(data_);
}

template <typename T>
T& vector<T>::operator[](size_t ind) {
    return data_[ind];
}

template <typename T>
T const& vector<T>::operator[](size_t ind) const {
    return data_[ind];
}

template <typename T>
T* vector<T>::data() {
    return data_;
}

template <typename T>
T const* vector<T>::data() const {
    return data_;
}

template <typename T>
size_t vector<T>::size() const {
    return size_;
}

template <typename T>
T& vector<T>::front() {
    return data_[0];
}

template <typename T>
T const& vector<T>::front() const {
    return data_[0];
}

template <typename T>
T& vector<T>::back() {
    return data_[size_ - 1];
}

template <typename T>
T const& vector<T>::back() const {
    return data_[size_ - 1];
}

template <typename T>
void vector<T>::push_back(const T& x) {
    if (size_ != capacity_) {
        new(data_ + size_) T(x);
    } else {
        T tmp(x);
        reserve(get_new_cap());
        new(data_ + size_) T(tmp);
    }
    ++size_;
}

template <typename T>
void vector<T>::pop_back() {
    --size_;
    data_[size_].~T();
}

template <typename T>
bool vector<T>::empty() const {
    return (size_ == 0);
}

template <typename T>
size_t vector<T>::capacity() const {
    return capacity_;
}

template <typename T>
void vector<T>::reserve(size_t s) {
    if (s > capacity_) {
        new_cap(s);
    }
}


template <typename T>
void vector<T>::shrink_to_fit() {
    if (capacity_ != size_) {
        if (size_ == 0) {
            operator delete(data_);
            data_ = nullptr;
        } else {
            new_cap(size_);
        }
    }
}


template <typename T>
void vector<T>::clear() {
    destroy_data(data_, size_, size_);
    size_ = 0;
}

template <typename T>
void vector<T>::swap(vector<T>& other) {
    std::swap(data_, other.data_);
    std::swap(capacity_, other.capacity_);
    std::swap(size_, other.size_);
}

template <typename T>
typename vector<T>::iterator vector<T>::begin() {
    return data_;
}

template <typename T>
typename vector<T>::iterator vector<T>::end() {
    return data_ + size_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
    return data_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::end() const {
    return data_ + size_;
}

template <typename T>
typename vector<T>::iterator vector<T>::insert(const_iterator pos, T const& x) {
    std::ptrdiff_t cur = pos - data_;
    push_back(x);
    for (size_t i = size_ - 1; i != cur; --i) {
        std::swap(data_[i], data_[i - 1]);
    }
    return data_ + cur;
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
    return erase(pos, pos + 1);
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last) {
    std::ptrdiff_t f_ind = first - begin();
    if (last <= first) {
        return data_ + f_ind;
    }
    std::ptrdiff_t l_ind = last - begin();
    for (size_t i = f_ind, j = l_ind; j != size_; ++i, ++j) {
        std::swap(data_[i], data_[j]);
    }
    destroy_data(data_, l_ind - f_ind, size_);
    size_ -= (l_ind - f_ind);
    return data_ + f_ind;
}

template <typename T>
typename vector<T>::iterator vector<T>::copy_data(const_iterator data, size_t old_size, size_t new_size) {
    size_t i;
    iterator buf = static_cast<T*>(operator new(new_size * sizeof(T)));
    try {
        for (i = 0; i < old_size; ++i) {
            new(buf + i) T(data[i]);
        }
    } catch (...) {
        destroy_data(buf, i, i);
        operator delete(buf);
        throw;
    }
    return buf;
}

template <typename T>
void vector<T>::destroy_data(const_iterator data, size_t cnt, size_t last_ind) {
    while (cnt) {
        --last_ind;
        data[last_ind].~T();
        --cnt;
    }
}

template <typename T>
size_t vector<T>::get_new_cap() {
    return size_ * 2 + 1;
}

template <typename T>
void vector<T>::new_cap(size_t n) {
    T* new_data = copy_data(data_, size_, n);
    destroy_data(data_, size_, size_);
    std::swap(data_, new_data);
    capacity_ = n;
}

#endif  // MY_VECTOR_H