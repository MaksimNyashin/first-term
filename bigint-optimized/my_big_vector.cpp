#include "my_big_vector.h"

#include <algorithm>

my_big_vector::my_big_vector() {
    my_big_vector* an = my_big_vector::alloc(0);
    cap = an->cap;
    cnt = an->cnt;
    std::copy_n(an->data, cap, data);
}

void my_big_vector::try_del() {
	--cnt;
	if (!cnt) {
		operator delete(this);
	}
}

my_big_vector* my_big_vector::make_copy() {
	++cnt;
	return this;
}

bool my_big_vector::is_alone() {
	return cnt == 1;
}

my_big_vector* my_big_vector::alloc(size_t capacity) {
	auto res = static_cast<my_big_vector*>(operator new(sizeof(my_big_vector) + capacity * sizeof(uint32_t)));
	res->cnt = 1;
	res->cap = capacity;
	return res;
}