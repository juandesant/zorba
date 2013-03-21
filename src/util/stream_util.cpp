/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "stdafx.h"

// local
#include "ascii_util.h"
#include "stream_util.h"

using namespace std;

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

int get_stream_uri_index() {
  //
  // This function is out-of-line because it has a static constant within it.
  // It has a static constant within it to guarantee (1) initialization before
  // use and (2) initialization happens exactly once.
  //
  // See: "Standard C++ IOStreams and Locales: Advanced Programmer's Guide and
  // Reference," Angelika Langer and Klaus Kreft, Addison-Wesley, 2000, section
  // 3.3.1.1: "Initializing and Maintaining the iword/pword Index."
  //
  // See: "The C++ Programming Language," Bjarne Stroustrup, Addison-Wesley,
  // 2000, section 10.4.8: "Local Static Store."
  //
  static int const index = ios_base::xalloc();
  return index;
}

void delete_stream_uri_callback( ios_base::event e, ios_base &ios,
                                 int index ) {
  //
  // See: "Standard C++ IOStreams and Locales: Advanced Programmer's Guide and
  // Reference," Angelika Langer and Klaus Kreft, Addison-Wesley, 2000, section
  // 3.3.1.4: "Using Stream Callbacks for Memory Management."
  //
  if ( e == ios_base::erase_event )
    delete[] static_cast<char const*>( ios.pword( index ) );
}

///////////////////////////////////////////////////////////////////////////////

streamsize read_without_whitespace( istream &is, char *buf, streamsize n ) {
  char const *const buf_orig = buf;
  char const *const buf_end = buf + n;

  while ( buf < buf_end ) {
    is.read( buf, n );
    if ( streamsize read = is.gcount() ) {
      read = ascii::remove_whitespace( buf, read );
      buf += read, n -= read;
    } else
      break;
  }
  return buf - buf_orig;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace zorba
/* vim:set et sw=2 ts=2: */