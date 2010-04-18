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

#include <memory>                       /* for auto_ptr */
#include <unicode/ustring.h>

#include "runtime/full_text/icu_tokenizer.h"

using namespace std;
U_NAMESPACE_USE

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

UChar32 const SubChar = 0xFFFD;         // replacement character

/**
 * Converts a UTF-8 string to UTF-16.  The caller is responsible to deleting
 * the returned string.
 */
static UChar* utf8_to_utf16( char const *utf8_s, int32_t utf8_len,
                             int32_t *utf16_len_ptr ) {
  int32_t utf16_len;
  UErrorCode err = U_ZERO_ERROR;
  u_strFromUTF8WithSub(
    NULL, 0, &utf16_len,
    utf8_s, utf8_len,
    SubChar, NULL,
    &err
  );
  // must not check "err" here since ICU always returns "buffer overflow"

  UChar *const utf16_buf = new UChar[ utf16_len ];
  err = U_ZERO_ERROR;
  u_strFromUTF8WithSub(
    utf16_buf, utf16_len, utf16_len_ptr,
    utf8_s, utf8_len,
    SubChar, NULL,
    &err
  );
#if 0 // TODO
  if ( U_FAILURE( err ) )
    ZORBA_ERROR( err );
#endif
  return utf16_buf;
}

/**
 * Converts a UTF-16 string to UTF-8.  The caller is responsible to deleting
 * the returned string.
 */
static char* utf16_to_utf8( UChar const *utf16_s, int32_t utf16_len,
                            int32_t *utf8_len_ptr = 0 ) {
  int32_t utf8_len;
  UErrorCode err = U_ZERO_ERROR;
  u_strToUTF8WithSub(
    NULL, 0, &utf8_len,
    utf16_s, utf16_len,
    SubChar, NULL,
    &err
  );
  // must not check "err" here since ICU always returns "buffer overflow"

  char *const utf8_buf = new char[ utf8_len ];
  err = U_ZERO_ERROR;
  u_strToUTF8WithSub(
    utf8_buf, utf8_len, utf8_len_ptr,
    utf16_s, utf16_len,
    SubChar, NULL,
    &err
  );
#if 0 // TODO
  if ( U_FAILURE( err ) )
    ZORBA_ERROR( err );
#endif
  return utf8_buf;
}

///////////////////////////////////////////////////////////////////////////////

icu_tokenizer::icu_tokenizer() {
  word_it_ = sent_it_ = NULL;
  UErrorCode err = U_ZERO_ERROR;

  // TODO: get actual locale
  word_it_ = dynamic_cast<RuleBasedBreakIterator*>(
    BreakIterator::createWordInstance( Locale::getUS(), err )
  );
  if ( U_FAILURE( err ) )
    /* TODO: ZORBA_ERROR(code) */;

  sent_it_ = dynamic_cast<RuleBasedBreakIterator*>(
    BreakIterator::createSentenceInstance( Locale::getUS(), err )
  );
  if ( U_FAILURE( err ) )
    /* TODO: ZORBA_ERROR(code) */;
}

icu_tokenizer::~icu_tokenizer() {
  delete word_it_;
  delete sent_it_;
}

///////////////////////////////////////////////////////////////////////////////

void icu_tokenizer::tokenize( string const &utf8_s, callback &cb ) {
  int32_t utf16_len;
  auto_ptr<UChar> const utf16_buf(
    utf8_to_utf16( utf8_s.c_str(), utf8_s.length(), &utf16_len )
  );
  UnicodeString const utf16_s( false, utf16_buf.get(), utf16_len );

  word_it_->setText( utf16_s );
  int32_t word_start = word_it_->first(), word_end = word_it_->next();

  sent_it_->setText( utf16_s );
  int32_t sent_start = sent_it_->first(), sent_end = sent_it_->next();

  while ( word_end != BreakIterator::DONE ) {
    if ( word_it_->getRuleStatus() != UBRK_WORD_NONE ) {
      int32_t const utf16_len = word_end - word_start;
      int32_t utf8_len;
      auto_ptr<char> const utf8_buf(
        utf16_to_utf8( utf16_buf.get() + word_start, utf16_len, &utf8_len )
      );
      cb( utf8_buf.get(), utf8_len, token_no_++, sent_no_, para_no_ );
    }
    word_start = word_end, word_end = word_it_->next();
    if ( word_end >= sent_end && sent_end != BreakIterator::DONE ) {
      sent_start = sent_end, sent_end = sent_it_->next();
      ++sent_no_;
    }
  }
}

} // namespace zorba
/* vim:set et sw=2 ts=2: */
