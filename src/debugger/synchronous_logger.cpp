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
#include "synchronous_logger.h"


zorba::synchronous_logger::SynchronousLogger::SynchronousLogger( std::ostream &stream )
:
m_stream(stream)
{
}

zorba::synchronous_logger::SynchronousLogger& zorba::synchronous_logger::SynchronousLogger::operator<<( const long input )
{
	SYNC_CODE(AutoLock lock(m_lock, Lock::WRITE);)
  m_stream << input;
	m_stream.flush();
	return *this;
}

zorba::synchronous_logger::SynchronousLogger& zorba::synchronous_logger::SynchronousLogger::operator<<( const zorba::String input )
{
	SYNC_CODE(AutoLock lock(m_lock, Lock::WRITE);)
  m_stream << input;
	m_stream.flush();
	return *this;
}

