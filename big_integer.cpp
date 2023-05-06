#include "big_integer.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace loc_consts {
constexpr uint32_t chunk_size = 32;
constexpr uint32_t chunk_size_log2 = 5;
constexpr uint64_t BETTA = 4'294'967'296;
constexpr unsigned long base = 10;
constexpr unsigned long transition_chunk_size = 9;
constexpr unsigned long transition_chunk = 1'000'000'000;

constexpr uint32_t CHUNK_MAX = std::numeric_limits<uint32_t>().max();
constexpr uint64_t DOUBLE_CHUNK_MAX = std::numeric_limits<uint64_t>().max();
constexpr int64_t SIGNED_DOUBLE_CHUNK_MIN = std::numeric_limits<int64_t>().min();
constexpr int64_t SIGNED_DOUBLE_CHUNK_MAX = std::numeric_limits<int64_t>().max();
} // namespace loc_consts

big_integer::big_integer() : _negative(false) {
  _digits.push_back(0);
}

big_integer::big_integer(const big_integer& other) = default;

big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned int a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(unsigned long a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(long long a)
    : big_integer(a == loc_consts::SIGNED_DOUBLE_CHUNK_MIN
                      ? loc_consts::DOUBLE_CHUNK_MAX - loc_consts::SIGNED_DOUBLE_CHUNK_MAX
                      : (a < 0 ? -a : a),
                  a < 0) {}

big_integer::big_integer(unsigned long long a) : big_integer(a, false) {}

big_integer::big_integer(unsigned long long a, bool sign) : _negative(sign) {
  _digits.push_back(a);
  if (a >>= loc_consts::chunk_size) {
    _digits.push_back(a);
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
  for (size_t i = sign; i < str.size(); i += loc_consts::transition_chunk_size) {
    unsigned long length = std::min(loc_consts::transition_chunk_size, str.size() - i);
    digit = std::stoi(str.substr(i, length));
    operator*=(length == loc_consts::transition_chunk_size ? loc_consts::transition_chunk
                                                           : static_cast<uint32_t>(pow(loc_consts::base, length)));
    operator+=(digit);
  }
  _negative = !(*this == 0) && sign;
}

big_integer::~big_integer() = default;

void big_integer::ensure_size(size_t n) {
  if (_digits.size() < n) {
    _digits.resize(n);
  }
}

void big_integer::reduce_zeroes() noexcept {
  while (_digits.size() > 1 && !_digits.back()) {
    _digits.pop_back();
  }
}

big_integer& big_integer::operator=(const big_integer& other) = default;

bool big_integer::abs_less(const big_integer& other) const {
  size_t size = _digits.size();
  if (size != other._digits.size()) {
    return size < other._digits.size();
  }
  return std::lexicographical_compare(_digits.rbegin(), _digits.rend(), other._digits.rbegin(), other._digits.rend());
}

bool big_integer::abs_eq(const big_integer& other) const {
  return _digits == other._digits;
}

big_integer& big_integer::convert() noexcept {
  if (_negative) {
    uint64_t carry = true;
    std::for_each(_digits.begin(), _digits.end(), [&carry](uint32_t& x) {
      uint64_t res = ~x;
      res += carry;
      x = res;
      carry = res >> loc_consts::chunk_size;
    });
  }
  return *this;
}

uint64_t eval_quotient(const big_integer& divisible, const big_integer& divider) {
  uint64_t l = 0, r = (loc_consts::DOUBLE_CHUNK_MAX >> loc_consts::chunk_size) + 1;
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

big_integer big_integer::divide(big_integer& A, big_integer B) {
  int shift = std::countl_zero(B._digits.back());
  A <<= shift;
  B <<= shift;
  big_integer::vec_ref a = A._digits, b = B._digits;
  size_t n = b.size(), m = a.size() - n;
  big_integer Q;
  big_integer::vec_ref q = Q._digits;
  q.resize(m + 1);
  if (A >= (B << static_cast<int>(m << loc_consts::chunk_size_log2))) {
    q[m] = 1;
    A -= (B << static_cast<int>(m << loc_consts::chunk_size_log2));
  } else {
    q[m] = 0;
  }
  for (size_t j = m; j-- > 0;) {
    uint64_t q_tmp =
        n + j < a.size() ? ((static_cast<uint64_t>(a[n + j]) << loc_consts::chunk_size) + a[n + j - 1]) / b[n - 1] : 0;
    q[j] = std::min(q_tmp, loc_consts::BETTA - 1);
    A -= ((q[j] * B) << static_cast<int>(j << loc_consts::chunk_size_log2));
    while (A < 0) {
      --q[j];
      A += (B << static_cast<int>(j << loc_consts::chunk_size_log2));
    }
  }
  Q.reduce_zeroes();
  return Q;
}

big_integer big_integer::divide(const big_integer& other) {
  if (abs_less(other)) {
    return 0;
  }
  big_integer copy(*this);
  big_integer quotient(divide(*this, big_integer(other)));
  *this = copy - other * quotient;
  return quotient;
}

big_integer::big_integer(vec vector) : _digits(vector), _negative(false) {}

template <typename F>
big_integer& big_integer::binary_bit_operation(const big_integer& other, const F& f) {
  bool less = abs_less(other);
  vector_bit_f(less ? other : *this, less ? *this : other, *this, f);
  return *this;
}

template <typename F>
void big_integer::vector_bit_f(const big_integer& a, const big_integer& b, big_integer& result, const F& f) {
  size_t max_len = std::max(a._digits.size(), b._digits.size());
  bool a_carry = a._negative, b_carry = b._negative;
  result._digits.resize(max_len);
  for (size_t i = 0; i < max_len; ++i) {
    uint64_t a_i = a._negative ? ~a._digits[i] : a._digits[i];
    a_i += a_carry;
    uint64_t b_i = b._negative ? ~b._digits[i] : b._digits[i];
    b_i += b_carry;
    a_carry = a_i > loc_consts::CHUNK_MAX;
    b_carry = b_i > loc_consts::CHUNK_MAX;
    result._digits[i] = f(a_i, b_i);
  }
  result._negative = f(a._negative, b._negative);
  result.convert();
  result.reduce_zeroes();
}

template <typename F>
void big_integer::vector_f(big_integer::const_vec_ref a, big_integer::const_vec_ref b, big_integer::vec_ref result,
                           const F& f) {
  bool carry = false;
  result.resize(std::max(a.size(), b.size()));
  for (size_t i = 0; i < a.size(); ++i) {
    int64_t res = f(f(a[i], (i < b.size() ? b[i] : 0)), carry);
    carry = (res > loc_consts::CHUNK_MAX) || (res < 0);
    result[i] = res;
  }
  if (carry) {
    result.push_back(carry);
  }
}

big_integer& big_integer::operator+=(const big_integer& other) {
  bool less = abs_less(other);
  big_integer::const_vec_ref greatest = less ? other._digits : _digits;
  big_integer::const_vec_ref smallest = less ? _digits : other._digits;
  if (_negative == other._negative) {
    vector_f(greatest, smallest, _digits, std::plus<int64_t>());
  } else {
    vector_f(greatest, smallest, _digits, std::minus<int64_t>());
  }
  reduce_zeroes();
  _negative = less ? other._negative : _negative && !abs_eq(0);
  return *this;
}

big_integer& big_integer::operator-=(const big_integer& other) {
  bool less = abs_less(other);
  big_integer::const_vec_ref greatest = less ? other._digits : _digits;
  big_integer::const_vec_ref smallest = less ? _digits : other._digits;
  if (_negative == other._negative) {
    vector_f(greatest, smallest, _digits, std::minus<int64_t>());
  } else {
    vector_f(greatest, smallest, _digits, std::plus<int64_t>());
  }
  reduce_zeroes();
  _negative = less ? !other._negative : _negative && !abs_eq(0);
  return *this;
}

big_integer& big_integer::operator*=(const big_integer& other) {
  size_t this_it = _digits.size();
  ensure_size(_digits.size() + other._digits.size());
  while (this_it-- > 0) {
    uint64_t carry = 0;
    uint64_t multiplier = _digits[this_it];
    _digits[this_it] = 0;
    size_t other_it = 0;
    while (other_it < other._digits.size() || carry) {
      const uint64_t result = (other_it < other._digits.size() ? other._digits[other_it] * multiplier : 0) +
                              _digits[this_it + other_it] + carry;
      _digits[this_it + other_it] = result;
      carry = result >> loc_consts::chunk_size;
      other_it++;
    }
  }
  _negative = _negative ^ other._negative;
  reduce_zeroes();
  return *this;
}

big_integer& big_integer::operator/=(const big_integer& other) {
  bool sign = _negative ^ other._negative;
  big_integer divider(other);
  divider._negative = _negative = false;
  *this = divide(divider);
  _negative = sign;
  return *this;
}

big_integer& big_integer::operator%=(const big_integer& other) {
  bool sign = _negative;
  big_integer divider(other);
  divider._negative = _negative = false;
  divide(divider);
  _negative = sign && *this != 0;
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
  size_t shift = other / loc_consts::chunk_size;
  ensure_size(_digits.size() + shift + 2);
  convert();
  for (size_t i = _digits.size() - 1; i-- > 0;) {
    _digits[i] = i < shift ? 0 : _digits[i - shift];
  }
  shift = other % loc_consts::chunk_size;
  for (size_t i = _digits.size() - 2; shift > 0 && i-- > other / loc_consts::chunk_size;) {
    _digits[i + 1] = (_digits[i + 1] << shift) | (_digits[i] >> (loc_consts::chunk_size - shift));
  }
  _digits[other / loc_consts::chunk_size] <<= shift;
  for (size_t i = 0; i < other / loc_consts::chunk_size; ++i) {
    _digits[i] = 0;
  }
  convert();
  reduce_zeroes();
  return *this;
}

big_integer& big_integer::operator>>=(int other) {
  if (_digits.size() * loc_consts::chunk_size < other) {
    return _negative ? *this = -1 : *this = 0;
  }
  size_t shift = other / loc_consts::chunk_size;
  ensure_size(_digits.size() + 2);
  convert();
  for (size_t i = 0; i < _digits.size(); ++i) {
    _digits[i] = i < _digits.size() - shift ? _digits[i + shift] : (_negative ? loc_consts::CHUNK_MAX : 0);
  }
  shift = other % loc_consts::chunk_size;
  for (size_t i = 0; shift > 0 && i < _digits.size() - 1; ++i) {
    _digits[i] = (_digits[i + 1] << (loc_consts::chunk_size - shift)) | (_digits[i] >> shift);
  }
  convert();
  reduce_zeroes();
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
  return operator-() - 1;
}

big_integer& big_integer::operator++() {
  return operator+=(1);
}

big_integer big_integer::operator++(int) {
  big_integer tmp(*this);
  operator++();
  return tmp;
}

big_integer& big_integer::operator--() {
  return *this -= 1;
}

big_integer big_integer::operator--(int) {
  big_integer tmp(*this);
  operator--();
  return tmp;
}

big_integer operator+(const big_integer& a, const big_integer& b) {
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
  if (a._negative == b._negative) {
    return a.abs_eq(b);
  }
  return false;
}

bool operator!=(const big_integer& a, const big_integer& b) {
  return !(a == b);
}

bool operator<(const big_integer& a, const big_integer& b) {
  if (a._negative == b._negative) {
    return a._negative ? b.abs_less(a) : a.abs_less(b);
  }
  return a._negative;
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
  bool sign = a._negative && !a.abs_eq(0);
  big_integer divisible(a);
  divisible._negative = false;
  std::string result;
  do {
    big_integer tmp(divisible.divide(loc_consts::transition_chunk));
    std::string portion(std::to_string(divisible._digits.front()));
    std::reverse(portion.begin(), portion.end());
    portion += std::string(loc_consts::transition_chunk_size - portion.size(), '0');
    result += portion;
    divisible = tmp;
  } while (big_integer(0).abs_less(divisible));
  while (result.size() > 1 && result.back() == '0') {
    result.pop_back();
  }
  if (sign) {
    result += "-";
  }
  std::reverse(result.begin(), result.end());
  return result;
}

std::ostream& operator<<(std::ostream& out, const big_integer& a) {
  return out << to_string(a);
}
