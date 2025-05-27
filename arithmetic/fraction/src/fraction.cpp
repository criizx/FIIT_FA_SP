#include "../include/fraction.h"

void fraction::optimise() {
	if (_numerator == 0) {
		_denominator = 1;
		return;
	}
	const big_int divisor = gcd(_numerator.abs(), _denominator.abs());
	_numerator /= divisor;
	_denominator /= divisor;
	if (_denominator < 0) {
		_numerator = -_numerator;
		_denominator = -_denominator;
	}
}

fraction::fraction(pp_allocator<big_int::value_type> alloc) : _numerator(0, alloc), _denominator(1, alloc) {}

fraction &fraction::operator+=(const fraction &rhs) & {
	if (_denominator == rhs._denominator) {
		_numerator += rhs._numerator;
	} else {
		big_int common = lcm(_denominator, rhs._denominator);
		_numerator = _numerator * (common / _denominator) + rhs._numerator * (common / rhs._denominator);
		_denominator = common;
	}
	optimise();
	return *this;
}

fraction fraction::operator+(const fraction &rhs) const {
	fraction result(*this);
	result += rhs;
	return result;
}

fraction &fraction::operator-=(const fraction &rhs) & {
	if (_denominator == rhs._denominator) {
		_numerator -= rhs._numerator;
	} else {
		big_int common = lcm(_denominator, rhs._denominator);
		_numerator = _numerator * (common / _denominator) - rhs._numerator * (common / rhs._denominator);
		_denominator = common;
	}
	optimise();
	return *this;
}

fraction fraction::operator-(const fraction &rhs) const {
	fraction result(*this);
	result -= rhs;
	return result;
}

fraction &fraction::operator*=(const fraction &rhs) & {
	_numerator *= rhs._numerator;
	_denominator *= rhs._denominator;
	optimise();
	return *this;
}

fraction fraction::operator*(const fraction &rhs) const {
	fraction result(*this);
	result *= rhs;
	return result;
}

fraction operator*(int lhs, const fraction &rhs) { return fraction(lhs) * rhs; }

fraction &fraction::operator/=(const fraction &rhs) & {
	if (rhs._numerator == 0) {
		throw std::invalid_argument("Division by zero");
	}
	_numerator *= rhs._denominator;
	_denominator *= rhs._numerator;
	optimise();
	return *this;
}

fraction fraction::operator/(const fraction &rhs) const {
	fraction result(*this);
	result /= rhs;
	return result;
}

bool fraction::operator==(const fraction &rhs) const noexcept {
	return _numerator == rhs._numerator && _denominator == rhs._denominator;
}

std::partial_ordering fraction::operator<=>(const fraction &rhs) const noexcept {
	big_int left = _numerator * rhs._denominator;
	big_int right = rhs._numerator * _denominator;
	bool same_sign = (_denominator > 0 && rhs._denominator > 0) || (_denominator < 0 && rhs._denominator < 0);
	if (same_sign) return left <=> right;
	return (left > right)   ? std::partial_ordering::less
	       : (left < right) ? std::partial_ordering::greater
	                        : std::partial_ordering::equivalent;
}

std::ostream &operator<<(std::ostream &os, const fraction &obj) {
	return os << obj._numerator << (obj._denominator == 1 ? "" : "/" + obj._denominator.to_string());
}

std::istream &operator>>(std::istream &is, fraction &obj) {
	std::string input;
	is >> input;
	size_t slash = input.find('/');
	if (slash != std::string::npos) {
		big_int num(input.substr(0, slash));
		big_int den(input.substr(slash + 1));
		if (den == 0) {
			is.setstate(std::ios::failbit);
			return is;
		}
		obj._numerator = num;
		obj._denominator = den;
	} else {
		try {
			obj._numerator = big_int(input);
			obj._denominator = 1;
		} catch (...) {
			is.setstate(std::ios::failbit);
			return is;
		}
	}
	obj.optimise();
	return is;
}

std::string fraction::to_string() const {
	return _denominator == 1 ? _numerator.to_string() : _numerator.to_string() + "/" + _denominator.to_string();
}
fraction fraction::sin(const fraction &epsilon) const {
	fraction x = *this;
	fraction result = x;
	fraction term = x;
	big_int k = 1;

	while (term.abs() > epsilon) {
		term *= -x * x;
		term /= fraction((2 * k) * (2 * k + 1));
		result += term;
		++k;
	}
	return result;
}

fraction fraction::cos(const fraction &epsilon) const {
	fraction x = *this;
	fraction result = fraction(1);
	fraction term = fraction(1);
	big_int k = 1;

	while (term.abs() > epsilon) {
		term *= -x * x;
		term /= fraction((2 * k - 1) * (2 * k));
		result += term;
		++k;
	}
	return result;
}

fraction fraction::tg(const fraction &epsilon) const {
	fraction cosine = cos(epsilon);
	if (cosine == fraction(0)) {
		throw std::domain_error("Tangent undefined at this point");
	}
	return sin(epsilon) / cosine;
}

fraction fraction::ctg(const fraction &epsilon) const {
	fraction sine = sin(epsilon);
	if (sine == fraction(0)) {
		throw std::domain_error("Cotangent undefined at this point");
	}
	return cos(epsilon) / sine;
}

fraction fraction::sec(const fraction &epsilon) const {
	fraction cosine = cos(epsilon);
	if (cosine == fraction(0)) {
		throw std::domain_error("Secant undefined at this point");
	}
	return fraction(1) / cosine;
}

fraction fraction::cosec(const fraction &epsilon) const {
	fraction sine = sin(epsilon);
	if (sine == fraction(0)) {
		throw std::domain_error("Cosecant undefined at this point");
	}
	return fraction(1) / sine;
}

fraction fraction::arcsin(const fraction &epsilon) const {
	if (abs() > fraction(1)) {
		throw std::invalid_argument("arcsin domain is [-1, 1]");
	}

	fraction x = *this;
	fraction sum = x;
	fraction term = x;
	big_int n = 1;

	while (true) {
		fraction coef((2 * n - 1) * (2 * n - 1), 2 * n * (2 * n + 1));
		term *= x * x * coef;
		if (term.abs() <= epsilon) break;
		sum += term;
		++n;
	}

	return sum;
}

fraction fraction::arccos(const fraction &epsilon) const {
	if (*this == fraction(1)) return fraction(0);
	if (*this == fraction(-1)) {
		return arcsin(fraction(1) - epsilon) * fraction(2);
	}
	return fraction(1).arcsin(epsilon) * fraction(2) - arcsin(epsilon);
}

fraction fraction::arctg(const fraction &epsilon) const {
	fraction x = *this;
	bool reciprocal = false;
	if (x.abs() > fraction(1)) {
		x = fraction(1) / x;
		reciprocal = true;
	}

	fraction sum = x;
	fraction term = x;
	big_int n = 1;

	while (true) {
		term *= -x * x;
		fraction next = term / fraction(2 * n + 1);
		if (next.abs() <= epsilon) break;
		sum += next;
		++n;
	}

	if (reciprocal) {
		fraction pi_2 = fraction(1).arcsin(fraction(1_bi, 1000000_bi)) * fraction(2);
		return pi_2 - sum;
	}

	return sum;
}

fraction fraction::arcctg(const fraction &epsilon) const {
	return fraction(1).arcsin(fraction(1_bi, 1000000_bi)) * fraction(2) - arctg(epsilon);
}

fraction fraction::arcsec(const fraction &epsilon) const {
	if (abs() < fraction(1)) {
		throw std::invalid_argument("arcsec domain is |x| >= 1");
	}
	return (fraction(1) / *this).arccos(epsilon);
}

fraction fraction::arccosec(const fraction &epsilon) const {
	if (abs() < fraction(1)) {
		throw std::invalid_argument("arccosec domain is |x| >= 1");
	}
	return (fraction(1) / *this).arcsin(epsilon);
}

fraction fraction::pow(size_t exponent) const {
	if (exponent == 0) {
		return fraction(1);
	}

	fraction base = *this;
	bool is_negative_exponent = (exponent < 0);

	if (is_negative_exponent) {
		base = fraction(_denominator, _numerator);
		exponent = -exponent;
	}

	fraction result(1);
	while (exponent > 0) {
		if (exponent & 1) {
			result *= base;
		}
		base *= base;
		exponent >>= 1;
	}

	return result;
}

fraction fraction::root(size_t degree, const fraction &epsilon) const {
	if (degree == 0) throw std::invalid_argument("Zero-degree root undefined");
	if (*this < fraction(0) && degree % 2 == 0) throw std::invalid_argument("Even root of negative number");
	if (*this == fraction(0)) return fraction(0);

	fraction x_prev(1);
	fraction x_next;
	fraction val = abs();

	while (true) {
		fraction denom = x_prev.pow(degree - 1);
		x_next = ((x_prev * fraction(degree - 1)) + (val / denom)) / fraction(degree);
		if ((x_next - x_prev).abs() / x_next.abs() <= epsilon) break;
		x_prev = x_next;
	}

	return x_next;
}

fraction fraction::ln(const fraction &epsilon) const {
	if (*this <= fraction(0)) {
		throw std::invalid_argument("ln undefined for non-positive values");
	}

	fraction x = *this;
	fraction y = (x - 1) / (x + 1);
	fraction y2 = y * y;
	fraction term = y;
	fraction sum = term;
	big_int k = 1;

	while (true) {
		term *= y2;
		fraction add = term / fraction(2 * k + 1);
		if (add.abs() <= epsilon) break;
		sum += add;
		++k;
	}

	return sum * fraction(2);
}

fraction fraction::log2(const fraction &epsilon) const { return ln(epsilon) / fraction(2).ln(epsilon); }

fraction fraction::lg(const fraction &epsilon) const { return ln(epsilon) / fraction(10).ln(epsilon); }

big_int gcd(big_int a, big_int b) {
	if (a == 0) return b;
	if (b == 0) return a;

	a = a < 0 ? -a : a;
	b = b < 0 ? -b : b;

	while (b) {
		big_int temp = b;
		b = a % b;
		a = temp;
	}

	return a;
}

big_int lcm(const big_int &a, const big_int &b) {
	if (a == 0 || b == 0) {
		return 0;
	}

	big_int abs_a = a < 0 ? -a : a;
	big_int abs_b = b < 0 ? -b : b;

	return (abs_a / gcd(abs_a, abs_b)) * abs_b;
}

fraction fraction::abs() const { return fraction(_numerator.abs(), _denominator.abs()); }

fraction fraction::operator-() const { return fraction(-_numerator, _denominator); }
