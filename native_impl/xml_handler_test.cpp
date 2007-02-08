/* -*- mode: c++; indent-tabs-mode: nil -*-
 *
 *  $Id: xml_handler_test.cpp,v 1.1 2006/10/09 07:07:59 Paul Pedersen Exp $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *
 *	Author: John Cowan,Paul Pedersen
 *
 */

#include "xml_handler.h"
#include "xml_scanner.h"
#include "xml_term.h"
#include "../context/context.h"
#include "../util/file.h"

using namespace std;
using namespace xqp;

int main(int argc, char* argv[]) 
{
	context ctx;
	rchandle<nodestore> nstore_h = ctx.get_nodestore();

  try {
    if (argc<2) {
      cerr << "Expecting one argument: PATH\n";
      return -1;
    }
    string path(argv[1],0,strlen(argv[1]));
		file f(path);
    if (!f.exists()) {
      cout << "file '" << path << "' does not exist\n";
      return -1;
    }
		cout << "read file..\n";
    unsigned sz = f.get_size();
    size_t n = (sz > (1<<24) ? (1<<24) : (size_t)(sz));
    char* ibuf = new char[n+1];
		f.readfile(ibuf,n);

		cout << "allocating scan/handlers..\n";
    xml_scanner xscanner;
		uint64_t uri = 0;
		vector<xml_term> xterm_v;
    xml_handler* xhandler = new xml_handler(&ctx,nstore_h,uri,xterm_v);

		cout << "scanning..\n";
    xscanner.scan(ibuf, n, dynamic_cast<scan_handler*>(xhandler));

		cout << "cleaning up..\n";
    delete xhandler;
		delete[] ibuf;

  } catch (xqp_exception& e1) {
    cout << "Application exception: " << e1.what() << '\t' << e1.get_msg() << endl;
  } catch (exception& e2) {
    cout << "System exception: " << e2.what() << endl;
  } catch (...) {
    cout << "Unhandled exception\n";
  }
	
}

