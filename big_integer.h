#pragma once

#include <iosfwd>
#include <string>
#include <vector>

struct big_integer {

  using vec = std::vector<uint32_t>;
  using vec_ref = std::vector<uint32_t>&;
  using const_vec_ref = const std::vector<uint32_t>&;

  big_integer();
  big_integer(const big_integer& other);
  big_integer(short a);
  big_integer(unsigned short a);
  big_integer(int a);
  big_integer(unsigned int a);
  big_integer(long a);
  big_integer(unsigned long a);
  big_integer(long long a);
  big_integer(unsigned long long a);
  explicit big_integer(const std::string& str);
  ~big_integer();

  void ensure_size(size_t n);
  void shrink_to_fit();
  void swap(big_integer& other);

  big_integer& operator=(const big_integer& other);

  big_integer& operator+=(const big_integer& rhs);
  big_integer& operator-=(const big_integer& rhs);
  big_integer& operator*=(const big_integer& rhs);
  big_integer& operator/=(const big_integer& rhs);
  big_integer& operator%=(const big_integer& rhs);

  big_integer& operator&=(const big_integer& rhs);
  big_integer& operator|=(const big_integer& rhs);
  big_integer& operator^=(const big_integer& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(const big_integer& a, const big_integer& b);
  friend bool operator!=(const big_integer& a, const big_integer& b);
  friend bool operator<(const big_integer& a, const big_integer& b);
  friend bool operator>(const big_integer& a, const big_integer& b);
  friend bool operator<=(const big_integer& a, const big_integer& b);
  friend bool operator>=(const big_integer& a, const big_integer& b);

  friend std::string to_string(const big_integer& a);

private:
  bool abs_less(const big_integer& other) const;
  bool abs_eq(const big_integer& other) const;

  big_integer& convert();
  friend uint64_t eval_quotient(const big_integer& divisible, const big_integer& divider);
  big_integer divide(const big_integer& other);
  big_integer(vec other);

  template <typename F>
  big_integer& binary_bit_operation(const big_integer& other, const F& f);
  template <typename F>
  friend void vector_bit_f(const big_integer& a, const big_integer& b, big_integer& result, const F& f);
  template <typename F>
  friend void vector_f(const_vec_ref a, const_vec_ref b, vec_ref result, const F& f);

private:
  vec _n;
  bool _sign;
};

big_integer operator+(const big_integer& a, const big_integer& b);
big_integer operator-(const big_integer& a, const big_integer& b);
big_integer operator*(const big_integer& a, const big_integer& b);
big_integer operator/(const big_integer& a, const big_integer& b);
big_integer operator%(const big_integer& a, const big_integer& b);

big_integer operator&(const big_integer& a, const big_integer& b);
big_integer operator|(const big_integer& a, const big_integer& b);
big_integer operator^(const big_integer& a, const big_integer& b);

big_integer operator<<(const big_integer& a, int b);
big_integer operator>>(const big_integer& a, int b);

bool operator==(const big_integer& a, const big_integer& b);
bool operator!=(const big_integer& a, const big_integer& b);
bool operator<(const big_integer& a, const big_integer& b);
bool operator>(const big_integer& a, const big_integer& b);
bool operator<=(const big_integer& a, const big_integer& b);
bool operator>=(const big_integer& a, const big_integer& b);

std::string to_string(const big_integer& a);
std::ostream& operator<<(std::ostream& out, const big_integer& a);
