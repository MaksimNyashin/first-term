#ifndef BIGINT_MY_BIG_VECTOR_H
#define BIGINT_MY_BIG_VECTOR_H

#include <cstdint>
#include <cstddef>

struct my_big_vector {
	size_t cnt;
	size_t cap;
	uint32_t data[];

	my_big_vector();
	void try_del();
	my_big_vector* make_copy();
	bool is_alone();
	static my_big_vector* alloc(size_t);
};

#endif // BIGINT_MY_BIG_VECTOR_H