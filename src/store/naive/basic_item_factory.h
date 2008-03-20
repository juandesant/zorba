/* -*- mode: c++; indent-tabs-mode: nil; tab-width: 2 -*-
 *
 *  $Id: basic_item_factory.h,v 1.1 2007/08/28 09:54:59 $
 *
 *	Copyright 2006-2007 FLWOR Foundation.
 *  Author: David Graf (david.graf@28msec.com), Donald Kossmann, Tim Kraska
 *
 */
#ifndef ZORBA_STORE_BASIC_ITEM_FACTORY_H
#define ZORBA_STORE_BASIC_ITEM_FACTORY_H

#include "zorbatypes/xqpstring.h"
#include "zorbatypes/representations.h"

#include "common/shared_types.h"

#include "store/api/item_factory.h"



namespace zorba { namespace store {
typedef StringPool UriPool;


class BasicItemFactory : public ItemFactory
{
protected:
  UriPool    * theUriPool;
  QNamePool  * theQNamePool;

public:
  BasicItemFactory(UriPool* uriPool, QNamePool* qnPool);

  virtual ~BasicItemFactory();

  Item_t createQName(
        xqpStringStore* ns,
        xqpStringStore* pre,
        xqpStringStore* local);

  Item_t createQName(
        const char* ns,
        const char* pre,
        const char* ln);

  Item_t createNCName(xqpStringStore* value);

  Item_t createAnyURI(xqpStringStore* value);
  Item_t createAnyURI(const char* value);

  Item_t createUntypedAtomic(xqpStringStore* value);

  Item_t createString(xqpStringStore* value);

  virtual Item_t createBase64Binary(xqp_base64Binary value);

  virtual Item_t createBoolean(xqp_boolean value);

  virtual Item_t createDecimal(xqp_decimal value);

  virtual Item_t createInteger(xqp_integer value);

  virtual Item_t createLong(xqp_long value);

  virtual Item_t createInt(xqp_int value);

  virtual Item_t createShort(xqp_short value);

  virtual Item_t createByte(xqp_byte value);

  virtual Item_t createDate(xqp_date& value);
  
  virtual Item_t createDate(short year, short month, short day);

  virtual Item_t createDate(const xqp_string& value);

  virtual Item_t createDateTime(xqp_dateTime& value);
  
  virtual Item_t createDateTime(short year, short month, short day, short hour, short minute, short second);

  virtual Item_t createDateTime(short year , short month, short day, short hour, short minute, short second, short timeZone);

  virtual Item_t createDateTime(const xqp_string& value);
  
  virtual Item_t createDateTime(const Item_t&, const Item_t&);

  virtual Item_t createDouble(xqp_double value);

  virtual Item_t createDuration(xqp_duration& value);
  
  virtual Item_t createDuration(const xqp_string& value);

  virtual Item_t createDuration(short years, short months, short days, short hours, short minutes, short seconds);

  virtual Item_t createENTITIES ( const xqp_string& value );

  virtual Item_t createENTITY ( const xqp_string& value );

  virtual Item_t createFloat ( xqp_float value );

  virtual Item_t createGDay (xqp_gDay& value );

  virtual Item_t createGDay ( const xqp_string& value );

  virtual Item_t createGDay ( short day );

  virtual Item_t createGMonth (xqp_gMonth& value);

  virtual Item_t createGMonth ( const xqp_string& value );

  virtual Item_t createGMonth ( short month );

  virtual Item_t createGMonthDay (xqp_gMonthDay& value);

  virtual Item_t createGMonthDay ( const xqp_string& value );

  virtual Item_t createGMonthDay ( short month, short day );

  virtual Item_t createGYear (xqp_gYear& value);
  
  virtual Item_t createGYear ( const xqp_string& value );

  virtual Item_t createGYear ( short year );

  virtual Item_t createGYearMonth (xqp_gYearMonth& value);

  virtual Item_t createGYearMonth ( const xqp_string& value );

  virtual Item_t createGYearMonth ( short year, short month );

  virtual Item_t createHexBinary ( xqp_hexBinary value );

  virtual Item_t createID ( const xqp_string& value );

  virtual Item_t createIDREF ( const xqp_string& value );

  virtual Item_t createIDREFS ( const xqp_string& value );

  virtual Item_t createLanguage ( const xqp_string& value );

  virtual Item_t createNMTOKEN ( const xqp_string& value );

  virtual Item_t createNMTOKENS ( const xqp_string& value );

  virtual Item_t createNOTATION ( const xqp_string& value );

  virtual Item_t createName ( const xqp_string& value );

  virtual Item_t createNegativeInteger ( xqp_integer value );

  virtual Item_t createNonNegativeInteger(xqp_uinteger value );

  virtual Item_t createNonPositiveInteger(xqp_integer value );

  virtual Item_t createNormalizedString(const xqp_string& value );

  virtual Item_t createPositiveInteger( xqp_uinteger value );

  virtual Item_t createTime(xqp_time& value);
  
  virtual Item_t createTime(const xqp_string& value );

  virtual Item_t createTime(short hour, short minute, short second );

  virtual Item_t createTime(short hour, short minute, short second, short timeZone);

  virtual Item_t createToken(const xqp_string& value);

  virtual Item_t createUnsignedByte(xqp_ubyte value);

  virtual Item_t createUnsignedInt(xqp_uint value);

  virtual Item_t createUnsignedLong(xqp_ulong value);

  virtual Item_t createUnsignedShort(xqp_ushort value);



  Item_t createDocumentNode(
        unsigned long   qid,
        xqpStringStore* baseURI,
        xqpStringStore* docURI,
        Iterator*       children,
        bool            isRoot,
        bool            copy,
        bool            typePreserve,
        bool            nsPreserve,
        bool            nsInherit);

  Item_t createElementNode(
        unsigned long     qid,
        Item*             name,
        Item*             type,
        Iterator*         childrenIte,
        Iterator*         attributesIte,
        Iterator*         namespacesIte,
        const NsBindings& contextBindings,
        const NsBindings& localBindings,
        bool              isRoot,
        bool              copy,
        bool              typePreserve,
        bool              nsPreserve,
        bool              nsInherit);

  Item_t createAttributeNode(
        unsigned long     qid,
        Iterator* nameIter,
        Item*     typeName,
        Iterator* valueIter,
        bool      isRoot);

  Item_t createTextNode(
        unsigned long   qid,
        Iterator*       valueIter,
        bool            isRoot);

  Item_t createTextNode(
        unsigned long   qid,
        xqpStringStore* value,
        bool            isRoot);

  Item_t createPiNode (
        unsigned long   qid,
        xqpStringStore* target,
        xqpStringStore* data,
        bool            isRoot);

  Item_t createCommentNode (
        unsigned long   qid,
        xqpStringStore* comment,
        bool            isRoot);
};

} // namespace store
} // namespace zorba

#endif
