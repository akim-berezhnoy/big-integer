#include "big_integer.h"

#include <cstddef>
#include <cstring>
#include <ostream>
#include <stdexcept>

big_integer::big_integer() = default;

big_integer::big_integer(const big_integer& other) = default;

big_integer::big_integer(int a) {}

big_integer::big_integer(const std::string& str) {}

big_integer::~big_integer() = default;

big_integer& big_integer::operator=(const big_integer& other) {
  return *this;
}

big_integer& big_integer::operator+=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator-=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator*=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator/=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator%=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator&=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator|=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator^=(const big_integer& rhs) {
  return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
  return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  return *this;
}

big_integer big_integer::operator~() const {
  return *this;
}

big_integer& big_integer::operator++() {
  return *this;
}

big_integer big_integer::operator++(int) {
  return *this;
}

big_integer& big_integer::operator--() {
  return *this;
}

big_integer big_integer::operator--(int) {
  return *this;
}

big_integer operator+(big_integer a, const big_integer& b) {
  return a;
}

big_integer operator-(big_integer a, const big_integer& b) {
  return a;
}

big_integer operator*(big_integer a, const big_integer& b) {
  return a;
}

big_integer operator/(big_integer a, const big_integer& b) {
  return a;
}

big_integer operator%(big_integer a, const big_integer& b) {
  return a;
}

big_integer operator&(big_integer a, const big_integer& b) {
  return a;
}

big_integer operator|(big_integer a, const big_integer& b) {
  return a |= b;
}

big_integer operator^(big_integer a, const big_integer& b) {
  return a;
}

big_integer operator<<(big_integer a, int b) {
  return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
  return a;
}

bool operator==(const big_integer& a, const big_integer& b) {
  return true;
}

bool operator!=(const big_integer& a, const big_integer& b) {
  return true;
}

bool operator<(const big_integer& a, const big_integer& b) {
  return true;
}

bool operator>(const big_integer& a, const big_integer& b) {
  return true;
}

bool operator<=(const big_integer& a, const big_integer& b) {
  return true;
}

bool operator>=(const big_integer& a, const big_integer& b) {
  return true;
}

std::string to_string(const big_integer& a) {
  return "";
}

std::ostream& operator<<(std::ostream& s, const big_integer& a) {
  return s << to_string(a);
}
