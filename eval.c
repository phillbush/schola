#include <string.h>

#include "simp.h"

#define FORMS                                            \
	X(FORM_AND,             "and"                   )\
	X(FORM_APPLY,           "apply"                 )\
	X(FORM_DEFINE,          "define"                )\
	X(FORM_DO,              "do"                    )\
	X(FORM_EVAL,            "eval"                  )\
	X(FORM_FALSE,           "false"                 )\
	X(FORM_IF,              "if"                    )\
	X(FORM_LAMBDA,          "lambda"                )\
	X(FORM_OR,              "or"                    )\
	X(FORM_QUOTE,           "quote"                 )\
	X(FORM_SET,             "set!"                  )\
	X(FORM_TRUE,            "true"                  )

#define BUILTINS                                                 \
	/* SYMBOL            FUNCTION        NARGS   VARIADIC */ \
	X("boolean?",        f_booleanp,     1,      false      )\
	X("byte?",           f_bytep,        1,      false      )\
	X("car",             f_car,          1,      false      )\
	X("cdr",             f_cdr,          1,      false      )\
	X("stdin",           f_stdin,        0,      false      )\
	X("stdout",          f_stdout,       0,      false      )\
	X("stderr",          f_stderr,       0,      false      )\
	X("display",         f_display,      1,      true       )\
	X("=",               f_equal,        2,      true       )\
	X("falsep",          f_falsep,       1,      false      )\
	X(">",               f_gt,           2,      true       )\
	X("<",               f_lt,           2,      true       )\
	X("string-new",      f_makestring,   1,      false      )\
	X("vector-new",      f_makevector,   1,      false      )\
	X("environment",     f_envnew,       1,      false      )\
	X("newline",         f_newline,      0,      true       )\
	X("null?",           f_nullp,        1,      false      )\
	X("port?",           f_portp,        1,      false      )\
	X("same?",           f_samep,        2,      false      )\
	X("slice-string",    f_slicestring,  3,      false      )\
	X("slice-vector",    f_slicevector,  3,      false      )\
	X("string-compare",  f_stringcmp,    2,      false      )\
	X("string-length",   f_stringlen,    1,      false      )\
	X("string-ref",      f_stringref,    2,      false      )\
	X("string?",         f_stringp,      1,      false      )\
	X("string-set!",     f_stringset,    3,      false      )\
	X("string->vector",  f_stringvector, 1,      false      )\
	X("symbol?",         f_symbolp,      1,      false      )\
	X("true?",           f_truep,        1,      false      )\
	X("vector",          f_vectorref,    2,      false      )\
	X("vector-ref",      f_vectorref,    2,      false      )\
	X("vector-length",   f_vectorlen,    1,      false      )\
	X("vector-set!",     f_vectorset,    3,      false      )\
	X("write",           f_write,        2,      false      )\
	X("+",               f_add,          2,      false      )\
	X("/",               f_divide,       2,      false      )\
	X("*",               f_multiply,     2,      false      )\
	X("-",               f_subtract,     2,      false      )

enum Forms {
#define X(n, s) n,
	FORMS
#undef  X
};

struct Builtin {
	char *name;
	Simp (*fun)(Simp, Simp);
	SimpSiz nargs;
};

static Simp
typepred(Simp ctx, Simp args, bool (*pred)(Simp, Simp))
{
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	return (*pred)(ctx, obj) ? simp_true() : simp_false();
}

static Simp
f_add(Simp ctx, Simp args)
{
	SimpInt n;
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isnum(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	n = simp_getnum(ctx, a) + simp_getnum(ctx, b);
	return simp_makenum(ctx, n);
}

static Simp
f_bytep(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_isbyte);
}

static Simp
f_car(Simp ctx, Simp args)
{
	SimpSiz size;
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	if (!simp_isvector(ctx, obj))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, obj);
	if (size < 1)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	return simp_getvectormemb(ctx, obj, 0);
}

static Simp
f_cdr(Simp ctx, Simp args)
{
	SimpSiz size;
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	if (!simp_isvector(ctx, obj))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, obj);
	if (size < 1)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	return simp_slicevector(ctx, obj, 1, size - 1);
}

static Simp
f_booleanp(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_isbool);
}

static Simp
f_stdin(Simp ctx, Simp args)
{
	(void)args;
	return simp_contextiport(ctx);
}

static Simp
f_stdout(Simp ctx, Simp args)
{
	(void)args;
	return simp_contextoport(ctx);
}

static Simp
f_stderr(Simp ctx, Simp args)
{
	(void)args;
	return simp_contexteport(ctx);
}

static Simp
f_display(Simp ctx, Simp args)
{
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	return simp_display(ctx, simp_contextoport(ctx), obj);
}

static Simp
f_divide(Simp ctx, Simp args)
{
	SimpInt d;
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isnum(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	d = simp_getnum(ctx, b);
	if (d == 0)
		return simp_makeexception(ctx, ERROR_DIVZERO);
	d = simp_getnum(ctx, a) / d;
	return simp_makenum(ctx, d);
}

static Simp
f_equal(Simp ctx, Simp args)
{
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isnum(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	if (simp_getnum(ctx, a) == simp_getnum(ctx, b))
		return simp_true();
	return simp_false();
}

static Simp
f_falsep(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_isfalse);
}

static Simp
f_gt(Simp ctx, Simp args)
{
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isnum(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	if (simp_getnum(ctx, a) > simp_getnum(ctx, b))
		return simp_true();
	return simp_false();
}

static Simp
f_lt(Simp ctx, Simp args)
{
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isnum(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	if (simp_getnum(ctx, a) < simp_getnum(ctx, b))
		return simp_true();
	return simp_false();
}

static Simp
f_envnew(Simp ctx, Simp args)
{
	Simp env;

	env = simp_getvectormemb(ctx, args, 0);
	return simp_makeenvironment(ctx, env);
}

static Simp
f_makestring(Simp ctx, Simp args)
{
	SimpInt size;
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	if (!simp_isnum(ctx, obj))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getnum(ctx, obj);
	if (size < 0)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	return simp_makestring(ctx, NULL, size);
}

static Simp
f_makevector(Simp ctx, Simp args)
{
	SimpInt size;
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	if (!simp_isnum(ctx, obj))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getnum(ctx, obj);
	if (size < 0)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	return simp_makevector(ctx, size);
}

static Simp
f_multiply(Simp ctx, Simp args)
{
	SimpInt n;
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isnum(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	n = simp_getnum(ctx, a) * simp_getnum(ctx, b);
	return simp_makenum(ctx, n);
}

static Simp
f_newline(Simp ctx, Simp args)
{
	(void)args;
	return simp_printf(ctx, simp_contextoport(ctx), "\n");
}

static Simp
f_nullp(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_isnil);
}

static Simp
f_portp(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_isport);
}

static Simp
f_samep(Simp ctx, Simp args)
{
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	return simp_issame(ctx, a, b) ? simp_true() : simp_false();
}

static Simp
f_slicevector(Simp ctx, Simp args)
{
	Simp v, a, b;
	SimpSiz from, size, capacity;

	v = simp_getvectormemb(ctx, args, 0);
	a = simp_getvectormemb(ctx, args, 1);
	b = simp_getvectormemb(ctx, args, 2);
	if (!simp_isvector(ctx, v))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	if (!simp_isnum(ctx, a))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	if (!simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	from = simp_getnum(ctx, a);
	size = simp_getnum(ctx, b);
	capacity = simp_getcapacity(ctx, v);
	if (from < 0 || from > capacity)
		return simp_makeexception(ctx, ERROR_RANGE);
	if (size < 0 || from + size > capacity)
		return simp_makeexception(ctx, ERROR_RANGE);
	return simp_slicevector(ctx, v, from, size);
}

static Simp
f_slicestring(Simp ctx, Simp args)
{
	Simp v, a, b;
	SimpSiz from, size, capacity;

	v = simp_getvectormemb(ctx, args, 0);
	a = simp_getvectormemb(ctx, args, 1);
	b = simp_getvectormemb(ctx, args, 2);
	if (!simp_isstring(ctx, v))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	if (!simp_isnum(ctx, a))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	if (!simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	from = simp_getnum(ctx, a);
	size = simp_getnum(ctx, b);
	capacity = simp_getcapacity(ctx, v);
	if (from < 0 || from > capacity)
		return simp_makeexception(ctx, ERROR_RANGE);
	if (size < 0 || from + size > capacity)
		return simp_makeexception(ctx, ERROR_RANGE);
	return simp_slicestring(ctx, v, from, size);
}

static Simp
f_subtract(Simp ctx, Simp args)
{
	SimpInt n;
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isnum(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	n = simp_getnum(ctx, a) - simp_getnum(ctx, b);
	return simp_makenum(ctx, n);
}

static Simp
f_stringcmp(Simp ctx, Simp args)
{
	SimpSiz size0, size1;
	Simp a, b;
	int cmp;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isstring(ctx, a) || !simp_isstring(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size0 = simp_getsize(ctx, a);
	size1 = simp_getsize(ctx, b);
	cmp = memcmp(
		simp_getstring(ctx, a),
		simp_getstring(ctx, b),
		size0 < size1 ? size0 : size1
	);
	if (cmp == 0 && size0 != size1)
		cmp = size0 < size1 ? -1 : +1;
	else if (cmp < 0)
		cmp = -1;
	else if (cmp > 0)
		cmp = +1;
	return simp_makenum(ctx, cmp);
}

static Simp
f_stringlen(Simp ctx, Simp args)
{
	SimpInt size;
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	if (!simp_isstring(ctx, obj))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, obj);
	return simp_makenum(ctx, size);
}

static Simp
f_stringref(Simp ctx, Simp args)
{
	SimpSiz size;
	SimpInt pos;
	Simp a, b;
	unsigned u;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isstring(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, a);
	pos = simp_getnum(ctx, b);
	if (pos < 0 || pos >= (SimpInt)size)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	u = simp_getstringmemb(ctx, a, pos);
	return simp_makebyte(ctx, u);
}

static Simp
f_stringp(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_isstring);
}

static Simp
f_stringvector(Simp ctx, Simp args)
{
	Simp vector, str, byte;
	SimpSiz i, size;
	unsigned char u;

	str = simp_getvectormemb(ctx, args, 0);
	if (!simp_isstring(ctx, str))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, str);
	vector = simp_makevector(ctx, size);
	if (simp_isexception(ctx, vector))
		return vector;
	for (i = 0; i < size; i++) {
		u = simp_getstringmemb(ctx, str, i);
		byte = simp_makebyte(ctx, u);
		if (simp_isexception(ctx, byte))
			return byte;
		simp_setvector(ctx, vector, i, byte);
	}
	return vector;
}

Simp
f_stringset(Simp ctx, Simp args)
{
	Simp str, pos, val;
	SimpSiz size;
	SimpInt n;
	unsigned char u;

	str = simp_getvectormemb(ctx, args, 0);
	pos = simp_getvectormemb(ctx, args, 1);
	val = simp_getvectormemb(ctx, args, 2);
	if (!simp_isstring(ctx, str) || !simp_isnum(ctx, pos) || !simp_isbyte(ctx, val))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, str);
	n = simp_getnum(ctx, pos);
	u = simp_getbyte(ctx, val);
	if (n < 0 || n >= (SimpInt)size)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	simp_setstring(ctx, str, n, u);
	return simp_void();
}

static Simp
f_symbolp(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_issymbol);
}

static Simp
f_truep(Simp ctx, Simp args)
{
	return typepred(ctx, args, simp_istrue);
}

Simp
f_vectorset(Simp ctx, Simp args)
{
	Simp str, pos, val;
	SimpSiz size;
	SimpInt n;

	str = simp_getvectormemb(ctx, args, 0);
	pos = simp_getvectormemb(ctx, args, 1);
	val = simp_getvectormemb(ctx, args, 2);
	if (!simp_isvector(ctx, str) || !simp_isnum(ctx, pos))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, str);
	n = simp_getnum(ctx, pos);
	if (n < 0 || n >= (SimpInt)size)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	simp_setvector(ctx, str, n, val);
	return simp_void();
}

static Simp
f_vectorlen(Simp ctx, Simp args)
{
	SimpInt size;
	Simp obj;

	obj = simp_getvectormemb(ctx, args, 0);
	if (!simp_isvector(ctx, obj))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, obj);
	return simp_makenum(ctx, size);
}

static Simp
f_vectorref(Simp ctx, Simp args)
{
	SimpSiz size;
	SimpInt pos;
	Simp a, b;

	a = simp_getvectormemb(ctx, args, 0);
	b = simp_getvectormemb(ctx, args, 1);
	if (!simp_isvector(ctx, a) || !simp_isnum(ctx, b))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	size = simp_getsize(ctx, a);
	pos = simp_getnum(ctx, b);
	if (pos < 0 || pos >= (SimpInt)size)
		return simp_makeexception(ctx, ERROR_OUTOFRANGE);
	return simp_getvectormemb(ctx, a, pos);
}

Simp
f_write(Simp ctx, Simp args)
{
	Simp obj, port;

	port = simp_getvectormemb(ctx, args, 0);
	if (!simp_isport(ctx, port))
		return simp_makeexception(ctx, ERROR_ILLTYPE);
	obj = simp_getvectormemb(ctx, args, 1);
	return simp_write(ctx, port, obj);
}

static Simp
defineorset(Simp ctx, Simp expr, Simp env, bool define)
{
	Simp symbol, value;

	if (simp_getsize(ctx, expr) != 3)       /* (define VAR VAL) */
		return simp_makeexception(ctx, ERROR_ARGS);
	symbol = simp_getvectormemb(ctx, expr, 1);
	if (!simp_issymbol(ctx, symbol))
		return simp_makeexception(ctx, ERROR_NOTSYM);
	value = simp_getvectormemb(ctx, expr, 2);
	value = simp_eval(ctx, value, env);
	if (simp_isexception(ctx, value))
		return value;
	if (define)
		value = simp_envdef(ctx, env, symbol, value);
	else
		value = simp_envset(ctx, env, symbol, value);
	if (simp_isexception(ctx, value))
		return value;
	return simp_void();
}

static Simp
set(Simp ctx, Simp expr, Simp env)
{
	return defineorset(ctx, expr, env, false);
}

static Simp
define(Simp ctx, Simp expr, Simp env)
{
	return defineorset(ctx, expr, env, true);
}

Simp
simp_initforms(Simp ctx)
{
	Simp forms, sym;
	SimpSiz i, len;
	unsigned char *formnames[] = {
#define X(n, s) [n] = (unsigned char *)s,
		FORMS
#undef  X
	};

	forms = simp_makevector(ctx, LEN(formnames));
	if (simp_isexception(ctx, forms))
		return forms;
	for (i = 0; i < LEN(formnames); i++) {
		len = strlen((char *)formnames[i]);
		sym = simp_makesymbol(ctx, formnames[i], len);
		if (simp_isexception(ctx, sym))
			return sym;
		simp_setvector(ctx, forms, i, sym);
	}
	return forms;
}

Simp
simp_initbuiltins(Simp ctx)
{
	Simp env, sym, ret, val;
	SimpSiz i, len;
	static Builtin builtins[] = {
#define X(s, p, a, v) { .name = s, .fun = &p, .nargs = a, },
		BUILTINS
#undef  X
	};

	env = simp_contextenvironment(ctx);
	for (i = 0; i < LEN(builtins); i++) {
		len = strlen(builtins[i].name);
		sym = simp_makesymbol(ctx, (unsigned char *)builtins[i].name, len);
		if (simp_isexception(ctx, sym))
			return sym;
		val = simp_makebuiltin(ctx, &builtins[i]);
		if (simp_isexception(ctx, val))
			return val;
		ret = simp_envdef(ctx, env, sym, val);
		if (simp_isexception(ctx, ret))
			return ret;
	}
	return simp_nil();
}

Simp
simp_eval(Simp ctx, Simp expr, Simp env)
{
	Builtin *bltin;
	Simp *forms;
	Simp operator, operands, arguments, extraargs;
	Simp params, var, val;
	SimpSiz noperands, narguments, nextraargs, i;

	forms = simp_getvector(ctx, simp_contextforms(ctx));
loop:
	if (simp_issymbol(ctx, expr))   /* expression is variable */
		return simp_envget(ctx, env, expr);
	if (!simp_isvector(ctx, expr))  /* expression is self-evaluating */
		return expr;
	if ((noperands = simp_getsize(ctx, expr)) == 0)
		return simp_makeexception(ctx, ERROR_EMPTY);
	noperands--;
	operator = simp_getvectormemb(ctx, expr, 0);
	operands = simp_slicevector(ctx, expr, 1, noperands);
	if (simp_issame(ctx, operator, forms[FORM_APPLY])) {
		/* (apply PROC ARG ... ARGS) */
		if (noperands < 2)
			return simp_makeexception(ctx, ERROR_ILLFORM);
		operator = simp_getvectormemb(ctx, operands, 0);
		extraargs = simp_getvectormemb(ctx, operands, noperands - 1);
		extraargs = simp_eval(ctx, extraargs, env);
		if (simp_isexception(ctx, extraargs))
			return extraargs;
		if (!simp_isvector(ctx, extraargs))
			return simp_makeexception(ctx, ERROR_ILLTYPE);
		nextraargs = simp_getsize(ctx, extraargs);
		operands = simp_slicevector(ctx, operands, 1, noperands - 2);
		noperands -= 2;
		goto apply;
	} else if (simp_issame(ctx, operator, forms[FORM_AND])) {
		/* (and EXPRESSION ...) */
		val = simp_true();
		for (i = 0; i < noperands; i++) {
			val = simp_getvectormemb(ctx, operands, i);
			val = simp_eval(ctx, val, env);
			if (simp_isexception(ctx, val))
				return val;
			if (simp_isfalse(ctx, val))
				return val;
		}
		return val;
	} else if (simp_issame(ctx, operator, forms[FORM_OR])) {
		/* (or EXPRESSION ...) */
		val = simp_false();
		for (i = 0; i < noperands; i++) {
			val = simp_getvectormemb(ctx, operands, i);
			val = simp_eval(ctx, val, env);
			if (simp_isexception(ctx, val))
				return val;
			if (simp_istrue(ctx, val))
				return val;
		}
		return val;
	} else if (simp_issame(ctx, operator, forms[FORM_DEFINE])) {
		return define(ctx, expr, env);
	} else if (simp_issame(ctx, operator, forms[FORM_DO])) {
		/* (do EXPRESSION ...) */
		if (noperands == 0)
			return simp_void();
		for (i = 0; i + 1 < noperands; i++) {
			val = simp_getvectormemb(ctx, operands, i);
			val = simp_eval(ctx, val, env);
			if (simp_isexception(ctx, val))
				return val;
		}
		expr = simp_getvectormemb(ctx, operands, i);
		goto loop;
	} else if (simp_issame(ctx, operator, forms[FORM_IF])) {
		/* (if [COND THEN]... [ELSE]) */
		if (noperands < 2)
			return simp_makeexception(ctx, ERROR_ILLFORM);
		for (i = 0; i + 1 < noperands; i++) {
			val = simp_getvectormemb(ctx, operands, i);
			val = simp_eval(ctx, val, env);
			if (simp_isexception(ctx, val))
				return val;
			i++;
			if (simp_istrue(ctx, val)) {
				break;
			}
		}
		if (i < noperands) {
			expr = simp_getvectormemb(ctx, operands, i);
			goto loop;
		}
		return simp_void();
	} else if (simp_issame(ctx, operator, forms[FORM_EVAL])) {
		/* (eval EXPRESSION ENVIRONMENT) */
		if (noperands != 2)
			return simp_makeexception(ctx, ERROR_ILLFORM);
		expr = simp_getvectormemb(ctx, operands, 0);
		env = simp_getvectormemb(ctx, operands, 1);
		goto loop;
	} else if (simp_issame(ctx, operator, forms[FORM_LAMBDA])) {
		/* (lambda PARAMETER BODY) */
		if (noperands != 2)
			return simp_makeexception(ctx, ERROR_ILLFORM);
		expr = simp_getvectormemb(ctx, operands, 1);
		params = simp_getvectormemb(ctx, operands, 0);
		if (!simp_issymbol(ctx, params) &&
		    !simp_isvector(ctx, params))
			return simp_makeexception(ctx, ERROR_ILLFORM);
		return simp_makeclosure(ctx, env, params, expr);
	} else if (simp_issame(ctx, operator, forms[FORM_QUOTE])) {
		/* (quote OBJ) */
		if (noperands != 1)
			return simp_makeexception(ctx, ERROR_ILLFORM);
		return simp_getvectormemb(ctx, operands, 0);
	} else if (simp_issame(ctx, operator, forms[FORM_SET])) {
		return set(ctx, expr, env);
	} else if (simp_issame(ctx, operator, forms[FORM_FALSE])) {
		/* (false) */
		if (noperands != 0)
			return simp_makeexception(ctx, ERROR_ILLFORM);
		return simp_false();
	} else if (simp_issame(ctx, operator, forms[FORM_TRUE])) {
		/* (true) */
		if (noperands != 0)
			return simp_makeexception(ctx, ERROR_ILLFORM);
		return simp_true();
	}
	extraargs = simp_nil();
	nextraargs = 0;
apply:
	/* procedure application */
	operator = simp_eval(ctx, operator, env);
	if (simp_isexception(ctx, operator))
		return operator;
	narguments = noperands + nextraargs;
	arguments = simp_makevector(ctx, narguments);
	if (simp_isexception(ctx, arguments))
		return arguments;
	for (i = 0; i < noperands; i++) {
		/* evaluate arguments */
		val = simp_getvectormemb(ctx, operands, i);
		val = simp_eval(ctx, val, env);
		if (simp_isexception(ctx, val))
			return val;
		simp_setvector(ctx, arguments, i, val);
	}
	for (i = 0; i < nextraargs; i++) {
		/* evaluate extra arguments */
		val = simp_getvectormemb(ctx, extraargs, i);
		val = simp_eval(ctx, val, env);
		if (simp_isexception(ctx, val))
			return val;
		simp_setvector(ctx, arguments, i + noperands, val);
	}
	if (simp_isbuiltin(ctx, operator)) {
		bltin = simp_getbuiltin(ctx, operator);
		if (narguments != bltin->nargs)
			return simp_makeexception(ctx, ERROR_ARGS);
		return (*bltin->fun)(ctx, arguments);
	}
	if (!simp_isclosure(ctx, operator))
		return simp_makeexception(ctx, ERROR_OPERATOR);
	expr = simp_getclosurebody(ctx, operator);
	env = simp_getclosureenv(ctx, operator);
	env = simp_makeenvironment(ctx, env);
	if (simp_isexception(ctx, env))
		return env;
	params = simp_getclosureparam(ctx, operator);
	if (simp_issymbol(ctx, params)) {
		var = simp_envdef(ctx, env, params, arguments);
		if (simp_isexception(ctx, var)) {
			return var;
		}
	} else if (simp_isvector(ctx, params)) {
		if (narguments != simp_getsize(ctx, params))
			return simp_makeexception(ctx, ERROR_ARGS);
		for (i = 0; i < narguments; i++) {
			var = simp_getvectormemb(ctx, params, i);
			val = simp_getvectormemb(ctx, arguments, i);
			var = simp_envdef(ctx, env, var, val);
			if (simp_isexception(ctx, var)) {
				return var;
			}
		}
	} else {
		return simp_makeexception(ctx, ERROR_UNKSYNTAX);
	}
	goto loop;
}
