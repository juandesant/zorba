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
#pragma once
#ifndef ZORBA_SPECIFIC_CLASS_SERIALIZATION
#define ZORBA_SPECIFIC_CLASS_SERIALIZATION

#include <zorba/item.h>
#include <zorba/xquery_exception.h>
#include "zorbatypes/rchandle.h"
#include "zorbatypes/m_apm.h"
#include "zorbatypes/floatimpl.h"
#include "zorbautils/checked_vector.h"

#include "zorbaserialization/class_serializer.h"
#include "zorbaserialization/archiver.h"

#include "store/api/shared_types.h"
#include "diagnostics/assert.h"

#include <stdio.h>
#include <map>

namespace zorba
{
  class XQType;
  class function;
  class Diagnostic;
  class ZorbaException;
#ifdef ZORBA_WITH_BIG_INTEGER
  class IntegerImpl;
#else
  template<typename IntType> class IntegerImpl;
#endif /* ZORBA_WITH_BIG_INTEGER */

namespace serialization
{

void operator&(Archiver& ar, const XQType*& obj);

#ifdef ZORBA_WITH_BIG_INTEGER
void operator&(Archiver& ar, IntegerImpl& obj);
#else
template<typename IntType>
void operator&(Archiver& ar, IntegerImpl<IntType>& obj);
#endif /* ZORBA_WITH_BIG_INTEGER */

void operator&(Archiver& ar, store::Item*& obj);
void operator&(Archiver& ar, zorba::Item& obj);

void operator&(Archiver& ar, const Diagnostic*& obj);
void operator&(Archiver& ar, ZorbaException*& obj);
void operator&(Archiver& ar, zorba::internal::diagnostic::location& obj);
void operator&(Archiver& ar, zorba::XQueryStackTrace& obj);
void operator&(Archiver& ar, zorba::XQueryStackTrace::Entry& obj);


#define SERIALIZE_TYPEMANAGER(type_mgr_type, type_mgr)                  \
  bool is_root_type_mgr = ar.is_serializing_out() && (!GENV.isRootStaticContextInitialized() || ((TypeManager*)type_mgr == (TypeManager*)&GENV_TYPESYSTEM)) ; \
  ar.set_is_temp_field(true);                                           \
  ar & is_root_type_mgr;                                                \
  ar.set_is_temp_field(false);                                          \
                                                                        \
  if (is_root_type_mgr)                                                 \
  {                                                                     \
    if (!ar.is_serializing_out())                                       \
    {                                                                   \
      type_mgr = (type_mgr_type*)&GENV_TYPESYSTEM;                      \
      /*RCHelper::addReference(type_mgr);*/                             \
    }                                                                   \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    ar & type_mgr;                                                      \
  }

#define SERIALIZE_TYPEMANAGER_RCHANDLE(type_mgr_type, type_mgr)                             \
  bool is_root_type_mgr = (!GENV.isRootStaticContextInitialized() || ((TypeManager*)type_mgr.getp() == (TypeManager*)&GENV_TYPESYSTEM));            \
  ar.set_is_temp_field(true);                                           \
  ar & is_root_type_mgr;                                            \
  ar.set_is_temp_field(false);                                      \
  if(is_root_type_mgr)                                              \
  {                                                                 \
    if(!ar.is_serializing_out())                                    \
     type_mgr = (type_mgr_type*)&GENV_TYPESYSTEM;                   \
  }                                                                 \
  else                                                              \
  {                                                                 \
    ar & type_mgr;                                                  \
  }


template<typename StringType>
void operator&(Archiver& ar, zorba::internal::VariableQName<StringType>& obj)
{
  if (ar.is_serializing_out())
  {
    bool is_ref;
    is_ref = ar.add_compound_field("VariableQName<StringType>",
                                   !FIELD_IS_CLASS,
                                   NULL,
                                   &obj,
                                   ARCHIVE_FIELD_NORMAL);
    if (!is_ref)
    {
      ar & obj.ns_;
      ar & obj.prefix_;
      ar & obj.localname_;
      ar.add_end_compound_field();
    }
    else
    {
      assert(false);
    }
  }
  else
  {
    char* type;
    char* value;
    int   id;
    ArchiveFieldKind field_treat = ARCHIVE_FIELD_NORMAL;
    int   referencing;
    bool  retval;

    retval = ar.read_next_field(&type, &value, &id, false, false, false,
                                &field_treat, &referencing);

    ar.check_nonclass_field(retval, field_treat, ARCHIVE_FIELD_NORMAL, id);

    ar & obj.ns_;
    ar & obj.prefix_;
    ar & obj.localname_;
    ar.read_end_current_level();
  }
}


} // namespace serialization
} // namespae zorba
#endif
/* vim:set et sw=2 ts=2: */
