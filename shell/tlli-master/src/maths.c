#include "maths.h"
#include "tlli_internal.h"

static unsigned char tlli_Which_Arithmetic_Type(int num, tlliValue** args)
{
	int i;
	for(i = 0; i < num; ++i) {
		unsigned char t = args[i]->type;
		if(t == TLLI_VAL_NUM) {
			return t;
		} else if(t != TLLI_VAL_INT) {
			return TLLI_VAL_NIL;
		}
	}
	return TLLI_VAL_INT;
}

tlliValue* tlli_Num_Add(int num, tlliValue** args)
{
	number val = TLLI_NUMBER_ZERO;
	int i;
	for(i = 0; i < num; ++i)
	{
		number v = TLLI_NUMBER_ZERO;
		tlliValueToNumber(args[i], &v);
		val += v;
	}

	tlliValue* rtn;
	tlliNumberToValue(val, &rtn);
	return rtn;
}

tlliValue* tlli_Int_Add(int num, tlliValue** args)
{
	int val = 0;
	int i;
	for(i = 0; i < num; ++i)
	{
		int v = 0;
		tlliValueToInt(args[i], &v);
		val += v;
	}

	tlliValue* rtn;
	tlliIntToValue(val, &rtn);
	return rtn;
}

tlliValue* tlli_Add(int num, tlliValue** args)
{
	if (num < 1) {
		return tlliNil; /* TODO: In future signal some kind of arity error */
	}
	unsigned char t = tlli_Which_Arithmetic_Type(num, args);
	switch(t)
	{
	case TLLI_VAL_NUM:
		return tlli_Num_Add(num, args);
		break;
	case TLLI_VAL_INT:
		return tlli_Int_Add(num, args);
		break;
	default:
		return tlliNil;
		break;
	}
}

tlliValue* tlli_Num_Sub(int num, tlliValue** args)
{
	number val = TLLI_NUMBER_ZERO;
	int i;
	tlliValueToNumber(args[0], &val);
	for(i = 1; i < num; ++i)
	{
		number v = TLLI_NUMBER_ZERO;
		tlliValueToNumber(args[i], &v);
		val -= v;
	}

	tlliValue* rtn;
	tlliNumberToValue(val, &rtn);
	return rtn;
}

tlliValue* tlli_Int_Sub(int num, tlliValue** args)
{
	int val = 0;
	int i;
	tlliValueToInt(args[0], &val);
	for(i = 1; i < num; ++i)
	{
		int v = 0;
		tlliValueToInt(args[i], &v);
		val -= v;
	}

	tlliValue* rtn;
	tlliIntToValue(val, &rtn);
	return rtn;
}

tlliValue* tlli_Sub(int num, tlliValue** args)
{
	if (num < 1) {
		return tlliNil; /* TODO: In future signal some kind of arity error */
	}
	unsigned char t = tlli_Which_Arithmetic_Type(num, args);
	switch(t)
	{
	case TLLI_VAL_NUM:
		return tlli_Num_Sub(num, args);
		break;
	case TLLI_VAL_INT:
		return tlli_Int_Sub(num, args);
		break;
	default:
		return tlliNil;
		break;
	}
}

tlliValue* tlli_Num_Mul(int num, tlliValue** args)
{
	number val = 0;
	int i;
	tlliValueToNumber(args[0], &val);
	for(i = 1; i < num; ++i)
	{
		number v = 0;
		tlliValueToNumber(args[i], &v);
		val *= v;
	}

	tlliValue* rtn;
	tlliNumberToValue(val, &rtn);
	return rtn;
}

tlliValue* tlli_Int_Mul(int num, tlliValue** args)
{
	int val = 0;
	int i;
	tlliValueToInt(args[0], &val);
	for(i = 1; i < num; ++i)
	{
		int v = 0;
		tlliValueToInt(args[i], &v);
		val *= v;
	}

	tlliValue* rtn;
	tlliIntToValue(val, &rtn);
	return rtn;
}

tlliValue* tlli_Mul(int num, tlliValue** args)
{
	if (num < 2) {
		return tlliNil; /* TODO: In future signal some kind of arity error */
	}
	unsigned char t = tlli_Which_Arithmetic_Type(num, args);
	switch(t)
	{
	case TLLI_VAL_NUM:
		return tlli_Num_Mul(num, args);
		break;
	case TLLI_VAL_INT:
		return tlli_Int_Mul(num, args);
		break;
	default:
		return tlliNil;
		break;
	}
}

tlliValue* tlli_Div(int num, tlliValue** args)
{
	if (num < 2) {
		return tlliNil; /* TODO: In future signal some kind of arity error */
	}
	number val = 0;
	int i;
	tlliValueToNumber(args[0], &val);
	for(i = 1; i < num; ++i)
	{
		number v = 0;
		tlliValueToNumber(args[i], &v);
		val /= v;
	}

	tlliValue* rtn;
	tlliNumberToValue(val, &rtn);
	return rtn;
}
