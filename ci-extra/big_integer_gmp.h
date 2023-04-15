#pragma once

#include <gmp.h>

#include <cstddef>
#include <iosfwd>

struct big_integer_gmp {
  big_integer_gmp();
  big_integer_gmp(const big_integer_gmp& other);
  big_integer_gmp(int a);
  explicit big_integer_gmp(const std::string& str);

  template <typename RNG>
  big_integer_gmp& random(size_t sz, RNG&& rng) {
    gmp_randstate_t state;
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, rng());
    mpz_urandomb(mpz, state, sz + 1);
    gmp_randclear(state);
    big_integer_gmp shift(1);
    shift <<= sz;
    *this -= shift;
    return *this;
  }

  ~big_integer_gmp();

  big_integer_gmp& operator=(const big_integer_gmp& other);

  big_integer_gmp& operator+=(const big_integer_gmp& rhs);
  big_integer_gmp& operator-=(const big_integer_gmp& rhs);
  big_integer_gmp& operator*=(const big_integer_gmp& rhs);
  big_integer_gmp& operator/=(const big_integer_gmp& rhs);
  big_integer_gmp& operator%=(const big_integer_gmp& rhs);

  big_integer_gmp& operator&=(const big_integer_gmp& rhs);
  big_integer_gmp& operator|=(const big_integer_gmp& rhs);
  big_integer_gmp& operator^=(const big_integer_gmp& rhs);

  big_integer_gmp& operator<<=(int rhs);
  big_integer_gmp& operator>>=(int rhs);

  big_integer_gmp operator+() const;
  big_integer_gmp operator-() const;
  big_integer_gmp operator~() const;

  big_integer_gmp& operator++();
  big_integer_gmp operator++(int);

  big_integer_gmp& operator--();
  big_integer_gmp operator--(int);

  friend bool operator==(const big_integer_gmp& a, const big_integer_gmp& b);
  friend bool operator!=(const big_integer_gmp& a, const big_integer_gmp& b);
  friend bool operator<(const big_integer_gmp& a, const big_integer_gmp& b);
  friend bool operator>(const big_integer_gmp& a, const big_integer_gmp& b);
  friend bool operator<=(const big_integer_gmp& a, const big_integer_gmp& b);
  friend bool operator>=(const big_integer_gmp& a, const big_integer_gmp& b);

  friend std::string to_string(const big_integer_gmp& a);

private:
  mpz_t mpz;
};

big_integer_gmp operator+(big_integer_gmp a, const big_integer_gmp& b);
big_integer_gmp operator-(big_integer_gmp a, const big_integer_gmp& b);
big_integer_gmp operator*(big_integer_gmp a, const big_integer_gmp& b);
big_integer_gmp operator/(big_integer_gmp a, const big_integer_gmp& b);
big_integer_gmp operator%(big_integer_gmp a, const big_integer_gmp& b);

big_integer_gmp operator&(big_integer_gmp a, const big_integer_gmp& b);
big_integer_gmp operator|(big_integer_gmp a, const big_integer_gmp& b);
big_integer_gmp operator^(big_integer_gmp a, const big_integer_gmp& b);

big_integer_gmp operator<<(big_integer_gmp a, int b);
big_integer_gmp operator>>(big_integer_gmp a, int b);

bool operator==(const big_integer_gmp& a, const big_integer_gmp& b);
bool operator!=(const big_integer_gmp& a, const big_integer_gmp& b);
bool operator<(const big_integer_gmp& a, const big_integer_gmp& b);
bool operator>(const big_integer_gmp& a, const big_integer_gmp& b);
bool operator<=(const big_integer_gmp& a, const big_integer_gmp& b);
bool operator>=(const big_integer_gmp& a, const big_integer_gmp& b);

std::string to_string(const big_integer_gmp& a);
std::ostream& operator<<(std::ostream& s, const big_integer_gmp& a);
