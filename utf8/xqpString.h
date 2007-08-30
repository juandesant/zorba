/**
 *
 * @copyright
 * ========================================================================
 *	Copyright 2007 FLWOR Foundation
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 * ========================================================================
 *
 * @author Sorin Nasoi (sorin.nasoi@ipdevel.ro)
 * @file utf8/xqpString.h
 *
 */ 
#ifndef XQP_STRING_UTF8_STRING_REPRESENTATION_30_AUG_2007
#define XQP_STRING_UTF8_STRING_REPRESENTATION_30_AUG_2007

#ifdef WIN32
#include "win32/compatib_defs.h"
#endif

#include <stdlib.h>
#include "utf8.h"

namespace xqp {
	class xqpString
	{
		public:
		//! @name constructor/destructor
		//! @{
			/**
			 * Construct an empty stdString
				*/
			xqpString();

			/**
			 * Construct a xqpString as a copy of another xqpString
			 * @param src A source UTF-8 encoded string
			 */
			xqpString(const xqpString& src);
			

			/**
			 * Construct a xqpString given a std::string
			 * @param src A source std::string containing UTF-8 encoded string
			 */
			xqpString(const std::string& src);


			/**
			 * Construct a xqpString as a copy of a C string.
			 * @param src %Source C string that is encoded as UTF-8
			 */
			xqpString(const char* src);
			
			~xqpString();
		//! @}
			
		//! @name xqpString::operator=()
		//! @{
			xqpString& operator=(const xqpString& src);
			xqpString& operator=(const std::string& src);
			xqpString& operator=(const char* src);
			/**
			 * @param cp Codepoint
			 */
			xqpString& operator=(uint32_t cp);
			xqpString& operator=(char c);
		//! @}
			
		//! @name xqpString::operator+=()
		//! @{
			xqpString& operator+=(const xqpString& src);
			xqpString& operator+=(const char* src);
			/**
			 * @param cp Codepoint
			 */
			xqpString& operator+=(uint32_t cp);
			xqpString& operator+=(char c);
		//! @}

		//! @name xqpString::operator==()
		//! @{
			inline bool operator==(const xqpString& src){
				return (compare(src) == 0);
			}

			inline bool operator==(const char* src){
				return (compare(src) == 0);
			}
		//!@}

		//! @name xqpString::operator!=()
		//! @{
			inline bool operator!=(const xqpString& src){
				return (compare(src) != 0);
			}

			inline bool operator!=(const char* src){
				return (compare(src) != 0);
			}
		//!@}

		//! @name Compare
		//! @{
			int compare(const xqpString& src)	const;
			int compare(const char* src)	const;
		//! @}

			inline	operator std::string() const{
				return utf8String;
			}
		private:
			std::string utf8String;
	};

	//! @name xqpString::stream I/O operators
	//! @{
	std::istream& operator>>(std::istream& is, xqpString& utf8_src);
	std::ostream& operator<<(std::ostream& os, const xqpString& utf8_src);
	//!@}
	
	//! @name xqpString::concatenation operator+()
	//! @{
	inline xqpString operator+(const xqpString& lsrc, const xqpString& rsrc){
		xqpString tmp (lsrc);
		tmp += rsrc;
		return tmp;
	}

	inline xqpString operator+(const xqpString& lsrc, const char* rsrc){
		xqpString tmp (lsrc);
		tmp += rsrc;
		return tmp;
	}

	inline xqpString operator+(const char* lsrc, const xqpString& rsrc){
		xqpString tmp (lsrc);
		tmp += rsrc;
		return tmp;
	}
	//! @}
}/* namespace xqp */

#endif

