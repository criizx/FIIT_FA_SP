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

std::strong_ordering big_int::operator<=>(const big_int &other) const noexcept {
	if (_sign != other._sign) {
		return _sign ? std::strong_ordering::greater : std::strong_ordering::less;
	}

	if (_digits.size() != other._digits.size()) {
		if (_sign) {
			return _digits.size() <=> other._digits.size();
		} else {
			return other._digits.size() <=> _digits.size();
		}
	}

	for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; --i) {
		if (_digits[i] != other._digits[i]) {
			if (_sign) {
				return _digits[i] <=> other._digits[i];
			} else {
				return other._digits[i] <=> _digits[i];
			}
		}
	}

	return std::strong_ordering::equal;
}

big_int::operator bool() const noexcept { return !(_digits.size() == 1 && _digits[0] == 0); }

big_int &big_int::operator+=(const big_int &other) & {
	if (_sign == other._sign) {
		plus_assign(other, 0);
	} else {
		big_int abs_this = *this;
		abs_this._sign = true;
		big_int abs_other = other;
		abs_other._sign = true;

		if (abs_this >= abs_other) {
			minus_assign(other, 0);
		} else {
			big_int temp = other;
			temp.minus_assign(*this, 0);
			*this = temp;
			_sign = other._sign;
		}
	}
	return *this;
}

big_int &big_int::operator-=(const big_int &other) & {
	if (_sign != other._sign) {
		plus_assign(other, 0);
	} else {
		big_int abs_this = *this;
		abs_this._sign = true;
		big_int abs_other = other;
		abs_other._sign = true;

		if (abs_this >= abs_other) {
			minus_assign(abs_other, 0);
		} else {
			big_int temp = other;
			temp.minus_assign(*this, 0);
			*this = temp;
			_sign = !_sign;
		}
	}
	return *this;
}

big_int big_int::operator+(const big_int &other) const {
	big_int result = *this;
	result += other;
	return result;
}

big_int big_int::operator-(const big_int &other) const {
	big_int result = *this;
	result -= other;
	return result;
}

big_int big_int::operator*(const big_int &other) const {
	big_int result = *this;
	result *= other;
	return result;
}

big_int big_int::operator/(const big_int &other) const {
	big_int result = *this;
	result /= other;
	return result;
}

big_int big_int::operator%(const big_int &other) const {
	big_int result = *this;
	result %= other;
	return result;
}

big_int big_int::operator&(const big_int &other) const {
	big_int result = *this;
	result &= other;
	return result;
}

big_int big_int::operator|(const big_int &other) const {
	big_int result = *this;
	result |= other;
	return result;
}

big_int big_int::operator^(const big_int &other) const {
	big_int result = *this;
	result ^= other;
	return result;
}

big_int big_int::operator<<(size_t shift) const {
	big_int result = *this;
	result <<= shift;
	return result;
}

big_int big_int::operator>>(size_t shift) const {
	big_int result = *this;
	result >>= shift;
	return result;
}

big_int big_int::operator~() const {
	big_int result = *this;

	for (auto &digit : result._digits) {
		digit = ~digit;
	}

	result._sign = !result._sign;
	return result;
}

big_int &big_int::operator&=(const big_int &other) & {
	big_int a = this->to_twos_complement();
	big_int b = other.to_twos_complement();

	size_t max_size = std::max(a._digits.size(), b._digits.size());
	a._digits.resize(max_size, a._sign ? 0 : 0xFFFFFFFF);
	b._digits.resize(max_size, b._sign ? 0 : 0xFFFFFFFF);

	for (size_t i = 0; i < max_size; ++i) {
		a._digits[i] &= b._digits[i];
	}

	*this = a.from_twos_complement();
	optimize();
	return *this;
}

big_int &big_int::operator|=(const big_int &other) & {
	big_int a = this->to_twos_complement();
	big_int b = other.to_twos_complement();

	size_t max_size = std::max(a._digits.size(), b._digits.size());
	a._digits.resize(max_size, a._sign ? 0 : 0xFFFFFFFF);
	b._digits.resize(max_size, b._sign ? 0 : 0xFFFFFFFF);

	for (size_t i = 0; i < max_size; ++i) {
		a._digits[i] |= b._digits[i];
	}

	*this = a.from_twos_complement();
	optimize();
	return *this;
}

big_int &big_int::operator++() & {
	*this = *this + 1;
	return *this;
}

big_int &big_int::operator^=(const big_int &other) & {
	big_int a = this->to_twos_complement();
	big_int b = other.to_twos_complement();

	size_t max_size = std::max(a._digits.size(), b._digits.size());
	a._digits.resize(max_size, a._sign ? 0 : 0xFFFFFFFF);
	b._digits.resize(max_size, b._sign ? 0 : 0xFFFFFFFF);

	for (size_t i = 0; i < max_size; ++i) {
		a._digits[i] ^= b._digits[i];
	}

	*this = a.from_twos_complement();
	optimize();
	return *this;
}

big_int &big_int::operator<<=(size_t shift) & {
	if (shift == 0 || (_digits.size() == 1 && _digits[0] == 0)) {
		return *this;
	}

	size_t digit_shifts = shift / (sizeof(unsigned int) * 8);
	size_t bit_shifts = shift % (sizeof(unsigned int) * 8);

	if (digit_shifts > 0) {
		_digits.insert(_digits.begin(), digit_shifts, 0);
	}

	if (bit_shifts > 0) {
		unsigned long long carry = 0;
		for (size_t i = 0; i < _digits.size(); ++i) {
			unsigned long long current = static_cast<unsigned long long>(_digits[i]) << bit_shifts;
			current |= carry;
			_digits[i] = static_cast<unsigned int>(current & 0xFFFFFFFF);
			carry = current >> (sizeof(unsigned int) * 8);
		}

		if (carry > 0) {
			_digits.push_back(static_cast<unsigned int>(carry));
		}
	}

	optimize();
	return *this;
}

big_int &big_int::operator>>=(size_t shift) & {
	if (shift == 0 || (_digits.size() == 1 && _digits[0] == 0)) {
		return *this;
	}

	size_t digit_shifts = shift / (sizeof(unsigned int) * 8);
	size_t bit_shifts = shift % (sizeof(unsigned int) * 8);

	if (digit_shifts >= _digits.size()) {
		_digits.clear();
		_digits.push_back(0);
		_sign = true;
		return *this;
	}

	if (digit_shifts > 0) {
		_digits.erase(_digits.begin(), _digits.begin() + digit_shifts);
	}

	if (bit_shifts > 0) {
		unsigned long long borrow = 0;
		for (int i = _digits.size() - 1; i >= 0; --i) {
			unsigned long long current = static_cast<unsigned long long>(_digits[i]);
			unsigned int next_borrow = (current & ((1ULL << bit_shifts) - 1))
			                           << ((sizeof(unsigned int) * 8) - bit_shifts);
			_digits[i] = (current >> bit_shifts) | static_cast<unsigned int>(borrow);
			borrow = next_borrow;
		}
	}

	optimize();
	return *this;
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) & {
	size_t other_size = other._digits.size() + shift;
	_digits.resize(std::max(_digits.size(), other_size), 0);

	unsigned long long carry = 0;
	for (size_t i = 0; i < other._digits.size() || carry > 0; ++i) {
		size_t idx = i + shift;
		if (idx >= _digits.size()) {
			_digits.push_back(0);
		}

		unsigned long long d1 = (i < other._digits.size()) ? other._digits[i] : 0;
		unsigned long long sum = _digits[idx] + d1 + carry;
		_digits[idx] = static_cast<unsigned int>(sum & 0xFFFFFFFF);
		carry = sum >> (sizeof(unsigned int) * 8);
	}

	while (!_digits.empty() && _digits.back() == 0) {
		_digits.pop_back();
	}

	return *this;
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) & {
	if (_digits.size() < other._digits.size() + shift) {
		_digits.resize(other._digits.size() + shift, 0);
	}

	unsigned int borrow = 0;
	for (size_t i = 0; i < other._digits.size() || borrow > 0; ++i) {
		size_t idx = i + shift;
		if (idx >= _digits.size()) {
			_digits.push_back(0);
		}

		unsigned int d1 = (i < other._digits.size()) ? other._digits[i] : 0;
		if (_digits[idx] >= d1 + borrow) {
			_digits[idx] -= d1 + borrow;
			borrow = 0;
		} else {
			_digits[idx] = static_cast<unsigned int>(0x100000000ULL + _digits[idx] - d1 - borrow);
			borrow = 1;
		}
	}

	optimize();
	return *this;
}

big_int &big_int::operator*=(const big_int &other) & {
	return multiply_assign(other, decide_mult(other._digits.size()));
}

big_int::multiplication_rule big_int::decide_mult(size_t rhs) const noexcept {
	const size_t lhs_size = _digits.size();
	const size_t threshold = 64;

	if (lhs_size < threshold || rhs < threshold) {
		return multiplication_rule::trivial;
	} else {
		return multiplication_rule::Karatsuba;
	}
}

big_int &big_int::operator/=(const big_int &other) & { return divide_assign(other, decide_div(other._digits.size())); }

big_int::division_rule big_int::decide_div(size_t rhs) const noexcept { return division_rule::trivial; }

big_int &big_int::operator%=(const big_int &other) & { return modulo_assign(other, decide_div(other._digits.size())); }

std::string big_int::to_string() const {
	if (_digits.empty()) {
		return "0";
	}

	big_int num = *this;
	num._sign = true;

	std::string result;
	while (!num._digits.empty()) {
		unsigned int remainder = num.divide_by_10();
		result.push_back(remainder + '0');
	}

	if (result.empty()) {
		result = "0";
	} else {
		std::reverse(result.begin(), result.end());
	}

	if (!_sign && result != "0") {
		result.insert(result.begin(), '-');
	}

	return result;
}

std::ostream &operator<<(std::ostream &stream, const big_int &value) {
	stream << value.to_string();
	return stream;
}

std::istream &operator>>(std::istream &stream, big_int &value) {
	std::string input;
	stream >> input;

	if (!input.empty()) {
		try {
			value = big_int(input);
		} catch (const std::exception &e) {
			stream.setstate(std::ios::failbit);
		}
	} else {
		stream.setstate(std::ios::failbit);
	}

	return stream;
}

bool big_int::operator==(const big_int &other) const noexcept {
	if (_sign != other._sign || _digits.size() != other._digits.size()) {
		return false;
	}

	for (size_t i = 0; i < _digits.size(); ++i) {
		if (_digits[i] != other._digits[i]) {
			return false;
		}
	}

	return true;
}

bool big_int::operator!=(const big_int &other) const noexcept { return !(*this == other); }

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
	if (radix != 10) {
		throw("big_int::big_int with radix != 10");
	}

	_sign = true;
	size_t start = 0;
	if (!num.empty()) {
		if (num[0] == '-') {
			_sign = false;
			start = 1;
		} else if (num[0] == '+') {
			start = 1;
		}
	}

	while (start < num.size() && num[start] == '0') {
		++start;
	}

	if (start == num.size()) {
		_sign = true;
		_digits.clear();
		return;
	}

	std::string digits_str = num.substr(start);
	_digits.push_back(0);

	for (char c : digits_str) {
		if (!isdigit(c)) {
			throw std::invalid_argument("invalid digit in number string");
		}
		unsigned int digit = c - '0';
		multiply_by_digit(10);
		unsigned long long carry = digit;
		for (size_t i = 0; i < _digits.size() && carry > 0; ++i) {
			unsigned long long sum = _digits[i] + carry;
			_digits[i] = static_cast<unsigned int>(sum & 0xFFFFFFFF);
			carry = sum >> (sizeof(unsigned int) * 8);
		}
		if (carry > 0) {
			_digits.push_back(static_cast<unsigned int>(carry));
		}
	}

	while (!_digits.empty() && _digits.back() == 0) {
		_digits.pop_back();
	}

	if (_digits.empty()) {
		_sign = true;
	}
}

big_int::big_int(pp_allocator<unsigned int>) : _sign(true), _digits(pp_allocator<unsigned int>()) {}

big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) & {
	bool result_sign = (_sign == other._sign);

	if ((_digits.size() == 1 && _digits[0] == 0) || (other._digits.size() == 1 && other._digits[0] == 0)) {
		_digits.clear();
		_digits.push_back(0);
		_sign = true;
		return *this;
	}

	if (rule == multiplication_rule::trivial) {
		std::vector<unsigned int, pp_allocator<unsigned int>> result(_digits.size() + other._digits.size(), 0);

		for (size_t i = 0; i < _digits.size(); ++i) {
			unsigned long long carry = 0;

			for (size_t j = 0; j < other._digits.size() || carry > 0; ++j) {
				unsigned long long current =
				    result[i + j] +
				    static_cast<unsigned long long>(_digits[i]) * (j < other._digits.size() ? other._digits[j] : 0) +
				    carry;

				result[i + j] = static_cast<unsigned int>(current & 0xFFFFFFFF);
				carry = current >> (sizeof(unsigned int) * 8);
			}
		}

		_digits = std::move(result);
	} else if (rule == multiplication_rule::Karatsuba) {
		big_int a = *this;
		big_int b = other;
		a._sign = b._sign = true;

		big_int result = karatsuba_multiply(a, b);
		_digits = std::move(result._digits);
	}

	_sign = result_sign;

	optimize();
	return *this;
}

big_int &big_int::divide_assign(const big_int &other, division_rule rule) & {
	if (other._digits.empty() || (other._digits.size() == 1 && other._digits[0] == 0)) {
		throw std::invalid_argument("Division by zero");
	}

	bool result_sign = (_sign == other._sign);

	big_int dividend = *this;
	big_int divisor = other;
	dividend._sign = divisor._sign = true;

	if (dividend < divisor) {
		_digits.clear();
		_digits.push_back(0);
		_sign = true;
		return *this;
	}

	if (rule == division_rule::trivial) {
		std::vector<unsigned int, pp_allocator<unsigned int>> quotient_digits;
		big_int remainder;

		for (int i = dividend._digits.size() - 1; i >= 0; --i) {
			remainder._digits.push_back(0);

			for (int j = remainder._digits.size() - 1; j > 0; --j) {
				remainder._digits[j] = remainder._digits[j - 1];
			}

			remainder._digits[0] = dividend._digits[i];
			remainder.optimize();

			if (remainder < divisor) {
				quotient_digits.push_back(0);
				continue;
			}

			unsigned int q = 0;
			unsigned int left = 0, right = 0xFFFFFFFF;

			while (left <= right) {
				unsigned int mid = left + (right - left) / 2;
				big_int multiple = divisor;
				multiple.multiply_by_digit(mid);

				if (multiple <= remainder) {
					q = mid;
					left = mid + 1;
				} else {
					right = mid - 1;
				}
			}

			big_int multiple = divisor;
			multiple.multiply_by_digit(q);
			remainder -= multiple;
			remainder.optimize();

			quotient_digits.push_back(q);
		}

		std::reverse(quotient_digits.begin(), quotient_digits.end());

		while (!quotient_digits.empty() && quotient_digits.back() == 0) {
			quotient_digits.pop_back();
		}

		_digits = std::move(quotient_digits);
		_sign = result_sign;

		if (_digits.empty()) {
			_digits.push_back(0);
			_sign = true;
		}
	} else {
		throw std::runtime_error("Non-trivial division rule not implemented");
	}

	optimize();
	return *this;
}

big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) & {
	if (other._digits.size() == 1 && other._digits[0] == 0) {
		throw std::invalid_argument("Modulo by zero");
	}

	if (_digits.size() == 1 && _digits[0] == 0) {
		return *this;
	}

	big_int dividend = *this;
	big_int divisor = other;
	dividend._sign = divisor._sign = true;

	if (dividend < divisor) {
		if (!_sign) {
			if (other._sign) {
				*this = other - *this;
			} else {
			}
		}
		return *this;
	}

	big_int quotient = dividend;
	quotient.divide_assign(divisor, rule);
	big_int product = quotient * divisor;
	big_int remainder = dividend - product;

	remainder._sign = _sign;

	if (!remainder._sign && !(remainder._digits.size() == 1 && remainder._digits[0] == 0)) {
		remainder += other;
	}

	*this = remainder;
	optimize();
	return *this;
}

big_int big_int::karatsuba_multiply(const big_int &a, const big_int &b) const {
	if (a._digits.size() < 2 || b._digits.size() < 2) {
		big_int result = a;
		result.multiply_assign(b, multiplication_rule::trivial);
		return result;
	}

	size_t m = std::max(a._digits.size(), b._digits.size()) / 2;

	big_int low1(std::vector<unsigned int, pp_allocator<unsigned int>>(
	    a._digits.begin(), a._digits.begin() + std::min(m, a._digits.size())));

	big_int high1(std::vector<unsigned int, pp_allocator<unsigned int>>(
	    a._digits.begin() + std::min(m, a._digits.size()), a._digits.end()));

	big_int low2(std::vector<unsigned int, pp_allocator<unsigned int>>(
	    b._digits.begin(), b._digits.begin() + std::min(m, b._digits.size())));

	big_int high2(std::vector<unsigned int, pp_allocator<unsigned int>>(
	    b._digits.begin() + std::min(m, b._digits.size()), b._digits.end()));

	if (high1._digits.empty()) high1._digits.push_back(0);
	if (high2._digits.empty()) high2._digits.push_back(0);

	big_int z0 = karatsuba_multiply(low1, low2);
	big_int z2 = karatsuba_multiply(high1, high2);

	big_int low1_plus_high1 = low1;
	low1_plus_high1 += high1;

	big_int low2_plus_high2 = low2;
	low2_plus_high2 += high2;

	big_int z1 = karatsuba_multiply(low1_plus_high1, low2_plus_high2);
	z1 -= z2;
	z1 -= z0;

	big_int result;
	result._digits.clear();

	result._digits = z0._digits;

	result.plus_assign(z1, m);

	result.plus_assign(z2, 2 * m);

	return result;
}

big_int operator""_bi(unsigned long long n) { return big_int(n); }

void big_int::multiply_by_digit(unsigned int digit) {
	unsigned long long carry = 0;
	for (size_t i = 0; i < _digits.size(); ++i) {
		unsigned long long product = (unsigned long long)_digits[i] * digit + carry;
		_digits[i] = static_cast<unsigned int>(product & 0xFFFFFFFF);
		carry = product >> (sizeof(unsigned int) * 8);
	}
	if (carry != 0) {
		_digits.push_back(static_cast<unsigned int>(carry));
	}
}

unsigned int big_int::divide_by_10() {
	unsigned long long remainder = 0;
	for (auto it = _digits.rbegin(); it != _digits.rend(); ++it) {
		unsigned long long value = (remainder << (sizeof(unsigned int) * 8)) | *it;
		remainder = value % 10;
		*it = static_cast<unsigned int>(value / 10);
	}

	while (!_digits.empty() && _digits.back() == 0) {
		_digits.pop_back();
	}

	return static_cast<unsigned int>(remainder);
}

void big_int::optimize() {
	while (!_digits.empty() && _digits.back() == 0) {
		_digits.pop_back();
	}

	if (_digits.empty()) {
		_sign = true;
		_digits.push_back(0);
	} else if (_digits.size() == 1 && _digits[0] == 0) {
		_sign = true;
	}
}

big_int big_int::abs() const & {
	big_int result = *this;
	result._sign = true;
	return result;
}

big_int big_int::operator-() const & {
	big_int result = *this;
	if (result != big_int(0)) {
		result._sign = !result._sign;
	}
	return result;
}

big_int big_int::operator*(int rhs) const { return *this * big_int(rhs); }

big_int operator*(int lhs, const big_int &rhs) { return big_int(lhs) * rhs; }