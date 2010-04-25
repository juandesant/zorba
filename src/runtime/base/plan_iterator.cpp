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
#include "context/static_context.h"

#include "compiler/api/compilercb.h"

#include "runtime/base/plan_iterator.h"
#include "runtime/api/runtimecb.h"

#include "runtime/util/flowctl_exception.h"

namespace zorba
{

SERIALIZABLE_TEMPLATE_VERSIONS(Batcher)
END_SERIALIZABLE_TEMPLATE_VERSIONS(Batcher)

SERIALIZABLE_TEMPLATE_VERSIONS(BinaryBaseIterator)
END_SERIALIZABLE_TEMPLATE_VERSIONS(BinaryBaseIterator)

SERIALIZABLE_TEMPLATE_VERSIONS(NaryBaseIterator)
END_SERIALIZABLE_TEMPLATE_VERSIONS(NaryBaseIterator)

SERIALIZABLE_TEMPLATE_VERSIONS(NoaryBaseIterator)
END_SERIALIZABLE_TEMPLATE_VERSIONS(NoaryBaseIterator)

SERIALIZABLE_TEMPLATE_VERSIONS(UnaryBaseIterator)
END_SERIALIZABLE_TEMPLATE_VERSIONS(UnaryBaseIterator)

SERIALIZABLE_CLASS_VERSIONS(PlanIterator)
END_SERIALIZABLE_CLASS_VERSIONS(PlanIterator)


/*******************************************************************************
  class PlanState
********************************************************************************/
PlanState::PlanState(dynamic_context* dctx, uint32_t blockSize, uint32_t aStackDepth)
  :
  theBlockSize(blockSize),
  theStackDepth(aStackDepth),
  theCompilerCB(0),
  theQuery(0),
  theDynamicContext(dctx),
  theHasToQuit(false)
{
  assert(dctx != NULL);
  theBlock = new int8_t[theBlockSize];
}



void PlanState::checkDepth(const QueryLoc& loc)
{
  if (theStackDepth > 256)
    ZORBA_ERROR_LOC_PARAM(XQP0019_INTERNAL_ERROR, loc, "stack overflow", "");
}


PlanState::~PlanState()
{
  delete[] theBlock;
  theBlock = 0;
}


/*******************************************************************************
  class PlanIterator
********************************************************************************/

void PlanIterator::serialize(::zorba::serialization::Archiver &ar)
{
  ar & theStateOffset;
  ar & loc;
  if(ar.dont_allow_delay_for_plan_sctx)
    ar.dont_allow_delay();
  ar & theSctx;
}


#ifndef NDEBUG

bool PlanIterator::consumeNext(
    store::Item_t& result,
    const PlanIterator* iter,
    PlanState& planState)
{
  if (planState.theHasToQuit) 
  {
    // std::cout << "Plan interupted" << std::endl;

    throw FlowCtlException(FlowCtlException::INTERRUPT);
  }
  bool status = iter->produceNext(result, planState);

  if (planState.theCompilerCB->theConfig.print_item_flow)
  {
    std::cout << "next (" << iter << " = " << typeid (*iter).name()
              << ") -> "
              << "status: " << status << " -> "
              << ((status && result != NULL) ? result->show() : xqp_string("null"))
              << std::endl;
  }
  return status;
}
#endif


}
