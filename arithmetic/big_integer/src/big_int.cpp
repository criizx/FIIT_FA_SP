//
// Created by Des Caldnd on 5/27/2024.
//

#include "../include/big_int.h"

#include <algorithm>
#include <cmath>
#include <compare>
#include <exception>
#include <ranges>
#include <sstream>
#include <string>

void big_int::remove_leading_zeros() {
	while (_digits.size() > 1 && _digits.back() == 0) {
		_digits.pop_back();
	}
	if (_digits.size() == 1 && _digits[0] == 0) {
		_sign = true;
	}
}

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
	if (_sign != other._sign) {
		return _sign ? std::strong_ordering::greater : std::strong_ordering::less;
	}

	const bool is_positive = _sign;
	const size_t lhs_size = _digits.size();
	const size_t rhs_size = other._digits.size();

	if (lhs_size != rhs_size) {
		if (lhs_size < rhs_size) {
			return is_positive ? std::strong_ordering::less : std::strong_ordering::greater;
		} else {
			return is_positive ? std::strong_ordering::greater : std::strong_ordering::less;
		}
	}

	for (size_t i = lhs_size; i-- > 0;) {
		if (_digits[i] != other._digits[i]) {
			if (_digits[i] < other._digits[i]) {
				return is_positive ? std::strong_ordering::less : std::strong_ordering::greater;
			} else {
				return is_positive ? std::strong_ordering::greater : std::strong_ordering::less;
			}
		}
	}

	return std::strong_ordering::equal;
}

big_int::operator bool() const noexcept { return this->_digits[_digits.size() - 1] == 0; }

big_int &big_int::operator++() & {
	*this += big_int(1);
	return *this;
}

big_int big_int::operator++(int) {
	big_int temp(*this);
	++(*this);
	return temp;
}

big_int &big_int::operator--() & {
	*this -= big_int(1);
	return *this;
}

big_int big_int::operator--(int) {
	big_int temp(*this);
	--(*this);
	return temp;
}

big_int &big_int::operator+=(const big_int &other) & {
	if (_sign == other._sign) {
		size_t max_size = std::max(_digits.size(), other._digits.size());
		unsigned int carry = 0;
		_digits.resize(max_size, 0);

		for (size_t i = 0; i < max_size || carry; ++i) {
			if (i == _digits.size()) {
				_digits.push_back(0);
			}
			unsigned int other_digit = (i < other._digits.size()) ? other._digits[i] : 0;
			uint64_t sum = static_cast<uint64_t>(_digits[i]) + other_digit + carry;
			carry = static_cast<unsigned int>(sum >> (sizeof(unsigned int) * 8));
			_digits[i] = static_cast<unsigned int>(sum & 0xFFFFFFFFu);
		}
	} else {
		const big_int &larger = (abs() >= other.abs()) ? *this : other;
		const big_int &smaller = (abs() >= other.abs()) ? other : *this;

		unsigned int borrow = 0;
		_digits = larger._digits;
		_sign = larger._sign;

		for (size_t i = 0; i < _digits.size() || borrow; ++i) {
			if (i == _digits.size()) {
				_digits.push_back(0);
			}
			unsigned int smaller_digit = (i < smaller._digits.size()) ? smaller._digits[i] : 0;
			unsigned int diff = _digits[i] - smaller_digit - borrow;
			borrow = (diff > _digits[i]) ? 1 : 0;
			_digits[i] = diff;
		}

		while (!_digits.empty() && _digits.back() == 0) {
			_digits.pop_back();
		}

		if (_digits.empty()) {
			_digits.push_back(0);
			_sign = true;
		}
	}

	return *this;
}

big_int big_int::abs() const {
	big_int result(*this);
	result._sign = true;
	return result;
}

big_int &big_int::operator-=(const big_int &other) & {
	big_int temp = other;
	temp._sign = !temp._sign;
	return *this += temp;
}

big_int big_int::operator+(const big_int &other) const {
	big_int result(*this);
	return result += other;
}

big_int big_int::operator-(const big_int &other) const {
	big_int result(*this);
	return result -= other;
}

big_int big_int::operator*(const big_int &other) const {
	if (!(*this) || !other) return big_int(0);

	std::vector<unsigned int, pp_allocator<unsigned int>> result;
	result.resize(_digits.size() + other._digits.size(), 0);

	constexpr size_t half_bits = sizeof(unsigned int) * 4;

	constexpr unsigned int half_mask = __detail::generate_half_mask();

	constexpr uint64_t full_mask = (static_cast<uint64_t>(half_mask) << half_bits) | half_mask;

	constexpr size_t full_bits = half_bits * 2;

	for (size_t i = 0; i < _digits.size(); ++i) {
		uint64_t carry = 0;
		for (size_t j = 0; j < other._digits.size() || carry; ++j) {
			uint64_t multiplier = (j < other._digits.size()) ? other._digits[j] : 0;
			uint64_t cur = result[i + j] + static_cast<uint64_t>(_digits[i]) * multiplier + carry;
			result[i + j] = static_cast<unsigned int>(cur & full_mask);
			carry = cur >> full_bits;
		}
	}

	while (result.size() > 1 && result.back() == 0) result.pop_back();

	bool res_sign = (_sign == other._sign);
	return big_int(std::move(result), res_sign);
}

big_int &big_int::operator*=(const big_int &other) & {
	*this = *this * other;
	return *this;
}

big_int big_int::operator/(const big_int &other) const {
	if (other == big_int(0)) throw std::runtime_error("Division by zero");

	big_int dividend = this->abs();
	big_int divisor = other.abs();

	if (dividend < divisor) return big_int(0);

	// n — число цифр делимого (представленных в базе 2^32)
	size_t n = dividend._digits.size();
	// Резервируем вектор для частного той же длины
	big_int quotient;
	quotient._digits.resize(n, 0);
	big_int R(0);  // начальный остаток равен 0

	// Проходим по цифрам от самой старшей (индекс n-1) к самой младшей (индекс 0)
	for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
		// R = R * (2^32) + текущая цифра делимого
		R = R << 32;
		R += big_int(dividend._digits[i]);

		// Находим максимальное q в [0, UINT_MAX], такое что divisor * q <= R.
		unsigned long long left = 0, right = std::numeric_limits<unsigned int>::max();
		unsigned int q_digit = 0;
		while (left <= right) {
			unsigned long long mid = left + (right - left) / 2;
			big_int mid_val(mid);  // конструируем big_int из mid
			big_int candidate = divisor * mid_val;
			if (candidate <= R) {
				q_digit = static_cast<unsigned int>(mid);
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}
		quotient._digits[i] = q_digit;
		R = R - (divisor * big_int(q_digit));
	}

	quotient._sign = (this->_sign == other._sign);
	quotient.remove_leading_zeros();
	return quotient;
}

big_int big_int::operator%(const big_int &other) const {
	if (other == big_int(0)) throw std::runtime_error("Modulo by zero");
	big_int quotient = *this / other;
	big_int product = quotient * other;
	big_int remainder = *this - product;

	if (remainder._digits.size() == 1 && remainder._digits[0] == 0)
		remainder._sign = true;
	else
		remainder._sign = this->_sign;

	return remainder;
}

big_int big_int::operator&(const big_int &other) const {}

big_int big_int::operator|(const big_int &other) const {
	throw not_implemented("big_int big_int::operator|(const big_int &) const", "your code should be here...");
}

big_int big_int::operator^(const big_int &other) const {
	throw not_implemented("big_int big_int::operator^(const big_int &) const", "your code should be here...");
}

big_int big_int::operator<<(size_t shift) const {
	size_t w = sizeof(unsigned int) * 8;
	size_t shift_words = shift / w;
	size_t shift_bits = shift % w;
	std::vector<unsigned int, pp_allocator<unsigned int>> res;
	res.resize(_digits.size() + shift_words + 1, 0);
	uint64_t mask = (static_cast<uint64_t>(1) << w) - 1;
	uint64_t carry = 0;
	for (size_t i = 0; i < _digits.size(); ++i) {
		uint64_t cur = (static_cast<uint64_t>(_digits[i]) << shift_bits) | carry;
		res[i + shift_words] = static_cast<unsigned int>(cur & mask);
		carry = cur >> w;
	}
	res[_digits.size() + shift_words] = static_cast<unsigned int>(carry);
	while (res.size() > 1 && res.back() == 0) res.pop_back();
	return big_int(std::move(res), _sign);
}

big_int big_int::operator>>(size_t shift) const {
	size_t w = sizeof(unsigned int) * 8;
	size_t shift_words = shift / w;
	size_t shift_bits = shift % w;
	if (shift_words >= _digits.size()) return big_int(0);
	std::vector<unsigned int, pp_allocator<unsigned int>> temp(_digits);
	temp.erase(temp.begin(), temp.begin() + shift_words);
	uint64_t mask = (static_cast<uint64_t>(1) << shift_bits) - 1;
	unsigned int carry = 0;
	for (size_t i = temp.size(); i-- > 0;) {
		uint64_t cur = (static_cast<uint64_t>(carry) << w) | temp[i];
		temp[i] = static_cast<unsigned int>(cur >> shift_bits);
		carry = static_cast<unsigned int>(cur & mask);
	}
	while (temp.size() > 1 && temp.back() == 0) temp.pop_back();
	return big_int(std::move(temp), _sign);
}

big_int &big_int::operator%=(const big_int &other) & {
	*this = *this % other;
	return *this;
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
	*this = *this << shift;
	return *this;
}

big_int &big_int::operator>>=(size_t shift) & {
	*this = *this >> shift;
	return *this;
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) & {
	throw not_implemented("big_int &big_int::plus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) & {
	throw not_implemented("big_int &big_int::minus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::operator/=(const big_int &other) & {
	*this = *this / other;
	return *this;
}

std::string big_int::to_string() const {
	if (_digits.size() == 1 && _digits[0] == 0) return "0";

	std::vector<unsigned int, pp_allocator<unsigned int>> temp(_digits);

	constexpr size_t half_bits = sizeof(unsigned int) * 4;
	constexpr unsigned int half_mask = __detail::generate_half_mask();
	constexpr uint64_t full_mask = (static_cast<uint64_t>(half_mask) << half_bits) | half_mask;
	constexpr uint64_t base = full_mask + 1;

	std::string result;

	while (!(temp.size() == 1 && temp[0] == 0)) {
		unsigned int carry = 0;
		for (int i = static_cast<int>(temp.size()) - 1; i >= 0; --i) {
			uint64_t current = static_cast<uint64_t>(carry) * base + temp[i];
			temp[i] = static_cast<unsigned int>(current / 10);
			carry = static_cast<unsigned int>(current % 10);
		}
		result.push_back('0' + carry);
		while (temp.size() > 1 && temp.back() == 0) temp.pop_back();
	}

	std::reverse(result.begin(), result.end());

	if (!_sign) result.insert(result.begin(), '-');

	return result;
}

std::ostream &operator<<(std::ostream &stream, const big_int &value) {
	stream << value.to_string();
	return stream;
}

std::istream &operator>>(std::istream &stream, big_int &value) {
	std::string s;
	stream >> s;
	value = big_int(s);
	return stream;
}

bool big_int::operator==(const big_int &other) const noexcept {
	return (*this <=> other) == std::strong_ordering::equal;
}

bool big_int::operator!=(const big_int &other) const noexcept {
	return (*this <=> other) != std::strong_ordering::equal;
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

big_int::big_int(const std::string &num, unsigned int radix, pp_allocator<unsigned int> alloc)
    : _sign(true), _digits(alloc) {
	if (num.empty()) {
		throw std::invalid_argument("Empty string");
	}
	if (radix < 2 || radix > 36) {
		throw std::invalid_argument("Radix must be between 2 and 36");
	}
	if (!validating_string(num, radix)) {
		throw std::invalid_argument("Invalid characters for the given radix.");
	}

	size_t start_idx = (num[0] == '-') ? 1 : 0;
	_sign = (start_idx == 0);

	if (start_idx >= num.size()) {
		throw std::invalid_argument("Number string is too short.");
	}

	big_int base(radix, alloc);
	big_int value(0, alloc);

	for (size_t i = start_idx; i < num.size(); ++i) {
		char ch = std::toupper(static_cast<unsigned char>(num[i]));
		unsigned int digit = (std::isdigit(ch) ? (ch - '0') : (ch - 'A' + 10));

		value *= base;
		value += big_int(digit, alloc);
	}

	_digits = std::move(value._digits);
	_sign = value._sign;

	if (_digits.size() == 1 && _digits[0] == 0) {
		_sign = true;
	}

	while (!_digits.empty() && _digits.back() == 0) {
		_digits.pop_back();
	}
	if (_digits.empty()) {
		_digits.push_back(0);
		_sign = true;
	}
}

big_int::big_int(pp_allocator<unsigned int> alloc) : _sign(true), _digits({0}, alloc) {}

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