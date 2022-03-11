// xll_mpir.cpp
#include "xll_mpir.h"

using namespace xll;

#ifdef _DEBUG
int test_mpz()
{
	//_crtBreakAlloc = 190;
	try {
		{
			mpz z;
			ensure(z == z);
			ensure(!(z != z));
			ensure(!(z < z));
			ensure(z <= z);
			ensure(!(z > z));
			ensure(z >= z);

			mpz z2{ z };
			ensure(z == z2);
			z = z2;
			ensure(z == z2);

			mpir_ui ui = 0;
			mpz z_ui(ui);
			ensure(z == z_ui);

			mpir_si si = 0;
			mpz z_si(si);
			ensure(z == z_si);

			double d = 0;
			mpz z_d(d);
			ensure(z == z_d);

			mpz z_s("0", 10);
			ensure(z == z_s);
		}
		{
			mpz _1(1);
			mpz _2(2);
			mpz _3(3);
			auto z = _1 + _2 * _3;
			ensure(z == mpz(7));
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return -1;
	}

	return 0;
}
int mpz_test = test_mpz();
#endif // _DEBUG

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