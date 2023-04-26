#include "big_integer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>

constexpr uint32_t chunk_size = 32;
constexpr unsigned long base = 10;
constexpr unsigned long transition_chunk_size = 9;
constexpr unsigned long to_div = 1'000'000'000;

big_integer::big_integer() : _negative(false) {
  _digits.push_back(0);
}

big_integer::big_integer(const big_integer& other) : _negative(other._negative) {
  _digits.resize(other._digits.size());
  std::copy(other._digits.begin(), other._digits.end(), _digits.begin());
}

big_integer::big_integer(short a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned short a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned int a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned long a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long long a) : _negative(a < 0) {
  unsigned long long x = std::abs(a);
  _digits.push_back(x);
  if (x >> chunk_size) {
    _digits.push_back(x >> chunk_size);
  }
}

big_integer::big_integer(unsigned long long a) : _negative(false) {
  _digits.push_back(a);
  if (a >> chunk_size) {
    _digits.push_back(a >> chunk_size);
  }
}

big_integer::big_integer(const std::string& str) : _negative(false) {
  *this = 0;
  if (str.empty()) {
    throw std::invalid_argument("Expected valid number while initializing big_integer, empty string found.");
  }
  bool sign = str[0] == '-';
  if (sign && str.length() == 1) {
    throw std::invalid_argument("Expected valid number while initializing big_integer, single dash found.");
  }
  if (!std::all_of(str.begin() + sign, str.end(), ::isdigit)) {
    throw std::invalid_argument("Met invalid character while initializing big_integer with a string.");
  }
  uint32_t digit = 0;
  for (size_t i = 0 + sign; i < str.size(); i += transition_chunk_size) {
    unsigned long length = std::min(transition_chunk_size, str.size() - i);
    digit = std::stoi(str.substr(i, length));
    (*this *= static_cast<uint32_t>(pow(base, length))) += digit;
  }
  _negative = !(*this == 0) && sign;
}

big_integer::~big_integer() = default;

void big_integer::ensure_size(size_t n) {
  if (_digits.size() < n) {
    _digits.resize(n);
  }
}

void big_integer::shrink_to_fit() {
  while (_digits.size() > 1 && !_digits.back()) {
    _digits.pop_back();
  }
}

void big_integer::swap(big_integer& other) {
  std::swap(_digits, other._digits);
  std::swap(_negative, other._negative);
}

big_integer& big_integer::operator=(const big_integer& other) {
  if (this != &other) {
    big_integer tmp(other);
    swap(tmp);
  }
  return *this;
}

bool big_integer::abs_less(const big_integer& other) const {
  size_t size = _digits.size();
  if (size != other._digits.size()) {
    return size < other._digits.size();
  }
  for (size_t i = 0, index = size - 1 - i; i < size; ++i, --index) {
    if (_digits[index] != other._digits[index]) {
      return _digits[index] < other._digits[index];
    }
  }
  return false;
}

bool big_integer::abs_eq(const big_integer& other) const {
  return (_digits.size() == other._digits.size()) && std::equal(_digits.begin(), _digits.end(), other._digits.begin());
}

big_integer& big_integer::convert() {
  if (_negative && *this != 0) {
    bool carry = true;
    std::for_each(_digits.begin(), _digits.end(), [&carry](uint32_t& x) {
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
    uint32_t m = (l + r) / 2;
    if (m * divider <= divisible) {
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
  auto it = static_cast<int64_t>(_digits.size() - other._digits.size());
  big_integer result;
  result.ensure_size(it + 1);
  while (it >= 0) {
    big_integer portion = vec(_digits.begin() + it, _digits.end());
    portion.shrink_to_fit();
    uint64_t quotient = eval_quotient(portion, other);
    result._digits[it] = quotient;
    big_integer replacement(portion - quotient * other);
    replacement.ensure_size(_digits.end() - _digits.begin() - it);
    std::copy(replacement._digits.begin(), replacement._digits.end(), _digits.begin() + it);
    --it;
  }
  shrink_to_fit();
  result.shrink_to_fit();
  return result;
}

big_integer::big_integer(vec vector) : _digits(std::move(vector)), _negative(false) {}

template <typename F>
big_integer& big_integer::binary_bit_operation(const big_integer& other, const F& f) {
  bool less = abs_less(other);
  const big_integer &greatest = less ? other : *this, smallest = less ? *this : other;
  vector_bit_f(greatest, smallest, *this, f);
  return *this;
}

template <typename F>
void vector_bit_f(const big_integer& a, const big_integer& b, big_integer& result, const F& f) {
  bool a_carry = a._negative, b_carry = b._negative;
  for (size_t i = 0; i < a._digits.size(); ++i) {
    uint64_t converted_a_i = (a._negative ? ~a._digits[i] : a._digits[i]);
    converted_a_i += a_carry;
    uint64_t converted_b_i =
        i < b._digits.size() ? (b._negative ? ~b._digits[i] : b._digits[i]) : (b._negative ? UINT32_MAX : 0);
    converted_b_i += b_carry;
    a_carry = converted_a_i >> chunk_size;
    b_carry = converted_b_i >> chunk_size;
    uint64_t res = f(converted_a_i, converted_b_i);
    if (i < result._digits.size()) {
      result._digits[i] = res;
    } else {
      result._digits.push_back(res);
    }
  }
  result._negative = f(a._negative, b._negative);
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
  big_integer::const_vec_ref greatest = less ? other._digits : _digits, smallest = less ? _digits : other._digits;
  if (_negative == other._negative) {
    vector_f(greatest, smallest, _digits, std::plus<int64_t>());
  } else {
    vector_f(greatest, smallest, _digits, std::minus<int64_t>());
  }
  if (less) {
    _negative = other._negative;
  }
  if (*this == 0) {
    _negative = false;
  }
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator-=(const big_integer& other) {
  return *this += -other;
}

big_integer& big_integer::operator*=(const big_integer& other) {
  auto this_it = static_cast<int64_t>(_digits.size());
  ensure_size(_digits.size() + other._digits.size());
  while (--this_it >= 0) {
    uint64_t carry = 0;
    uint64_t multiplier = _digits[this_it];
    _digits[this_it] = 0;
    size_t other_it = 0;
    while (other_it < other._digits.size() || carry) {
      const uint64_t result = (other_it < other._digits.size() ? other._digits[other_it] * multiplier : 0) +
                              _digits[this_it + other_it] + carry;
      _digits[this_it + other_it] = result;
      carry = result >> chunk_size;
      other_it++;
    }
  }
  _negative = _negative ^ other._negative;
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator/=(const big_integer& other) {
  bool sign = _negative ^ other._negative;
  auto divider(other);
  divider._negative = false;
  _negative = false;
  *this = divide(divider);
  _negative = sign;
  return *this;
}

big_integer& big_integer::operator%=(const big_integer& other) {
  bool sign = _negative;
  auto divider(other);
  divider._negative = false;
  _negative = false;
  divide(divider);
  _negative = sign;
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
  ensure_size(_digits.size() + shift + 2);
  convert();
  for (int64_t i = _digits.size() - 1; i >= 0; --i) {
    _digits[i] = i - shift < 0 ? 0 : _digits[i - shift];
  }
  shift = other % chunk_size;
  if (shift > 0) {
    for (int64_t i = _digits.size() - 2; i >= other / chunk_size; --i) {
      _digits[i + 1] = (_digits[i + 1] << shift) | (_digits[i] >> (chunk_size - shift));
    }
  }
  _digits[other / chunk_size] <<= shift;
  for (size_t i = 0; i < other / chunk_size; ++i) {
    _digits[i] = 0;
  }
  convert();
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator>>=(int other) {
  if (_digits.size() * chunk_size < other) {
    return _negative ? *this = -1 : *this = 0;
  }
  uint64_t shift = other / chunk_size;
  ensure_size(_digits.size() + 2);
  convert();
  for (size_t i = 0; i < _digits.size(); ++i) {
    _digits[i] = i < _digits.size() - shift ? _digits[i + shift] : (_negative ? UINT32_MAX : 0);
  }
  shift = other % chunk_size;
  if (shift > 0) {
    for (size_t i = 0; i < _digits.size() - 1; ++i) {
      _digits[i] = (_digits[i + 1] << (chunk_size - shift)) | (_digits[i] >> shift);
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
    tmp._negative = !tmp._negative;
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
  if (a._negative == b._negative || (b._digits.size() == 1 && b._digits.back() == 0)) {
    return a.abs_eq(b);
  }
  return false;
}

bool operator!=(const big_integer& a, const big_integer& b) {
  return !(a == b);
}

bool operator<(const big_integer& a, const big_integer& b) {
  return (a - b)._negative;
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
  bool sign = a._negative;
  big_integer divisible(a);
  divisible._negative = false;
  std::string result;
  big_integer::vec answer;
  while (big_integer(0).abs_less(divisible)) {
    big_integer tmp(divisible.divide(to_div));
    answer.push_back(divisible._digits.front());
    divisible = tmp;
  }
  if (answer.empty()) {
    result += '0';
  } else {
    result += sign ? "-" : "";
    result += std::to_string(answer.back());
    answer.pop_back();
    while (!answer.empty()) {
      std::string tmp(to_string(answer.back()));
      answer.pop_back();
      result += std::string(transition_chunk_size - tmp.size(), '0') + tmp;
    }
  }
  return result;
}

std::ostream& operator<<(std::ostream& out, const big_integer& a) {
  return out << to_string(a);
}
