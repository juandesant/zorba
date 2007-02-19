/* -*- mode: c++; indent-tabs-mode: nil -*-
 *
 *  $Id: plan_visitor.h,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Author: John Cowan, Paul Pedersen
 *
 */

#ifndef XQP_PLAN_VISITOR_H
#define XQP_PLAN_VISITOR_H

#include "expr_visitor.h"

/*______________________________________________________________________
|  
|	 Design note: Visitor pattern.  See, for example:
|	 "Modern C++ Design" by Andrei Alexandrescu,
|  Addison Wesley (2001), Chapter 10.
|_______________________________________________________________________*/

namespace xqp 
{

class expr;
class expr_list;
class var_expr;
class order_modifier;
class flwor_expr;
class quantified_expr;
class case_clause;
class typeswitch_expr;
class if_expr;
class fo_expr;
class ft_select_expr;
class ft_contains_expr;
class instanceof_expr;
class treat_expr;
class castable_expr;
class cast_expr;
class unary_expr;
class validate_expr;
class extension_expr;
class relpath_expr;
class axis_step_expr;
class primary_expr;
class literal_expr;
class order_expr;
class funcall_expr;
class cons_expr;
class doc_expr;
class elem_expr;
class attr_expr;
class text_expr;
class comment_expr;
class pi_expr;


class plan_visitor : public expr_visitor
{
public:	
	plan_visitor();
	~plan_visitor();

public:

 /*..........................................
 	:  begin visit                            :
 	:.........................................*/
	bool begin_visit(expr&);
	bool begin_visit(expr_list&);
	bool begin_visit(var_expr&);
	bool begin_visit(order_modifier&);
	bool begin_visit(flwor_expr&);
	bool begin_visit(quantified_expr&);
	bool begin_visit(case_clause&);
	bool begin_visit(typeswitch_expr&);
	bool begin_visit(if_expr&);
	bool begin_visit(fo_expr&);
	bool begin_visit(ft_select_expr&);
	bool begin_visit(ft_contains_expr&);
	bool begin_visit(instanceof_expr&);
	bool begin_visit(treat_expr&);
	bool begin_visit(castable_expr&);
	bool begin_visit(cast_expr&);
	bool begin_visit(unary_expr&);
	bool begin_visit(validate_expr&);
	bool begin_visit(extension_expr&);
	bool begin_visit(relpath_expr&);
	bool begin_visit(axis_step_expr&);
	bool begin_visit(primary_expr&);
	bool begin_visit(literal_expr&);
	bool begin_visit(order_expr&);
	bool begin_visit(funcall_expr&);
	bool begin_visit(cons_expr&);
	bool begin_visit(doc_expr&);
	bool begin_visit(elem_expr&);
	bool begin_visit(attr_expr&);
	bool begin_visit(text_expr&);
	bool begin_visit(comment_expr&);
	bool begin_visit(pi_expr&);


 /*..........................................
 	:  end visit                              :
 	:.........................................*/
	void end_visit(expr&);
	void end_visit(expr_list&);
	void end_visit(var_expr&);
	void end_visit(order_modifier&);
	void end_visit(flwor_expr&);
	void end_visit(quantified_expr&);
	void end_visit(case_clause&);
	void end_visit(typeswitch_expr&);
	void end_visit(if_expr&);
	void end_visit(fo_expr&);
	void end_visit(ft_select_expr&);
	void end_visit(ft_contains_expr&);
	void end_visit(instanceof_expr&);
	void end_visit(treat_expr&);
	void end_visit(castable_expr&);
	void end_visit(cast_expr&);
	void end_visit(unary_expr&);
	void end_visit(validate_expr&);
	void end_visit(extension_expr&);
	void end_visit(relpath_expr&);
	void end_visit(axis_step_expr&);
	void end_visit(primary_expr&);
	void end_visit(literal_expr&);
	void end_visit(order_expr&);
	void end_visit(funcall_expr&);
	void end_visit(cons_expr&);
	void end_visit(doc_expr&);
	void end_visit(elem_expr&);
	void end_visit(attr_expr&);
	void end_visit(text_expr&);
	void end_visit(comment_expr&);
	void end_visit(pi_expr&);

};

} /* namespace xqp */
#endif /* XQP_PLAN_VISITOR_H */
