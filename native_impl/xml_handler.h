/* -*- mode: c++; indent-tabs-mode: nil -*-
 *
 *  $Id: xml_handler.h,v 1.1.1.1 2006/10/09 06:58:39 Paul Pedersen Exp $
 *
 *  Copyright 2006-2007 FLWOR Foundation.
 *
 *	Author: John Cowan,Paul Pedersen
 *
 */
 
/*______________________________________________________________________
|  
|	Code derived from the 'tagsoup' HTML parser,
|	written by John Cowan (cowan@ccil.org).
|_______________________________________________________________________*/


#ifndef XQP_XML_HANDLER_H
#define XQP_XML_HANDLER_H

#include "nodestore.h"
#include "scan_handler.h"
#include "xml_term.h"

#include "../util/rchandle.h"
#include "../util/URI.h"

#include <string>
#include <stack>
#include <ostream>
#include <sstream>
#include <utility>


namespace xqp {

class context;

class xml_handler : public scan_handler
{
public:
	typedef std::pair<std::string,std::string> attrpair_t;

protected:  // state
	static const uint32_t STACK_CAPACITY = 65536;
	std::string the_name_stack[STACK_CAPACITY];
	uint32_t the_id_stack[STACK_CAPACITY];
	uint32_t nodeid_stack[STACK_CAPACITY];
	uint32_t top;
	uint32_t qtop;
	uint32_t ntop;

  std::string the_attribute;			// most recent attribute
  std::string the_element;				// most recent element tag
	std::string	the_PCDATA;					// most recent PCDATA
  std::string	the_PITarget;				// most recent PITarget
  uint16_t the_entity;						// most recent entity 

	uint32_t term_pos;							// current term position
	uint32_t last_pos;							// last term position, for delta-coding
	uint64_t uri;										// current URI hash
	std::vector<xml_term>& term_v;	// term index accumulator
	std::vector<attrpair_t> attr_v;	// term index accumulator

	// indexing switches
	bool term_indexing;							// index terms
	bool bigram_indexing;						// index term bigrams
	bool e_indexing;								// index elements
	bool p_indexing;								// index     c/word::t
	bool gp_indexing;								// index   b/c/word::t
	bool ggp_indexing;							// index a/b/c/word::t

	rchandle<nodestore> nstore_h;
	context * ctx_p;

	uint32_t the_id;								// context node
	uint32_t the_parentid;					// context parent id
	uint32_t the_docid;							// context document id
	uint32_t the_qnameid;						// context qname id
	uint32_t the_nsid;							// context namespace id

	ostringstream textbuf;

public:	// ctor, dtor
	xml_handler(
		context *,
	  rchandle<nodestore>, 					// accumulate document here
		uint64_t uri,									// document URI
	  std::vector<xml_term>&);			// accumulate terms here

	xml_handler(
		context *,
	  rchandle<nodestore>, 					// accumulate document here
		std::string const& uri,				// doucment URI
	  std::vector<xml_term>&);			// accumulate terms here

	~xml_handler() {}

protected:
	void error(std::string const& msg) const throw (xqp_exception);

public:	
	void add_term(xml_term const&);
	void add_term(std::string const& term, uint64_t uri, uint32_t pos);
	uint32_t get_term_position() const { return term_pos; }
	uint32_t short get_entity() { return the_entity; }

public:	// callback methods
	//  Report an attribute name with no value.
	void adup(const char* buf, int offset, int length);

	//  Report an attribute name, a value will follow;
	void aname(const char* buf, int offset, int length);

	//  Report an attribute value.
	void aval(const char* buf, int offset, int length);

	//  Report and entity reference or character reference.
	void entity(const char* buf, int offset, int length);

	//  Report EOF.
	void eof(const char* buf, int offset, int length);

	//  Report end tag.
	void etag(const char* buf, int offset, int length);

	//  Report general identifier (element type name) of a start tag.
	void gi(const char* buf, int offset, int length);

	//  Report character data.
	void pcdata(const char* buf, int offset, int length);

	//  Report the data part of a processing instruction.
	void pi(const char* buf, int offset, int length);

	//  Report the target part of a processing instruction.
	void pitarget(const char* buf, int offset, int length);

	//  Report the close of a start tag.
	void stagc(const char* buf, int offset, int length);

	//  Report a comment.
	void cmnt(const char* buf, int offset, int length);

	void handle_pcdata(const char*, int, int);

};

}	/* namespace xqp */
#endif	/* XQP_XML_HANDLER_H */
