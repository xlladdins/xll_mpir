// xll_mpir.cpp
#include "xll_mpir.h"

using namespace xll;

class mpz {
	mpz_t z;
public:
	mpz()
	{
		mpz_init(z);
	}
	mpz(const char* str, int base)
		: mpz()
	{
		mpz_set_str(z, str, base);
	}
	mpz(const mpz& z_)
		: mpz()
	{
		mpz_init_set(z, z_);
	}
	mpz& operator=(const mpz& z_)
	{
		if (this != &z_) {
			mpz_clear(z);
			mpz_init_set(z, z_);
		}

		return *this;
	}
	~mpz()
	{
		mpz_clear(z);
	}
	operator mpz_t&()
	{
		return z;
	}
	operator const mpz_t&() const
	{
		return z;
	}
};

inline OPER4 from_mpz(const mpz_t& z, int base)
{
	OPER4 o;

	std::string str;
	size_t n = mpz_sizeinbase(z, base) + 2;
	str.reserve(n);
	mpz_get_str(str.data(), base, z);
	o.resize(static_cast<unsigned>(1 + (n - 1) / 255), 1);
	for (unsigned i = 0; i < o.rows(); ++i) {
		int m = n >= 255 ? 255 : static_cast<int>(n);
		o[i] = OPER4(str.data() + i * 255, m);
		n -= 255;
	}

	return o;
}

inline mpz to_mpz(const OPER4& o, int base)
{
	std::string str;
	for (unsigned i = 0; i < o.size(); ++i) {
		if (!o[i].is_str()) {
			return mpz{}; // 0
		}
		str.append(o[i].val.str + 1, o[i].val.str[0]);
	}

	return mpz(str.c_str(), base);
}

AddIn xai_mpz_(
	Function(XLL_HANDLEX, "xll_mpz_", "\\MPZ")
	.Arguments({
		Arg(XLL_LPOPER4, "int", "is an integer as a string or array of strings."),
		Arg(XLL_LONG, "_base", "is the base. Default is 0."),
	})
	.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a handle to a mpz_t.")
);
HANDLEX WINAPI xll_mpz_(const LPOPER4 pstr, LONG base)
{
#pragma XLLEXPORT
	handle<mpz> h(new mpz(to_mpz(*pstr, base)));

	return h.get();
}

AddIn xai_mpz(
	Function(XLL_LPOPER4, "xll_mpz", "MPZ")
	.Arguments({
		Arg(XLL_HANDLEX, "mpz", "is handle returned by \\MPZ."),
		Arg(XLL_LONG, "_base", "is the base. Default is 0."),
		})
	.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return mpz as a string of digits in base base.")
);
LPOPER4 xll_mpz(HANDLEX h, LONG base)
{
#pragma XLLEXPORT
	static OPER4 o;

	handle<mpz> h_(h);
	if (h_) {
		if (base == 0) {
			base = 10;
		}
		o = from_mpz(*h_, base);
	}
	else {
		o = ErrNA4;
	}

	return &o;
}

inline OPER register_mpz_op(const std::string& op, const std::string& help)
{
	std::string cname("_xll_mpz_");
	cname.append(op);

	std::string xlname("MPZ.");
	xlname.append(op);
	std::transform(xlname.begin(), xlname.end(), xlname.begin(), [](int c) { return (unsigned char)::toupper(c); });

	Args args(XLL_HANDLEX, cname.c_str(), xlname.c_str());
	args.Arguments({
		Arg(XLL_HANDLEX, "mpz1", "is a handle returned by \\MPZ."),
		Arg(XLL_HANDLEX, "mpz2", "is a handle returned by \\MPZ."),
		});
	args.Category(CATEGORY);
	args.FunctionHelp(help.c_str());

	return Register(args);
}
Auto<Open> xao_reg([]() {
	OPER o;

	o = register_mpz_op("add", "Return mpz1 added to mpz2.");
	o = register_mpz_op("mul", "Return mpz1 multiplied by mpz2.");
	o = register_mpz_op("sub", "Return mpz1 subtracted by mpz2.");

	return TRUE;
	});

#define mpz_op(op) \
extern "C" __declspec(dllexport) HANDLEX xll_mpz_##op(HANDLEX h1, HANDLEX h2) \
{ handle<mpz> h1_(h1); handle<mpz> h2_(h2); if (h1_ and h2_) mpz_##op(*h1_, *h1_, *h2_); else return 0; return h1; }

mpz_op(add)
mpz_op(mul)
mpz_op(sub)

AddIn xai_mpz_mul_(
	Function(XLL_HANDLEX, "xll_mpz_mul_", "\\MPZ.MUL")
	.Arguments({
		Arg(XLL_HANDLEX, "mpz1", "is a handle returned by \\MPZ."),
		Arg(XLL_HANDLEX, "mpz2", "is a handle returned by \\MPZ."),
		})
	.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a new handle to mpz1 multiplied by mpz2.")
);
HANDLEX xll_mpz_mul_(HANDLEX h1, HANDLEX h2)
{
#pragma XLLEXPORT
	handle<mpz> h_(new mpz());
	handle<mpz> h1_(h1);
	handle<mpz> h2_(h2);
	if (h1_ and h2_) {
		mpz_mul(*h_, *h1_, *h2_);
	}
	else {
		return 0;
	}

	return h_.get();
}