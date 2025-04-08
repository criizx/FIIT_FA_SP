//
// Created by Des Caldnd on 5/27/2024.
//

#include "../include/big_int.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <ranges>
#include <sstream>
#include <string>

bool validating_string(const std::string &str, int radix) {
	if (str.empty()) return false;

	size_t start_idx = (str[0] == '-') ? 1 : 0;
	if (start_idx == str.size()) return false;

	return std::all_of(str.begin() + start_idx, str.end(), [radix](char ch) {
		ch = std::toupper(static_cast<unsigned char>(ch));
		if (std::isdigit(ch)) {
			return (ch - '0') < radix;
		} else if (ch >= 'A' && ch <= 'Z') {
			return (ch - 'A' + 10) < radix;
		}
		return false;
	});
}

std::strong_ordering big_int::operator<=>(const big_int &other) const noexcept {
	throw not_implemented("std::strong_ordering big_int::operator<=>(const big_int &) const noexcept",
	                      "your code should be here...");
}

big_int::operator bool() const noexcept {
	throw not_implemented("big_int::operator bool()", "your code should be here...");
}

big_int &big_int::operator++() & {
	throw not_implemented("big_int &big_int::operator++()", "your code should be here...");
}

big_int big_int::operator++(int) {
	throw not_implemented("big_int big_int::operator++(int)", "your code should be here...");
}

big_int &big_int::operator--() & {
	throw not_implemented("big_int &big_int::operator--()", "your code should be here...");
}

big_int big_int::operator--(int) {
	throw not_implemented("big_int big_int::operator--(int)", "your code should be here...");
}

big_int &big_int::operator+=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator+=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator-=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator-=(const big_int &)", "your code should be here...");
}

big_int big_int::operator+(const big_int &other) const {
	throw not_implemented("big_int big_int::operator+(const big_int &) const", "your code should be here...");
}

big_int big_int::operator-(const big_int &other) const {
	throw not_implemented("big_int big_int::operator-(const big_int &) const", "your code should be here...");
}

big_int big_int::operator*(const big_int &other) const {
	throw not_implemented("big_int big_int::operator*(const big_int &) const", "your code should be here...");
}

big_int big_int::operator/(const big_int &other) const {
	throw not_implemented("big_int big_int::operator/(const big_int &) const", "your code should be here...");
}

big_int big_int::operator%(const big_int &other) const {
	throw not_implemented("big_int big_int::operator%(const big_int &) const", "your code should be here...");
}

big_int big_int::operator&(const big_int &other) const {
	throw not_implemented("big_int big_int::operator&(const big_int &) const", "your code should be here...");
}

big_int big_int::operator|(const big_int &other) const {
	throw not_implemented("big_int big_int::operator|(const big_int &) const", "your code should be here...");
}

big_int big_int::operator^(const big_int &other) const {
	throw not_implemented("big_int big_int::operator^(const big_int &) const", "your code should be here...");
}

big_int big_int::operator<<(size_t shift) const {
	throw not_implemented("big_int big_int::operator<<(size_t ) const", "your code should be here...");
}

big_int big_int::operator>>(size_t shift) const {
	throw not_implemented("big_int big_int::operator>>(size_t) const", "your code should be here...");
}

big_int &big_int::operator%=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator%=(const big_int &)", "your code should be here...");
}

big_int big_int::operator~() const {
	throw not_implemented("big_int big_int::operator~() const", "your code should be here...");
}

big_int &big_int::operator&=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator&=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator|=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator|=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator^=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator^=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator<<=(size_t shift) & {
	throw not_implemented("big_int &big_int::operator<<=(size_t)", "your code should be here...");
}

big_int &big_int::operator>>=(size_t shift) & {
	throw not_implemented("big_int &big_int::operator>>=(size_t)", "your code should be here...");
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) & {
	throw not_implemented("big_int &big_int::plus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) & {
	throw not_implemented("big_int &big_int::minus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::operator*=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator*=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator/=(const big_int &other) & {
	throw not_implemented("big_int &big_int::operator/=(const big_int &)", "your code should be here...");
}

std::string big_int::to_string() const {
	throw not_implemented("std::string big_int::to_string() const", "your code should be here...");
}

std::ostream &operator<<(std::ostream &stream, const big_int &value) {
	throw not_implemented("std::ostream &operator<<(std::ostream &, const big_int &)", "your code should be here...");
}

std::istream &operator>>(std::istream &stream, big_int &value) {
	throw not_implemented("std::istream &operator>>(std::istream &, big_int &)", "your code should be here...");
}

bool big_int::operator==(const big_int &other) const noexcept {
	throw not_implemented("bool big_int::operator==(const big_int &) const noexcept", "your code should be here...");
}

big_int::big_int(const std::vector<unsigned int, pp_allocator<unsigned int>> &digits, bool sign)
    : _sign(sign), _digits(digits) {
	while (!_digits.empty() && _digits.back() == 0) {
		_digits.pop_back();
	}

	if (_digits.empty()) {
		_sign = true;
		_digits.push_back(0);
	}
}

big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign) noexcept
    : _sign(sign), _digits(std::move(digits)) {
	while (!_digits.empty() && _digits.back() == 0) {
		_digits.pop_back();
	}

	if (_digits.empty()) {
		_sign = true;
		_digits.push_back(0);
	}
}

big_int::big_int(const std::string &num, unsigned int radix, pp_allocator<unsigned int>) {
	if (num.empty()) throw std::exception();
	if (!validating_string(num, radix)) {
		throw std::invalid_argument("Invalid number representation for the given radix.");
	}

	_sign = (num[0] == '-') ? false : true;
	size_t start_idx = (num[0] == '-') ? 1 : 0;

	_digits.clear();
	big_int base(radix);
	big_int value(0);

	for (size_t i = start_idx; i < num.size(); ++i) {
		char ch = std::toupper(static_cast<unsigned char>(num[i]));
		unsigned int digit = std::isdigit(ch) ? (ch - '0') : (ch - 'A' + 10);

		value *= base;
		value += big_int(digit);
	}

	_digits = std::move(value._digits);
}

big_int::big_int(pp_allocator<unsigned int>) {
	throw not_implemented("big_int::big_int(pp_allocator<unsigned int>)", "your code should be here...");
}

big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) & {
	throw not_implemented(
	    "big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) &",
	    "your code should be here...");
}

big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) & {
	throw not_implemented("big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &",
	                      "your code should be here...");
}

big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) & {
	throw not_implemented("big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) &",
	                      "your code should be here...");
}

big_int operator""_bi(unsigned long long n) {
	throw not_implemented("big_int operator\"\"_bi(unsigned long long n)", "your code should be here...");
}