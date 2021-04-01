#include "my_vector.h"

#include <algorithm>


size_t get_new_cap(size_t cap) {
	return cap == 0 ? 1 : (cap * 2);
}

my_vector::my_vector() : my_size(0) {}

my_vector::my_vector(std::vector<uint32_t> vec) : my_size(vec.size()){
	set_flag(vec.size() > SIZE);
	if (is_small()) {
		std::copy_n(vec.begin(), size(), small);
	} else {
		big = my_big_vector::alloc(size());
		std::copy_n(vec.begin(), size(), big->data);
	}
}

my_vector::my_vector(my_vector const& other) : my_size(other.my_size) {
	if (is_small()) {
		std::copy_n(other.small, my_size, small);
	} else {
		big = other.big->make_copy();
	}
}

my_vector& my_vector::operator=(my_vector const& other) {
	if (this != &other) {
		if (!is_small()) {
			big->try_del();
		}
		my_size = other.my_size;
		if (other.is_small()) {
			std::copy_n(other.small, my_size, small);
		} else {
			big = other.big->make_copy();
		}
	}
	return *this;
}

my_vector::~my_vector() {
	if (!is_small()) {
		big->try_del();
	}
}

void my_vector::push_back(uint32_t x) {
	resize(size() + 1);
	back() = x;
	
}

void my_vector::pop_back() {
	if  (!is_small()) {
		dupl();
	}
	--my_size;
}

uint32_t& my_vector::back() {
	dupl();
	return is_small() ? small[size() - 1] : big->data[size() - 1];
}

uint32_t const& my_vector::back() const {
	return is_small() ? small[size() - 1] : big->data[size() - 1];
}

void my_vector::resize(size_t new_size) {
    size_t sz = size();
	if (is_small()) {
		if (new_size <= SIZE) {
			if (sz < new_size) {
				std::fill(small + sz, small + new_size, 0);
			}
			my_size = new_size;
		} else {
			my_big_vector* new_big = my_big_vector::alloc(std::max(get_new_cap(SIZE), new_size));
			std::copy_n(small, SIZE, new_big->data);
			big = new_big;
			std::fill(big->data + sz, big->data + new_size, 0);
			my_size = new_size;
			set_flag(true);
		}
	} else {
		if (new_size > sz) {
			if (new_size > big->cap) {
				dupl_cap(std::max(get_new_cap(big->cap), new_size));
			} else {
				dupl();
			}
			std::fill(big->data + sz, big->data + new_size, 0);
		}
		my_size = new_size;
		set_flag(true);
	}
}

size_t my_vector::size() const {
	return my_size & (FLAG_MASK - 1);
}

uint32_t& my_vector::operator[](size_t ind) {
	if (is_small()) {
		return small[ind];
	} else {
		dupl();
		return big->data[ind];
	}
}

uint32_t const& my_vector::operator[](size_t ind) const {
	if (is_small()) {
		return small[ind];
	} else {
		return big->data[ind];
	}
}


bool my_vector::is_small() const {
	return !static_cast<bool>(my_size & FLAG_MASK);
}

void my_vector::set_flag(bool new_flag) {
	if (new_flag) {
		my_size |= FLAG_MASK;
	} else {
		my_size &= (FLAG_MASK - 1);
	}
}

void my_vector::dupl_cap(size_t cap) {
	if (cap > big->cap || !big->is_alone()) {
		my_big_vector* new_ = my_big_vector::alloc(cap);
		std::copy_n(big->data, size(), new_->data);
		big->try_del();
		big = new_;
	}
}

void my_vector::dupl() {
	if (!is_small()) {
		dupl_cap(big->cap);
	}
}