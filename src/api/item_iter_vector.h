/*
 * Copyright 2006-2016 zorba.io
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
#ifndef ZORBA_VECTOR_ITERATOR_H
#define ZORBA_VECTOR_ITERATOR_H

#include <zorba/iterator.h>
#include "common/shared_types.h"
#include <vector>

namespace zorba {

class DiagnosticHandler;

/*******************************************************************************
  Iterator used for iterating over an internal store::Iterator_t.
********************************************************************************/
class VectorIterator: public Iterator 
{
  friend class Unmarshaller;
  friend class Item;

protected:
  std::vector<store::Item_t>            theVector;
  std::vector<store::Item_t>::iterator  theIter;
  DiagnosticHandler                   * theDiagnosticHandler;
  bool                                  theIsOpen;     

public:
  VectorIterator(const std::vector<store::Item_t>&, DiagnosticHandler*);

  virtual ~VectorIterator();

  virtual void open();

  virtual bool next(Item&);

  virtual void close();

  bool isOpen() const { return theIsOpen; }
};

} /* namespace zorba */

#endif
/* vim:set et sw=2 ts=2: */
