/*
 * Copyright 2006-2013 The FLWOR Foundation.
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

#pragma once
#ifndef ZORBA_FTP_CLIENT_FTP_CONNECTIONS_H
#define ZORBA_FTP_CLIENT_FTP_CONNECTIONS_H

// standard
#include <map>

// Zorba
#include <zorba/external_function_parameter.h>
#include <zorba/zorba_string.h>

// local
#include "curl_streambuf.h"

namespace zorba {
namespace ftp_client {

///////////////////////////////////////////////////////////////////////////////

class connections : public ExternalFunctionParameter {
public:
  // inherited
  virtual void destroy() throw();

  bool delete_buf( String const &uri );
  curl::streambuf* get_buf( String const &uri ) const;
  curl::streambuf* new_buf( String const &uri );

private:
  // map connection URIs -> cURL::streambuf
  typedef std::map<String,curl::streambuf*> conn_buf_map;
  conn_buf_map conn_buf_;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ftp_client
} // namespace zorba
#endif /* ZORBA_FTP_CLIENT_FTP_CONNECTIONS_H */
/* vim:set et sw=2 ts=2: */
