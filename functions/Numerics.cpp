
/* -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
 *
 *  $Id: Numerics.cpp,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Author: Paul Pedersen, John Cowan
 *
 */

#include "Numerics.h"
#include "NumericsImpl.h"
#include <iostream>

using namespace std;
namespace xqp {




/*______________________________________________________________________
|  
|	6.2.1 op:numeric-add
|	op:numeric-add($arg1 as numeric, $arg2 as numeric) as numeric
|	
|	Summary: Backs up the "+" operator and returns the arithmetic sum of 
|	its operands: ($arg1 + $arg2).
|	
|	Note:
|	For xs:float or xs:double values, if one of the operands is a zero or 
|	a finite number and the other is INF or -INF, INF or -INF is returned. 
|	If both operands are INF, INF is returned. If both operands are -INF, 
|	-INF is returned. If one of the operands is INF and the other is -INF, 
|	NaN is returned.
|_______________________________________________________________________*/

op_numeric_add::op_numeric_add(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_add::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericAddIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_add::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_add::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==2);
}


op_numeric_add_int::op_numeric_add_int(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_add_int::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericAddIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_add_int::type_check(
	signature& sig) const
{
	return xs_integer;
}

bool op_numeric_add_int::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==2);
}



/*______________________________________________________________________
|  
|	6.2.2 op:numeric-subtract
|	op:numeric-subtract($arg1 as numeric, $arg2 as numeric) as numeric
|	
|	Summary: Backs up the "-" operator and returns the arithmetic 
|	difference of its operands: ($arg1 - $arg2).
|	
|	Note:
|	For xs:float or xs:double values, if one of the operands is a zero or 
|	a finite number and the other is INF or -INF, an infinity of the 
|	appropriate sign is returned. If both operands are INF or -INF, NaN is 
|	returned. If one of the operands is INF and the other is -INF, an 
|	infinity of the appropriate sign is returned.
|_______________________________________________________________________*/

op_numeric_subtract::op_numeric_subtract(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_subtract::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericSubtractIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_subtract::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_subtract::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==2);
}



/*______________________________________________________________________
|  
|	6.2.3 op:numeric-multiply
|	op:numeric-multiply($arg1 as numeric, $arg2 as numeric) as numeric
|	
|	Summary: Backs up the "*" operator and returns the arithmetic product 
|	of its operands: ($arg1 * $arg2).
|	
|	Note:
|	For xs:float or xs:double values, if one of the operands is a zero and 
|	the other is an infinity, NaN is returned. If one of the operands is a 
|	non-zero number and the other is an infinity, an infinity with the 
|	appropriate sign is returned.
|_______________________________________________________________________*/

op_numeric_multiply::op_numeric_multiply(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_multiply::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericMultiplyIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_multiply::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_multiply::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==2);
}



/*______________________________________________________________________
|  
|	6.2.4 op:numeric-divide
|	op:numeric-divide($arg1 as numeric, $arg2 as numeric) as numeric
|	
|	Summary: Backs up the "div" operator and returns the arithmetic 
|	quotient of its operands: ($arg1 div $arg2).
|	
|	As a special case, if the types of both $arg1 and $arg2 are 
|	xs:integer, then the return type is xs:decimal.
|	
|	Notes:
|	For xs:decimal and xs:integer operands, if the divisor is (positive or 
|	negative) zero, an error is raised [err:FOAR0001]. For xs:float and 
|	xs:double operands, floating point division is performed as specified 
|	in [IEEE 754-1985].
|	
|	For xs:float or xs:double values, a positive number divided by 
|	positive zero returns INF. A negative number divided by positive zero 
|	returns -INF. Division by negative zero returns -INF and INF, 
|	respectively. Positive or negative zero divided by positive or 
|	negative zero returns NaN. Also, INF or -INF divided by INF or -INF 
|	returns NaN.
|_______________________________________________________________________*/

op_numeric_divide::op_numeric_divide(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_divide::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericDivideIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_divide::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_divide::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==2);
}



/*______________________________________________________________________
|  
|	6.2.5 op:numeric-integer-divide
|	op:numeric-integer-divide($arg1 as numeric, $arg2 as numeric) as xs:integer
|	
|	Summary: This function backs up the "idiv" operator and performs an 
|	integer division: that is, it divides the first argument by the 
|	second, and returns the integer obtained by truncating the fractional 
|	part of the result. The division is performed so that the sign of the 
|	fractional part is the same as the sign of the dividend.
|	
|	If the dividend, $arg1, is not evenly divided by the divisor, $arg2, 
|	then the quotient is the xs:integer value obtained, ignoring 
|	(truncating) any remainder that results from the division (that is, no 
|	rounding is performed). Thus, the semantics " $a idiv $b " are 
|	equivalent to " ($a div $b) cast as xs:integer " except for error 
|	situations.
|	
|	If the divisor is (positive or negative) zero, then an error is raised 
|	[err:FOAR0001]. If either operand is NaN or if $arg1 is INF or -INF 
|	then an error is raised [err:FOAR0002].
|	
|	Note:
|	The semantics of this function are different from integer division as 
|	defined in programming languages such as Java and C++.
|_______________________________________________________________________*/

op_numeric_integer_divide::op_numeric_integer_divide(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_integer_divide::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericIntegerDivideIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_integer_divide::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_integer_divide::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==2);
}



/*______________________________________________________________________
|  
|	6.2.6 op:numeric-mod
|	op:numeric-mod($arg1 as numeric, $arg2 as numeric) as numeric
|	
|	Summary: Backs up the "mod" operator. Informally, this function 
|	returns the remainder resulting from dividing $arg1, the dividend, by 
|	$arg2, the divisor. The operation a mod b for operands that are 
|	xs:integer or xs:decimal, or types derived from them, produces a 
|	result such that (a idiv b)*b+(a mod b) is equal to a and the 
|	magnitude of the result is always less than the magnitude of b. This 
|	identity holds even in the special case that the dividend is the 
|	negative integer of largest possible magnitude for its type and the 
|	divisor is -1 (the remainder is 0). It follows from this rule that the 
|	sign of the result is the sign of the dividend.
|	
|	For xs:integer and xs:decimal operands, if $arg2 is zero, then an 
|	error is raised [err:FOAR0001].
|	
|	For xs:float and xs:double operands the following rules apply:
|	  * If either operand is NaN, the result is NaN.
|	  * If the dividend is positive or negative infinity, or the divisor 
|			is positive or negative zero (0), or both, the result is NaN.
|	  * If the dividend is finite and the divisor is an infinity, the 
|			result equals the dividend.
|	  * If the dividend is positive or negative zero and the divisor is 
|			finite, the result is the same as the dividend.
|	  * In the remaining cases, where neither positive or negative 
|			infinity, nor positive or negative zero, nor NaN is involved, the 
|			result obeys (a idiv b)*b+(a mod b) = a. Division is truncating 
|			division, analogous to integer division, not [IEEE 754-1985] rounding 
|			division i.e. additional digits are truncated, not rounded to the 
|			required precision.
|_______________________________________________________________________*/

op_numeric_mod::op_numeric_mod(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_mod::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericModIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_mod::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_mod::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==2);
}



/*______________________________________________________________________
|  
|	6.2.7 op:numeric-unary-plus
|	op:numeric-unary-plus($arg as numeric) as numeric
|	
|	Summary: Backs up the unary "+" operator and returns its operand with 
|	the sign unchanged: (+ $arg). Semantically, this operation performs no 
|	operation.
|_______________________________________________________________________*/

op_numeric_unary_plus::op_numeric_unary_plus(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_unary_plus::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return argv[0];
}

sequence_type_t op_numeric_unary_plus::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_unary_plus::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==1);
}



/*______________________________________________________________________
|  
|	6.2.8 op:numeric-unary-minus
|	op:numeric-unary-minus($arg as numeric) as numeric
|	
|	Summary: Backs up the unary "-" operator and returns its operand with 
|	the sign reversed: (- $arg). If $arg is positive, its negative is 
|	returned; if it is negative, its positive is returned.
|	
|	For xs:integer and xs:decimal arguments, 0 and 0.0 return 0 and 0.0, 
|	respectively. For xs:float and xs:double arguments, NaN returns NaN, 
|	0.0E0 returns -0.0E0 and vice versa. INF returns -INF. -INF returns 
|	INF.
|_______________________________________________________________________*/

op_numeric_unary_minus::op_numeric_unary_minus(
	const signature& sig)
:
	function(sig)
{
}

iterator_t op_numeric_unary_minus::operator()(
	yy::location loc, 
	vector<iterator_t>& argv) const
{
	if (!validate_args(argv)) return NULL;
	return new OpNumericUnaryMinusIterator(loc, argv[0]);
}

sequence_type_t op_numeric_unary_minus::type_check(
	signature& sig) const
{
	return xs_decimal;
}

bool op_numeric_unary_minus::validate_args(
	vector<iterator_t>& argv) const
{
	return (argv.size()==1);
}

  
/*______________________________________________________________________
|  
| 6.3 Comparison Operators on Numeric Values
|_______________________________________________________________________*/

// 6.3.1 op:numeric-equal
op_numeric_equal::op_numeric_equal(const signature& sig)
:
function(sig)
{}

iterator_t op_numeric_equal::operator()( yy::location loc, vector<iterator_t>& argv) const {
	if (!validate_args(argv))
		return NULL;
	return new OpNumericEqualIterator(loc, argv[0], argv[1]);
}

sequence_type_t op_numeric_equal::type_check(signature& sig) const {
	return xs_boolean;
}

bool op_numeric_equal::validate_args(vector<iterator_t>& argv) const {
	return (argv.size() == 2);
}

// 6.3.2 op:numeric-less-than
// 6.3.3 op:numeric-greater-than


/*______________________________________________________________________
|  
| 6.4 Functions on Numeric Values
|_______________________________________________________________________*/

// 6.4.1 fn:abs
fn_abs::fn_abs(const signature& sig)
:
	function(sig)
{   	
}

iterator_t fn_abs::operator()( yy::location loc, vector<iterator_t>& argv) const
{
	if (!validate_args(argv))
		return NULL;
	return new FnAbsIterator(loc, argv[0]);
}

sequence_type_t fn_abs::type_check(signature& sig) const
{
	return xs_decimal;
}

bool fn_abs::validate_args(vector<iterator_t>& argv) const
{
	return (argv.size() == 1);
}

// 6.4.2 fn:ceiling
// 6.4.3 fn:floor
// 6.4.4 fn:round
// 6.4.5 fn:round-half-to-even



  
} /* namespace xqp */

