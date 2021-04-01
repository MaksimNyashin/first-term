#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

static const uint32_t SHIFT = 32;
static const uint32_t HALF_SHIFT = SHIFT / 2;
static const uint32_t HALF_ONES = (1ll << HALF_SHIFT) - 1;

big_integer::big_integer() : num({0}), sign(false) {}

big_integer::big_integer(big_integer const& other) :
    num(other.num),
    sign(other.sign) {}

big_integer::big_integer(int a) :
    num({static_cast<uint32_t>(std::abs(1ll * a))}),
    sign(a < 0) {}

big_integer::big_integer(uint32_t a) :
    num({a}),
    sign(false) {}

big_integer::big_integer(std::string const& str) {
    big_integer res;
    res.sign = (str[0] == '-');
    for (size_t i = res.sign; i < str.size(); i++) {
        res *= 10;
        res += (uint32_t)(str[i] - '0');
    }
    res.sign = (str[0] == '-');
    *this = res;
}

big_integer::~big_integer() = default;

big_integer& big_integer::operator=(big_integer const& other) = default;

uint32_t addInt(uint32_t &a, uint32_t b) {
    uint32_t c = a;
    a += b;
    return b != 0 && a <= c;
}

uint32_t subInt(uint32_t &a, uint32_t b) {
    uint32_t c = a;
    a -= b;
    return b != 0 && a >= c;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    if (rhs == 0) {
        return *this;
    }
    if (sign == rhs.sign) {
        uint32_t carry = 0;
        for (size_t i = 0; i < rhs.num.size() || carry; i++) {
            if (i == num.size()) {
                num.push_back(0);
            }
            uint32_t newCarry = addInt(num[i], (i < rhs.num.size() ? rhs.num[i] : 0));
            newCarry += addInt(num[i], carry);
            carry = newCarry;
        }
    } else {
        *this -= -rhs;
    }
    remFrontZero();
    return *this;
}


big_integer& big_integer::operator-=(big_integer const& rhs) {
    if (rhs == 0) {
        return *this;
    }
    if (sign == rhs.sign) {
        if (*this == rhs) {
            return *this = big_integer(0);
        }
        if ((*this < rhs) != sign) {
            big_integer tmp = rhs;
            tmp -= *this;
            return *this = -tmp;
        }
        uint32_t carry = 0;
        for (size_t i = 0; i < rhs.num.size() || carry; i++) {
            uint32_t newCarry = subInt(num[i], (i < rhs.num.size() ? rhs.num[i] : 0));
            newCarry += subInt(num[i], carry);
            carry = newCarry;
        }
    } else {
        *this += -rhs;
    }
    remFrontZero();
    return *this;
}

std::pair<uint32_t, uint32_t> mul(uint32_t a, uint32_t b) {
    uint32_t a1 = (a >> HALF_SHIFT), b1 = (b >> HALF_SHIFT);
    uint32_t a2 = a & HALF_ONES, b2 = b & HALF_ONES;
    uint32_t fro = a1 * b1, mid = a1 * b2;
    uint32_t a2b1 = a2 * b1, bac = a2 * b2;
    uint32_t carry = addInt(mid, a2b1);
    fro += (mid >> HALF_SHIFT);
    mid &= HALF_ONES;
    mid <<= HALF_SHIFT;
    carry <<= HALF_SHIFT;
    carry += addInt(bac, mid);
    fro += carry;
    return std::make_pair(fro, bac);
}

big_integer abs(big_integer const& rhs) {
    big_integer res = rhs;
    res.sign = false;
    return res;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    size_t n = (std::max(num.size(), rhs.num.size()) + 1) / 2;
    if (n < 100000) {   
        big_integer res;
        res.num.resize(num.size() + rhs.num.size() + 1);
        res.sign = (sign != rhs.sign);
        for (size_t i = 0; i < num.size(); i++) {
            uint32_t carry = 0;
            for (size_t j = 0; j < rhs.num.size() || carry; j++) {
                if (j == rhs.num.size()) {
                    res.num[i + j] = carry;
                    break;
                }
                std::pair<uint32_t, uint32_t> par = mul(num[i], rhs.num[j]);
                par.first += addInt(par.second, carry);
                carry = par.first;
                carry += addInt(res.num[i + j], par.second);
            }
        }
        *this = res;
        remFrontZero();
        return *this;
    }
    
    n *= SHIFT;
    big_integer a = *this >> n;
    big_integer c = rhs >> n;
    big_integer b = *this - (a << n);
    big_integer d = rhs - (c << n);
    big_integer ac = a * c;
    big_integer bd = b * d;
    big_integer abcd = (a + b) * (c + d);
    big_integer mid = abcd - ac - bd;
    *this = (ac << n * 2) + (mid << n) + bd;
    remFrontZero();
    return *this;
}

uint32_t div64(uint64_t a, uint64_t b, uint64_t d) {
    uint64_t c = (a << SHIFT) + b;
    return c / d;
}

bool smaller_shift(big_integer& a, big_integer& b, size_t k, size_t l) {
    if (a.num.size() - k > b.num.size() - l) {
        return false;
    }
    for (ptrdiff_t i = b.num.size()-1; i >= 0; --i) {
        size_t q = i - l + k;
        if (a.num[q] != b.num[i]) {
            return a.num[q] < b.num[i];
        }
    }
    return false;
}

void sub_in_div(big_integer& a, big_integer const& b, size_t k, size_t l) {
    uint32_t carry = 0;
    for (size_t i = 0; i < b.num.size() || carry; i++) {
        size_t q = i - l + k;
        uint32_t newCarry = subInt(a.num[q], carry);
        newCarry += subInt(a.num[q], (i < b.num.size() ? b.num[i] : 0));
        carry = newCarry;
    }
    a.remFrontZero();
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    if (rhs == 0) {
        throw std::invalid_argument("Division by zero!");
    }
    if (num < rhs.num) {
        return *this = big_integer(0);
    }
    big_integer a = *this;
    big_integer b = rhs;
    uint32_t f = (1ULL << SHIFT) / (b.num.back() + 1);
    a *= f;
    b *= f;
    big_integer res;
    bool res_sign = (a.sign != b.sign);
    a.sign = b.sign = false;
    size_t n = a.num.size(), m = b.num.size();
    res.num.resize(n - m + 1);
    size_t ai = n;
    for (size_t i = m; i <= n; i++) {
        ai--;
        if (ai >= a.num.size()) {
            res.num[ai - m + 1] = 0;
            continue;
        }
        uint32_t zdb = div64((ai + 1 < a.num.size() ? a.num[ai + 1] : 0), a.num[ai], b.num.back());
        big_integer tmp = b * zdb;
        while (smaller_shift(a, tmp, ai, m - 1)) {
            tmp -= b;
            zdb--;
        }
        res.num[ai - m + 1] = zdb;
        sub_in_div(a, tmp, ai, m - 1);
    }
    res.sign = res_sign;
    res.remFrontZero();
    return *this = res;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    big_integer res = *this - (*this / rhs) * rhs;
    res.remFrontZero();
    return *this = res;
}

void neg_inv(big_integer& a) {
    if (!a.sign) {
        return;
    }
    for (size_t i = 0; i < a.num.size(); i++) {
        a.num[i] = ~a.num[i];
    }
    a--;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    bool new_sign = (sign & rhs.sign);
    big_integer c = big_integer(rhs);
    if (num.size() < c.num.size()) {
        num.resize(c.num.size());
    } else {
        c.num.resize(num.size());
    }
    neg_inv(*this);
    neg_inv(c);
    for (size_t i = 0; i < num.size(); i++) {
        num[i] = num[i] & c.num[i];
    }
    sign = new_sign;
    neg_inv(*this);
    remFrontZero();
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    bool new_sign = (sign | rhs.sign);
    big_integer c = big_integer(rhs);
    if (num.size() < c.num.size()) {
        num.resize(c.num.size());
    } else {
        c.num.resize(num.size());
    }
    neg_inv(*this);
    neg_inv(c);
    for (size_t i = 0; i < num.size(); i++) {
        num[i] = num[i] | c.num[i];
    }
    sign = new_sign;
    neg_inv(*this);
    remFrontZero();
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
    bool new_sign = (sign ^ rhs.sign);
    big_integer c = big_integer(rhs);
    if (num.size() < c.num.size()) {
        num.resize(c.num.size());
    } else {
        c.num.resize(num.size());
    }
    neg_inv(*this);
    neg_inv(c);
    for (size_t i = 0; i < num.size(); i++) {
        num[i] = num[i] ^ c.num[i];
    }
    sign = new_sign;
    neg_inv(*this);
    remFrontZero();
    return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
    if (rhs < 0) {
        throw std::invalid_argument("negative shift");
    }
    big_integer res;
    res.sign = (*this).sign;
    uint32_t shift = SHIFT - rhs % SHIFT;
    uint32_t start = rhs / SHIFT;
    res.num.resize(num.size() + start + 1);
    for (size_t i = 0; i < num.size(); i++) {
        uint32_t fir = num[i] >> shift;
        if (shift == SHIFT) {
            fir = 0;
        }
        uint32_t sec = num[i] & ((1ll << shift) - 1);
        res.num[i + start] += (sec << (rhs % SHIFT));
        res.num[i + start + 1] += fir;
    }
    res.remFrontZero();
    return *this = res;
}

big_integer& big_integer::operator>>=(int rhs) {
    if (rhs < 0) {
        throw std::invalid_argument("negative shift");
    }
    if ((uint32_t)rhs > num.size() * SHIFT) {
        return *this = 0;
    }
    big_integer res;
    res.sign = (*this).sign;
    uint32_t shift = rhs % SHIFT;
    uint32_t start = rhs / SHIFT;
    res.num.resize(num.size() - start + 1);
    for (size_t i = start; i < num.size(); i++) {
        uint32_t sec = num[i] >> shift;
        uint32_t fir = (i + 1 < num.size() ? num[i + 1] : 0) & ((1ll << shift) - 1);
        res.num[i - start] = (fir << (SHIFT - shift)) + sec;
    }
    bool needAdd = ((num[start] & ((1ll << shift) - 1)) > 0);
    for (size_t i = 0; i < start; i++) {
        if (num[i] > 0) {
            needAdd = true;
            break;
        }
    }
    if (res.sign && needAdd) {
        res--;
    }
    res.remFrontZero();
    return *this = res;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    if (*this == 0) {
        return *this;
    }
    big_integer r = *this;
    r.sign = !r.sign;
    r.remFrontZero();
    return r;
}

big_integer big_integer::operator~() const {
    big_integer res = big_integer(*this);
    neg_inv(res);
    for (size_t i = 0; i < res.num.size(); i++) {
        res.num[i] = ~res.num[i];
    }
    res.sign = !res.sign;
    neg_inv(res);
    res.remFrontZero();
    return res;
}

big_integer& big_integer::operator++() {
    return *this += 1;
}

big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer& big_integer::operator--() {
    return *this -= 1;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

int8_t compare(big_integer const& a, big_integer const& b) {
    int8_t sign = a.sign ? -1: 1;
    if (a.sign != b.sign) {
        return sign;
    }
    if (a.num.size() != b.num.size()) {
        return (a.num.size() < b.num.size() ? -1 : 1) * sign;
    }
    for (size_t j = 1; j <= a.num.size(); j++) {
        size_t i = a.num.size() - j;
        if (a.num[i] != b.num[i]) {
            return (a.num[i] < b.num[i] ? -1 : 1) * sign;
        }
    }
    return 0;
}

bool operator==(big_integer const& a, big_integer const& b) {
    return compare(a, b) == 0;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return compare(a, b) != 0;
}

bool operator<(big_integer const& a, big_integer const& b) {
    return compare(a, b) < 0;
}

bool operator>(big_integer const& a, big_integer const& b) {
    return compare(a, b) > 0;
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return compare(a, b) <= 0;
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return compare(a, b) >= 0;
}

std::string to_string(big_integer const& a) {
    std::string res;
    big_integer x = a;
    x.sign = false;
    while (x != 0) {
        res.push_back((char)((x % 10).num.back() + '0'));
        x /= 10;
    }
    if (res.length() == 0) {
        res = "0";
    } else if a.sign() {
        res.push_back('-');
    }
    std::reverse(res.begin(), res.end());

    return res;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}


void big_integer::remFrontZero() {
    while (num.size() > (size_t)1 && num.back() == 0) {
        num.pop_back();
    }
    if (num.back() == 0) {
        sign = false;
    }
}
