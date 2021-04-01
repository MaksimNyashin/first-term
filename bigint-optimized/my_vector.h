#ifndef BIGINT_MY_VECTOR_H
#define BIGINT_MY_VECTOR_H

#include "my_big_vector.h"

#include <vector>

class my_vector {
private:
	static constexpr size_t SIZE = sizeof(my_big_vector*) / sizeof(uint32_t);
	static constexpr uint32_t FLAG_MASK = (1ul <<  (sizeof(uint32_t) * 8 - 1));
	size_t my_size = 0;
	union {
		uint32_t small[SIZE];
		my_big_vector *big;
	};
	void dupl();
	void dupl_cap(size_t);
	bool is_small() const;
	void set_flag(bool);

public:
	my_vector();
	explicit my_vector(std::vector<uint32_t>);
	// my_vector(size_t s);
	my_vector(my_vector const&);
	~my_vector();
	my_vector& operator=(my_vector const&);

	size_t size() const;
	uint32_t const& back() const;
	uint32_t& back();
	void push_back(uint32_t);
	void pop_back();
	void resize(size_t);
	uint32_t const& operator[](size_t) const;
	uint32_t& operator[](size_t);
};

#endif  // BIGINT_MY_VECTOR_H