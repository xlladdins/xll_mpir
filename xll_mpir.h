// xll_mpir.h - MPIR Excel header
#pragma once
#include "mpir/mpir.h"
#include "xll/xll/xll.h"

#ifndef CATEGORY
#define CATEGORY "MPIR"
#endif

// multi of strings to string
inline std::string to_string(const xll::OPER4& o)
{
	std::string s;

	for (const auto& oi : o) {
		if (!oi.is_str()) {
			return std::string{};
		}
		s.append(oi.val.str + 1, oi.val.str[0]);
	}

	return s;

}
// string to one column OPER of strings
inline xll::OPER4 to_oper(const std::string& s, unsigned len = 255)
{
	unsigned n = static_cast<unsigned>(s.length());
	xll::OPER4 o(1 + (n - 1)/len, 1);

	for (unsigned i = 0; i < o.rows(); ++i) {
		o[i] = xll::OPER4(s.data() + i * len, std::min(n, len));
		n -= len;
	}

	return o;
}

class mpz {
	mpz_t z;
public:
	mpz()
	{
		mpz_init(z);
	}
	explicit mpz(int i)
	{
		mpz_init_set_si(z, i);
	}
	explicit mpz(mpir_ui ui)
	{
		mpz_init_set_ui(z, ui);
	}
	explicit mpz(mpir_si si)
	{
		mpz_init_set_si(z, si);
	}
	explicit mpz(double d)
	{
		mpz_init_set_d(z, d);
	}
	mpz(const char* str, int base)
	{
		mpz_init_set_str(z, str, base);
	}
	mpz(const mpz& z_)
	{
		mpz_init_set(z, z_);
	}
	mpz(mpz&& z_) noexcept
		: mpz()
	{
		mpz_swap(z, z_);
		mpz_clear(z_);
	}
	mpz& operator=(const mpz& z_)
	{
		if (this != &z_) {
			mpz_clear(z);
			mpz_init_set(z, z_);
		}

		return *this;
	}
	mpz& operator=(mpz&& z_) noexcept
	{
		if (this != &z_) {
			mpz_swap(z, z_);
			mpz_clear(z_);
		}

		return *this;
	}
	~mpz()
	{
		mpz_clear(z);
	}
	operator mpz_t& ()
	{
		return z;
	}
	operator const mpz_t& () const
	{
		return z;
	}
	void swap(mpz& z_)
	{
		mpz_swap(z, z_);
	}

	// spaceship needs some help
	bool operator==(const mpz& z_) const { return mpz_cmp(z, z_) == 0; }
	int operator<=>(const mpz& z_) const { return mpz_cmp(z, z_); }

	mpz& operator+=(const mpz& z_) { mpz_add(z, z, z_); return *this; }
	mpz& operator-=(const mpz& z_) { mpz_sub(z, z, z_); return *this; }
	mpz& operator*=(const mpz& z_) { mpz_mul(z, z, z_); return *this; }
	mpz& operator/=(const mpz& z_) { mpz_div(z, z, z_); return *this; }
};

//inline auto operator<=>(const mpz& z, const mpz& z_) { return z.operator<=>(z_); }

inline mpz operator+(mpz z, const mpz& z_) { return z += z_; }
inline mpz operator-(mpz z, const mpz& z_) { return z -= z_; }
inline mpz operator*(mpz z, const mpz& z_) { return z *= z_; }
inline mpz operator/(mpz z, const mpz& z_) { return z /= z_; }

inline xll::OPER4 from_mpz(const mpz_t& z, int base)
{
	std::string s(mpz_sizeinbase(z, base) + 2, 0);
	mpz_get_str(s.data(), base, z);

	return to_oper(s);
}

inline mpz to_mpz(const xll::OPER4& o, int base)
{
	return mpz(to_string(o).c_str(), base);
}

class mpq {
	mpq_t q;
public:
	mpq()
	{
		mpq_init(q);
	}
	mpq(char* str, int base)
		: mpq()
	{
		mpq_set_str(q, str, base);
	}
	mpq(const mpq& q_)
		: mpq()
	{
		mpq_set(q, q_);
	}
	mpq(const mpz& z_)
		: mpq()
	{
		mpq_set_z(q, z_);
	}
	~mpq()
	{
		mpq_clear(q);
	}
	operator mpq_t& ()
	{
		return q;
	}
	operator const mpq_t& () const
	{
		return q;
	}

	void swap(mpq& q_)
	{
		mpq_swap(q, q_);
	}
	const auto& num() const
	{
		return *mpq_numref(q);
	}
	const auto& den() const
	{
		return *mpq_denref(q);
	}
};

//mpz_sizeinbase(mpq_numref(op), base) + mpz_sizeinbase(mpq_denref(op), base) + 3