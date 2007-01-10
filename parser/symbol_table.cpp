/* -*- mode: c++; indent-tabs-mode: nil -*-
 *
 *  $Id: symbol_table.cpp,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 */

#include "symbol_table.h"

#include <stdlib.h>
#include <string>

using namespace std;
namespace xqp {

symbol_table::symbol_table(uint32_t initial_heapsize)
:
	heap(initial_heapsize)
{
}

symbol_table::~symbol_table()
{
}

uint32_t symbol_table::size() const
{
	return heap.size();
}

uint32_t symbol_table::put(char const* text, uint32_t length)
{
	return heap.put(text, 0, length);
}

uint32_t symbol_table::put_ncname(char const* text, uint32_t length)
{
	return heap.put(text, 0, length);
}

uint32_t symbol_table::put_qname(char const* text, uint32_t length)
{
	return heap.put(text, 0, length);
}

uint32_t symbol_table::put_uri(char const* text, uint32_t length)
{
	return heap.put(text, 0, length);
}

uint32_t symbol_table::put_varname(char const* text, uint32_t length)
{
	return heap.put(text, 0, length);
}

uint32_t symbol_table::put_entityref(char const* text, uint32_t length)
{
	return heap.put(text, 0, length);
}

uint32_t symbol_table::put_charref(char const* text, uint32_t length)
{
	return heap.put(text, 0, length);
}

uint32_t symbol_table::put_stringlit(char const* text, uint32_t length)
{
	return heap.put(text, 1, length-2);
}

long long symbol_table::decimalval(char const* text, uint32_t length)
{
	return strtoll(text, NULL, 10);
}

double symbol_table::doubleval(char const* text, uint32_t length)
{
	return strtod(text, NULL);
}

int symbol_table::intval(char const* text, uint32_t length)
{
	return strtol(text, NULL, 10);
}

std::string symbol_table::get(uint32_t id)
{
	uint32_t n = heap.get_length(id);
	char buf[n+1];
	heap.get(id, buf, 0, n);
	return string(buf, 0, n);
}


}	/* namespace xqp */
