#include "big_integer.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <utility>

constexpr uint32_t chunk_size = 32;
constexpr uint64_t base = 10;

big_integer::big_integer() : _sign(false) {
  _n.push_back(0);
}

big_integer::big_integer(const big_integer& other) : _sign(other._sign) {
  _n.resize(other._n.size());
  std::copy(other._n.begin(), other._n.end(), _n.begin());
}

big_integer::big_integer(short a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned short a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned int a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned long a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long long a) : _sign(a < 0) {
  auto x = static_cast<unsigned long long>(abs(a));
  _n.push_back(x);
  if (x >> chunk_size) {
    _n.push_back(x >> chunk_size);
  }
}

big_integer::big_integer(unsigned long long a) : _sign(false) {
  _n.push_back(a);
  if (a >> chunk_size) {
    _n.push_back(a >> chunk_size);
  }
}

big_integer::big_integer(const std::string& str)
  : _sign(false)
{
  *this = 0;
  if (str.empty()) {
    throw std::invalid_argument("Expected number while initializing big_integer, empty string found.");
  }
  bool sign = str[0] == '-';
  if (sign && str.length() == 1) {
    throw std::invalid_argument("Expected valid number while initializing big_integer, single dash found.");
  }
  if (std::any_of(str.begin()+sign, str.end(), [] (char x) { return !std::isdigit(x); })) {
    throw std::invalid_argument("Met invalid character while initializing big_integer from string.");
  }
//  std::for_each()
  for (auto it = str.begin() + sign; it < str.end(); ++it) {
    (*this *= base) += static_cast<uint32_t>(*it - '0');
  }
  _sign = !(*this == 0) && sign;
}

big_integer::~big_integer() = default;

void big_integer::ensure_size(size_t n) {
  while (_n.size() < n) {
    _n.push_back(0);
  }
}

void big_integer::shrink_to_fit() {
  while (_n.size() > 1 && !_n.back()) {
    _n.pop_back();
  }
}

void big_integer::swap(big_integer& other) {
  std::swap(_n, other._n);
  std::swap(_sign, other._sign);
}

big_integer& big_integer::operator=(const big_integer& other) {
  if (this != &other) {
    big_integer tmp(other);
    swap(tmp);
  }
  return *this;
}

bool big_integer::abs_less(const big_integer& other) const {
  size_t size = _n.size();
  if (size != other._n.size()) {
    return size < other._n.size();
  }
  for (size_t i = 1, index = size - i; i <= size; ++i, index = size - i) {
    if (_n[index] != other._n[index]) {
      return _n[index] < other._n[index];
    }
  }
  return false;
}

bool big_integer::abs_eq(const big_integer& other) const {
  return (_n.size() == other._n.size()) && std::equal(_n.begin(), _n.end(), other._n.begin());
}

big_integer& big_integer::convert() {
  if (*this != 0 && _sign) {
    bool carry = true;
    std::for_each(_n.begin(), _n.end(), [&carry](uint32_t& x) {
      uint64_t res = ~x;
      res += carry;
      x = res;
      carry = res >> chunk_size;
    });
  }
  return *this;
}

uint64_t eval_quotient(const big_integer& divisible, const big_integer& divider) {
  uint64_t l = 0, r = (UINT64_MAX >> chunk_size) + 1;
  while (r - l > 1) {
    uint32_t m = (r - l) / 2 + l;
    if (big_integer(m) * divider <= divisible) {
      l = m;
    } else {
      r = m;
    }
  }
  return l;
}

big_integer big_integer::divide(const big_integer& other) {
  if (abs_less(other)) {
    return 0;
  }
  auto divider_size = static_cast<vec::difference_type>(other._n.size());
  int64_t it = static_cast<int64_t>(_n.size()) - divider_size;
  big_integer result(vec(it + 1));
  while (it >= 0) {
    big_integer caret(vec(_n.begin() + it, _n.end()));
    caret.shrink_to_fit();
    uint64_t quotient = eval_quotient(caret, other);
    result._n[it] = quotient;
    big_integer repl(caret - quotient * other);
    repl.ensure_size(_n.end() - _n.begin() - it);
    std::copy(repl._n.begin(), repl._n.end(), _n.begin() + it);
    --it;
  }
  shrink_to_fit();
  result.shrink_to_fit();
  return result;
}

big_integer::big_integer(vec other) : _n(std::move(other)), _sign(false) {}

template <typename F>
big_integer& big_integer::binary_bit_operation(const big_integer& other, const F& f) {
  bool less = abs_less(other);
  const big_integer &greatest = less ? other : *this, smallest = less ? *this : other;
  vector_bit_f(greatest, smallest, *this, f);
  return *this;
}

template <typename F>
void vector_bit_f(const big_integer& a, const big_integer& b, big_integer& result, const F& f) {
  bool a_carry = a._sign, b_carry = b._sign;
  for (size_t i = 0; i < a._n.size(); ++i) {
    uint64_t converted_a_i = (a._sign ? ~a._n[i] : a._n[i]);
    converted_a_i += a_carry;
    uint64_t converted_b_i = i < b._n.size() ? (b._sign ? ~b._n[i] : b._n[i]) : (b._sign ? UINT32_MAX : 0);
    converted_b_i += b_carry;
    a_carry = converted_a_i >> chunk_size;
    b_carry = converted_b_i >> chunk_size;
    uint64_t res = f(converted_a_i, converted_b_i);
    if (i < result._n.size()) {
      result._n[i] = res;
    } else {
      result._n.push_back(res);
    }
  }
  result._sign = f(a._sign, b._sign);
  result.convert();
  result.shrink_to_fit();
}

template <typename F>
void vector_f(big_integer::const_vec_ref a, big_integer::const_vec_ref b, big_integer::vec_ref result, const F& f) {
  bool carry = false;
  for (size_t i = 0; i < a.size(); ++i) {
    int64_t res = f(f(a[i], (i < b.size() ? b[i] : 0)), carry);
    carry = (res > UINT32_MAX) || (res < 0);
    if (i < result.size()) {
      result[i] = res;
    } else {
      result.push_back(res);
    }
  }
  if (carry) {
    result.push_back(carry);
  }
}

big_integer& big_integer::operator+=(const big_integer& other) {
  bool less = abs_less(other);
  big_integer::const_vec_ref greatest = less ? other._n : _n, smallest = less ? _n : other._n;
  if (_sign == other._sign) {
    vector_f(greatest, smallest, _n, std::plus<int64_t>());
  } else {
    vector_f(greatest, smallest, _n, std::minus<int64_t>());
  }
  if (less) {
    _sign = other._sign;
  }
  if (*this == 0) {
    _sign = false;
  }
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator-=(const big_integer& other) {
  return *this += -other;
}

big_integer& big_integer::operator*=(const big_integer& other) {
  auto this_it = static_cast<int64_t>(_n.size());
  ensure_size(_n.size() + other._n.size());
  while (--this_it >= 0) {
    uint64_t carry = 0;
    uint64_t multiplier = _n[this_it];
    _n[this_it] = 0;
    size_t other_it = 0;
    while (other_it < other._n.size() || carry) {
      const uint64_t result =
          (other_it < other._n.size() ? other._n[other_it] * multiplier : 0) + _n[this_it + other_it] + carry;
      _n[this_it + other_it] = result;
      carry = result >> chunk_size;
      other_it++;
    }
  }
  _sign = _sign ^ other._sign;
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator/=(const big_integer& other) {
  bool sign = _sign ^ other._sign;
  auto divider(other);
  divider._sign = false;
  _sign = false;
  *this = divide(divider);
  _sign = sign;
  return *this;
}

big_integer& big_integer::operator%=(const big_integer& other) {
  bool sign = _sign;
  auto divider(other);
  divider._sign = false;
  _sign = false;
  divide(divider);
  _sign = sign;
  return *this;
}

big_integer& big_integer::operator&=(const big_integer& other) {
  return binary_bit_operation(other, std::bit_and());
}

big_integer& big_integer::operator|=(const big_integer& other) {
  return binary_bit_operation(other, std::bit_or());
}

big_integer& big_integer::operator^=(const big_integer& other) {
  return binary_bit_operation(other, std::bit_xor());
}

big_integer& big_integer::operator<<=(int other) {
  uint64_t shift = other / chunk_size;
  ensure_size(_n.size() + shift + 2);
  convert();
  for (int64_t i = _n.size() - 1; i >= 0; --i) {
    _n[i] = i - shift < 0 ? 0 : _n[i - shift];
  }
  shift = other % chunk_size;
  if (shift > 0) {
    for (int64_t i = _n.size() - 2; i >= other / chunk_size; --i) {
      _n[i + 1] = (_n[i + 1] << shift) | (_n[i] >> (chunk_size - shift));
    }
  }
  _n[other / chunk_size] <<= shift;
  for (size_t i = 0; i < other / chunk_size; ++i) {
    _n[i] = 0;
  }
  convert();
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator>>=(int other) {
  if (_n.size() * chunk_size < other) return _sign ? *this = -1 : *this = 0;
  uint64_t shift = other / chunk_size;
  ensure_size(_n.size() + 2);
  convert();
  for (size_t i = 0; i < _n.size(); ++i) {
    _n[i] = i < _n.size() - shift ? _n[i + shift] : (_sign ? UINT32_MAX : 0);
  }
  shift = other % chunk_size;
  if (shift > 0) {
    for (size_t i = 0; i < _n.size() - 1; ++i) {
      _n[i] = (_n[i + 1] << (chunk_size - shift)) | (_n[i] >> shift);
    }
  }
  convert();
  shrink_to_fit();
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  big_integer tmp(*this);
  if (tmp != 0) {
    tmp._sign = !tmp._sign;
  }
  return tmp;
}

big_integer big_integer::operator~() const {
  return -*this - 1;
}

big_integer& big_integer::operator++() {
  return *this += 1;
}

big_integer big_integer::operator++(int) {
  big_integer tmp(*this);
  ++(*this);
  return tmp;
}

big_integer& big_integer::operator--() {
  return *this -= 1;
}

big_integer big_integer::operator--(int) {
  big_integer tmp(*this);
  --(*this);
  return tmp;
}

big_integer operator+(const big_integer& a, const big_integer& b) {
  ;
  return big_integer(a) += b;
}

big_integer operator-(const big_integer& a, const big_integer& b) {
  return big_integer(a) -= b;
}

big_integer operator*(const big_integer& a, const big_integer& b) {
  return big_integer(a) *= b;
}

big_integer operator/(const big_integer& a, const big_integer& b) {
  return big_integer(a) /= b;
}

big_integer operator%(const big_integer& a, const big_integer& b) {
  return big_integer(a) %= b;
}

big_integer operator&(const big_integer& a, const big_integer& b) {
  return big_integer(a) &= b;
}

big_integer operator|(const big_integer& a, const big_integer& b) {
  return big_integer(a) |= b;
}

big_integer operator^(const big_integer& a, const big_integer& b) {
  return big_integer(a) ^= b;
}

big_integer operator<<(const big_integer& a, int b) {
  return big_integer(a) <<= b;
}

big_integer operator>>(const big_integer& a, int b) {
  return big_integer(a) >>= b;
}

bool operator==(const big_integer& a, const big_integer& b) {
  if (a._sign == b._sign || (b._n.size() == 1 && b._n.back() == 0)) {
    return a.abs_eq(b);
  }
  return false;
}

bool operator!=(const big_integer& a, const big_integer& b) {
  return !(a == b);
}

bool operator<(const big_integer& a, const big_integer& b) {
  return (a - b)._sign;
}

bool operator>(const big_integer& a, const big_integer& b) {
  return b < a;
}

bool operator<=(const big_integer& a, const big_integer& b) {
  return !(a > b);
}

bool operator>=(const big_integer& a, const big_integer& b) {
  return !(a < b);
}

std::string to_string(const big_integer& a) {
  bool sign = a._sign;
  big_integer divisible(a);
  divisible._sign = false;
  uint32_t neutral = static_cast<uint32_t>('0');
  std::string result;
  while (big_integer(0).abs_less(divisible)) {
    big_integer tmp = divisible.divide(base);
    result += static_cast<char>(neutral + divisible._n[0]);
    divisible = tmp;
  }
  if (result.empty()) {
    result += '0';
  }
  std::reverse(result.begin(), result.end());
  return sign ? "-" + result : result;
}

std::ostream& operator<<(std::ostream& out, const big_integer& a) {
  return out << to_string(a);
}
