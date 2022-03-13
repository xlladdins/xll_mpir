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
	HANDLEX h = INVALID_HANDLEX;

	base = base ? base : 10;
	if (pstr->is_num()) {
		handle<mpz> z_(pstr->as_num());
		if (z_) {
			handle<mpz> h_(new mpz(*z_));
			h = h_.get();
		}
		else {
			handle<std::string> s_(pstr->as_num());
			if (s_) {
				handle<mpz> h_(new mpz(s_->data(), base));
				h = h_.get();
			}
			else {
				handle<mpz> h_(new mpz(pstr->as_num()));
				h = h_.get();
			}
		}
	}
	else if (pstr->is_str() || pstr->is_multi()) {
		handle<mpz> h_(new mpz(to_string(*pstr), base));
		h = h_.get();
	}

	return h;
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

// substitute key-value pairs in o
inline OPER sub(const OPER& o, const OPER& kv, const OPER& l = "{{", const OPER& r = "}}")
{
	ensure(o.is_str());
	ensure(kv.columns() == 2);
	ensure(l.is_str());
	ensure(r.is_str());

	OPER o_(o);

	for (unsigned i = 0; i < kv.rows(); ++i) {
		ensure(kv(i, 0).is_str());
		ensure(kv(i, 1).is_str());
		OPER k = l & kv(i, 0) & r;
		OPER v = kv(i, 1);
		o_ = Excel(xlfSubstitute, o_, k, v);
	}

	return o_;
}

inline OPER register_mpz_op(const OPER& kv)
{
	OPER4 procedure = sub(OPER("_xll_mp{{X}}{{OP}}"), kv);
	OPER4 functionText = sub(OPER("MP{{X}}.{{OP}}"), kv);
	functionText = Excel(xlfUpper, functionText);

	Args args(XLL_HANDLEX, procedure.as_cstr(), functionText.as_cstr());
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

	for (const auto& X : OPER({ "z", "q", "f" })) {
		for (const auto& )
	}

	o = register_mpz_op("add", "Return mpz1 added to mpz2.");
	o = register_mpz_op("mul", "Return mpz1 multiplied by mpz2.");
	o = register_mpz_op("sub", "Return mpz1 subtracted by mpz2.");

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

mpx_op_(z, add)
mpx_op_(z, sub)
mpx_op_(z, mul)
mpx_op_(z, div)

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
