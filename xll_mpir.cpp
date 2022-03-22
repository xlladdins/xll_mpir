// xll_mpir.cpp
#include "xll_mpir.h"

using namespace xll;

#ifdef _DEBUG
template<class MPX>
int test_mp()
{
	//_crtBreakAlloc = 190;
	try {
		{
			MPX z;
			ensure(z == z);
			ensure(!(z != z));
			ensure(!(z < z));
			ensure(z <= z);
			ensure(!(z > z));
			ensure(z >= z);

			MPX z2{ z };
			ensure(z == z2);
			z = z2;
			ensure(z == z2);

			mpir_ui ui = 0;
			MPX z_ui(ui);
			ensure(z == z_ui);

			mpir_si si = 0;
			MPX z_si(si);
			ensure(z == z_si);

			double d = 0;
			MPX z_d(d);
			ensure(z == z_d);

			MPX z_s("0", 10);
			ensure(z == z_s);
		}
		{
			MPX _1(1);
			MPX _2(2);
			MPX _3(3);
			auto z = _1 + _2 * _3;
			ensure(z == MPX(7));
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return -1;
	}

	return 0;
}
int mpz_test = test_mp<mpz>();
int mpf_test = test_mp<mpf>();
#endif // _DEBUG

AddIn xai_string_(
	Function(XLL_HANDLEX, "xll_string_", "\\STRING")
	.Arguments({ Arg(XLL_LPOPER4, "string", "is a string.") })
	.Uncalced()
	.FunctionHelp("Return a handle to a string.")
	.Category(CATEGORY)
);
HANDLEX WINAPI xll_string_(LPOPER4 po)
{
#pragma XLLEXPORT
	handle<std::string> h(new std::string(to_string(*po)));

	return h.get();
}

AddIn xai_string(
	Function(XLL_LPOPER4, "xll_string", "STRING")
	.Arguments({
		Arg(XLL_HANDLEX, "handle", "is a handle to a string."),
		Arg(XLL_WORD, "_wrap", "is the maximum string length. Default is 255."),
		})
	.FunctionHelp("Return a string given a handle.")
	.Category(CATEGORY)
);
LPOPER4 WINAPI xll_string(HANDLEX h, WORD len)
{
#pragma XLLEXPORT
	static OPER4 o;

	handle<std::string> h_(h);
	if (h_) {
		o = to_oper(*h_, len);
	}
	else {
		o = ErrNA4;
	}

	return &o;
}

template<class MPX>
HANDLEX xll_mpx_(const LPOPER4 pstr, LONG base)
{
	HANDLEX h = INVALID_HANDLEX;

	base = base ? base : 10;

	if (pstr->is_str() or pstr->is_multi()) {
		handle<MPX> h_(new MPX(to_string(*pstr), base));
		h = h_.get();
	}
	else if (pstr->is_num()) {
		handle<MPX> z_(pstr->as_num());
		if (z_) { // MPX handle
			handle<MPX> h_(new MPX(*z_));
			h = h_.get();
		}
		else {
			handle<std::string> s_(pstr->as_num());
			if (s_) { // string handle
				handle<MPX> h_(new MPX(s_->data(), base));
				h = h_.get();
			}
			else { // double
				handle<MPX> h_(new MPX(pstr->as_num()));
				h = h_.get();
			}
		}
	}

	return h;
}

AddIn xai_mpz_(
	Function(XLL_HANDLEX, "xll_mpz_", "\\MPZ")
	.Arguments({
		Arg(XLL_LPOPER4, "int", "is an integer as a string or array of strings."),
		Arg(XLL_LONG, "_base", "is the base. Default is 10."),
	})
	.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a handle to a mpz_t.")
);
HANDLEX WINAPI xll_mpz_(const LPOPER4 pstr, LONG base)
{
#pragma XLLEXPORT
	return xll_mpx_<mpz>(pstr, base);
}

AddIn xai_mpf_(
	Function(XLL_HANDLEX, "xll_mpf_", "\\MPF")
	.Arguments({
		Arg(XLL_LPOPER4, "float", "is a floating point number as a string or array of strings."),
		Arg(XLL_LONG, "_base", "is the base. Default is 10."),
		Arg(XLL_WORD, "_prec", "is the optional precision."),
		})
	.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a handle to a mpf_t.")
);
HANDLEX WINAPI xll_mpf_(const LPOPER4 pstr, LONG base, WORD prec)
{
#pragma XLLEXPORT
	base = base ? base : 10;
	if (prec) {
		mpf_set_default_prec(prec);
	}
	return xll_mpx_<mpf>(pstr, base);
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
		o = to_oper(h_->to_string(base));
	}
	else {
		o = ErrNA4;
	}

	return &o;
}

AddIn xai_mpf(
	Function(XLL_LPOPER4, "xll_mpf", "MPF")
	.Arguments({
		Arg(XLL_HANDLEX, "mpf", "is handle returned by \\MPF."),
		Arg(XLL_LONG, "_base", "is the optional the base. Default is 0."),
		})
	.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return mpz as a string of digits in base base.")
);
LPOPER4 xll_mpf(HANDLEX h, LONG base)
{
#pragma XLLEXPORT
	static OPER4 o;

	base = base ? base : 10;
	handle<mpf> h_(h);
	if (h_) {
		o = to_oper(h_->to_string(base));
	}
	else {
		o = ErrNA4;
	}

	return &o;
}

// substitute key-value pairs in o
inline OPER sub(const OPER& o, const OPER& kv, const OPER& l = "{{", const OPER& r = "}}")
{
	ensure(o.is_str());
	ensure(kv.size() % 2 == 0);
	ensure(l.is_str());
	ensure(r.is_str());

	OPER o_(o);
	o_.resize(o.size() / 2, 2);

	for (unsigned i = 0; i < kv.rows(); ++i) {
		ensure(kv(i, 0).is_str());
		ensure(kv(i, 1).is_str());
		OPER k = l & kv(i, 0) & r;
		OPER v = kv(i, 1);
		o_ = Excel(xlfSubstitute, o_, k, v);
	}

	return o_;
}

inline OPER register_mpx_op(const OPER4& X, const OPER4& OP)
{
	OPER4 procedure = OPER4("_xll_mp") & X & OPER4("_") & OP;
	OPER4 functionText = OPER4("MP") & X & OPER4(".") & OP;
	functionText = Excel4(xlfUpper, functionText);

	Args args(XLL_HANDLEX, procedure.as_cstr(), functionText.to_string().c_str());
	auto x1 = X & OPER4("1");
	auto x2 = X & OPER4("2");
	Arg arg1(XLL_HANDLEX, x1.as_cstr(), "is a handle.");
	Arg arg2(XLL_HANDLEX, x2.as_cstr(), "is a handle.");
	args.Arguments({ arg1, arg2 });
	args.Category(CATEGORY);
	auto help = OPER4("Return ") & OP & OPER4("(") & x1 & OPER4(", ") & x2 & OPER4(").");
	args.FunctionHelp(help.as_cstr());

	return Register(args);
}
Auto<OpenAfter> xao_reg([]() {
	OPER o;

	for (const auto& X : { "z", "q", "f" }) {
		for (const auto& OP : { "add", "sub", "mul", "div" }) {
			register_mpx_op(OPER4(X), OPER4(OP));
		}
	}

	return TRUE;
	});

// in-place operations
#define mpx_op(X, OP) \
extern "C" __declspec(dllexport) HANDLEX xll_mp##X##_##OP(HANDLEX h1, HANDLEX h2) \
{ handle<mp##X> h1_(h1); handle<mp##X> h2_(h2); \
  if (h1_ and h2_) mp##X##_##OP(*h1_, *h1_, *h2_); \
  else return 0; return h1; }

// return new object
#define mpx_op_(X, OP) \
extern "C" __declspec(dllexport) HANDLEX xll_mp##X##_##OP##_(HANDLEX h1, HANDLEX h2) \
{ handle<mp##X> h_(new mp##X()); \
  handle<mp##X> h1_(h1); handle<mp##X> h2_(h2); \
  if (h1_ and h2_) mp##X##_##OP(*h_, *h1_, *h2_); \
  else return 0; return h_.get(); }

mpx_op(z, add)
mpx_op(z, sub)
mpx_op(z, mul)
mpx_op(z, div)

mpx_op(q, add)
mpx_op(q, sub)
mpx_op(q, mul)
mpx_op(q, div)

mpx_op(f, add)
mpx_op(f, sub)
mpx_op(f, mul)
mpx_op(f, div)

mpx_op_(z, add)
mpx_op_(z, sub)
mpx_op_(z, mul)
mpx_op_(z, div)
