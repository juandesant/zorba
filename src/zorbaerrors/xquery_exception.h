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

#ifndef ZORBA_XQUERY_EXCEPTION_H
#define ZORBA_XQUERY_EXCEPTION_H

#include <zorba/xquery_exception.h>
#include "compiler/parser/query_loc.h"
#include "err.h"

namespace zorba {

////////// XQueryException creation ///////////////////////////////////////////

/**
 * Makes a dynamically allocated XQueryException with the given parameters
 * substituted into the error message.
 *
 * @param error The error.
 * @param throw_file The source-code file name whence the exception was thrown.
 * @param throw_line The source-code line number whence the exception was
 * thrown.
 * @param params The error message parameters.
 * @return Returns a new XQueryException.
 */
ZORBA_DLL_PUBLIC XQueryException*
new_xquery_exception( char const *throw_file, unsigned throw_line,
                      Error const &error,
                      internal::err::parameters const &params,
                      internal::err::location const &loc =
                        internal::err::location::empty );

/**
 * Creates a dynamically allocated XQueryException using the local name of an
 * error.
 * \hideinitializer
 */
#define NEW_XQUERY_EXCEPTION(...) \
  MAKE_EXCEPTION_VAR( new_xquery_exception, zorba::err:: __VA_ARGS__ )

////////// XQuery error source location ///////////////////////////////////////

/**
 * Sets the XQuery source location of the given ZorbaException but only if it's
 * actually an XQueryException.
 *
 * @param ze The ZorbaException to set the location of.
 * @param file The XQuery file name.
 * @param line The line number.
 * @param col The column number.
 * @param overwrite If \c false, sets the location only if the exception
 * doesn't already have one; if \c true, always sets the location even if the
 * exception already has one.
 */
inline void set_source( ZorbaException &ze, char const *file,
                        XQueryException::line_type line,
                        XQueryException::column_type col,
                        bool overwrite = true ) {
  if ( XQueryException *const xe = dynamic_cast<XQueryException*>( &ze ) )
    if ( !xe->has_source() || overwrite )
      xe->set_source( file, line, col );
}

/**
 * Sets the XQuery source location of the given ZorbaException but only if it's
 * actually an XQueryException.
 *
 * @tparam StringType The \a file string type.
 * @param ze The ZorbaException to set the location of.
 * @param file The XQuery file name.
 * @param line The line number.
 * @param col The column number.
 * @param overwrite If \c false, sets the location only if the exception
 * doesn't already have one; if \c true, always sets the location even if the
 * exception already has one.
 */
template<class StringType> inline
void set_source( ZorbaException &ze, StringType const &file,
                 XQueryException::line_type line,
                 XQueryException::column_type col, bool overwrite = true ) {
  set_source( ze, file.c_str(), line, col, overwrite );
}

/**
 * Sets the XQuery source location of the given ZorbaException but only if it's
 * actually an XQueryException.
 *
 * @param ze The ZorbaException to set the location of.
 * @param loc The query location.
 * @param overwrite If \c false, sets the location only if the exception
 * doesn't already have one; if \c true, always sets the location even if the
 * exception already has one.
 */
inline void set_source( ZorbaException &ze, QueryLoc const &loc,
                        bool overwrite = true ) {
  set_source(
    ze, loc.getFilename(), loc.getLineno(), loc.getColumnBegin(), overwrite
  );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace zorba
#endif /* ZORBA_XQUERY_EXCEPTION_H */
/* vim:set et sw=2 ts=2: */