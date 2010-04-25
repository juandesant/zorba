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
#include "zorbautils/fatal.h"
#include "zorbaerrors/Assert.h"
#include "zorbaerrors/error_manager.h"

#include "context/static_context.h"

#include "compiler/api/compilercb.h"

#include "runtime/core/flwor_iterator.h"
#include "runtime/core/gflwor/comp_function.h"
#include "runtime/api/runtimecb.h"
#include "runtime/api/plan_iterator_wrapper.h"
#include "runtime/visitors/planiter_visitor.h"

#include "system/globalenv.h"

#include "store/api/temp_seq.h"
#include "store/api/store.h"
#include "store/api/pul.h"
#include "store/api/item_factory.h"


#ifndef WIN32
#include <sys/time.h>
#endif

#include <algorithm>


namespace zorba
{

namespace flwor
{
SERIALIZABLE_CLASS_VERSIONS(OrderByClause)
END_SERIALIZABLE_CLASS_VERSIONS(OrderByClause)

SERIALIZABLE_CLASS_VERSIONS(GroupByClause)
END_SERIALIZABLE_CLASS_VERSIONS(GroupByClause)

SERIALIZABLE_CLASS_VERSIONS(ForLetClause)
END_SERIALIZABLE_CLASS_VERSIONS(ForLetClause)

SERIALIZABLE_CLASS_VERSIONS(FLWORIterator)
END_SERIALIZABLE_CLASS_VERSIONS(FLWORIterator)

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  ForLetClause                                                               //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  Creates a new ForClause for a FOR var.

  @param var The var_expr for this FOR variable.
  @param VarRefs The ForVarIterators representing the references to this var.
  @param aInput The iterator computing the domain expr of this variable.
********************************************************************************/
ForLetClause::ForLetClause(
    store::Item* aVarName,
    const std::vector<PlanIter_t>& varRefs,
    PlanIter_t& input)
  :
  theVarName(aVarName->getStringValue()),
  theType(FOR),
  theInput(input),
  theVarRefs(varRefs),
  theDoLazyEval(true)
{
}


/***************************************************************************//**
  Creates a new ForClause for a FOR var and its associated positional var.

  @param var The var_expr for this FOR variable.
  @param varRefs The ForVarIterators representing the references to this var.
  @param aPosVars ForVarIterators representing the references to the associated
         positional var.
  @param aInput The iterator computing the value of this variable.
********************************************************************************/
ForLetClause::ForLetClause(
    store::Item* aVarName,
    const std::vector<PlanIter_t>& varRefs,
    const std::vector<PlanIter_t>& posVarRefs,
    PlanIter_t& input)
  :
  theVarName(aVarName->getStringValue()),
  theType(FOR),
  theInput(input),
  theVarRefs(varRefs),
  thePosVarRefs(posVarRefs),
  theDoLazyEval(true)
{
}


/***************************************************************************//**
  Creates a new ForClause for a LET variable

  @param var The var_expr for this LET variable.
  @param aLetVars Vector of ForVar iterators representing the references to this
         variable.
  @param aInput The iterator computing the value of this variable.
********************************************************************************/
ForLetClause::ForLetClause(
    store::Item* aVarName,
    const std::vector<PlanIter_t>& varRefs,
    PlanIter_t& input,
    bool lazyEval,
    bool needsMaterialization)
  :
  theVarName(aVarName->getStringValue()),
  theType(LET),
  theInput(input),
  theVarRefs(varRefs),
  theDoLazyEval(lazyEval)
{
}


/***************************************************************************//**

********************************************************************************/
void ForLetClause::serialize(::zorba::serialization::Archiver& ar)
{
  ar & theVarName;
  SERIALIZE_ENUM(ForLetType, theType)
  ar & theVarRefs;
  ar & thePosVarRefs;
  ar & theInput;
  ar & theDoLazyEval;
}


/***************************************************************************//**

********************************************************************************/
void ForLetClause::accept(PlanIterVisitor& v) const
{
  // TODO correct for visitor
  switch ( theType )
  {
  case FOR:
    v.beginVisitFlworForVariable(getVarName(), theVarRefs, thePosVarRefs);
    theInput->accept(v);
    v.endVisitFlworForVariable();
    break;
  case LET:
    v.beginVisitFlworLetVariable(true, getVarName(), theVarRefs);
    theInput->accept(v);
    v.endVisitFlworLetVariable();
    break;
  default:
    ZORBA_ASSERT(false);
  }
}


/***************************************************************************//**

********************************************************************************/
xqpStringStore_t ForLetClause::getVarName() const
{
  return theVarName;
}



/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  OrderByClause                                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
OrderByClause::OrderByClause(
    const QueryLoc& loc,
    const std::vector<OrderSpec>& orderSpecs,
    bool stable)
  :
  theLocation(loc),
  theOrderSpecs(orderSpecs),
  theStable(stable)
{
}
  

/***************************************************************************//**

********************************************************************************/
void OrderByClause::accept(PlanIterVisitor& v) const
{
  std::vector<OrderSpec>::const_iterator iter;
  std::vector<OrderSpec>::const_iterator end = theOrderSpecs.end();
  for (iter = theOrderSpecs.begin(); iter != end; ++iter)
  {
    iter->accept(v);
  }
}


/***************************************************************************//**

********************************************************************************/
uint32_t OrderByClause::getStateSizeOfSubtree() const
{
  uint32_t size = 0;

  std::vector<OrderSpec>::const_iterator iter;
  std::vector<OrderSpec>::const_iterator end = theOrderSpecs.end();
  for (iter = theOrderSpecs.begin(); iter != end; ++iter)
  {
    size += iter->theDomainIter->getStateSizeOfSubtree();
  }

  return size;
}


/***************************************************************************//**

********************************************************************************/
void OrderByClause::open(static_context* sctx, PlanState& planState, uint32_t& offset)
{
  std::vector<OrderSpec>::iterator iter;
  std::vector<OrderSpec>::const_iterator end = theOrderSpecs.end();
  for (iter = theOrderSpecs.begin(); iter != end; ++iter)
  {
    iter->open(planState, offset);

    if (iter->theCollation.size() != 0) 
    {
      iter->theCollator = sctx->get_collator(iter->theCollation, theLocation);
    }
    else
    {
      iter->theCollator = sctx->get_default_collator(theLocation);
    }
  }
}


/***************************************************************************//**

********************************************************************************/
void OrderByClause::reset(PlanState& planState)
{
  std::vector<OrderSpec>::iterator iter = theOrderSpecs.begin();
  std::vector<OrderSpec>::iterator end = theOrderSpecs.end();
  for (; iter != end; iter++)
  {
    iter->theDomainIter->reset(planState);
  }
}


/***************************************************************************//**

********************************************************************************/
void OrderByClause::close(PlanState& planState)
{
  std::vector<OrderSpec>::iterator iter = theOrderSpecs.begin();
  std::vector<OrderSpec>::iterator end = theOrderSpecs.end();
  for (; iter != end; iter++)
  {
    iter->theDomainIter->close(planState);
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  GroupByClause                                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**

********************************************************************************/
GroupByClause::GroupByClause(
    const QueryLoc& loc,
    std::vector<GroupingSpec> aGroupingSpecs,
    std::vector<NonGroupingSpec> aNonGroupingSpecs)
  :
  theLocation(loc),
  theGroupingSpecs(aGroupingSpecs),
  theNonGroupingSpecs(aNonGroupingSpecs)
{
}


/***************************************************************************//**

********************************************************************************/
void GroupByClause::accept(PlanIterVisitor& v) const
{ 
  v.beginVisitGroupByClause();

  ulong numSpecs = theGroupingSpecs.size();
  for (ulong i = 0; i < numSpecs; ++i)
  {
    theGroupingSpecs[i].accept(v);
  }

  numSpecs = theNonGroupingSpecs.size();
  for (ulong i = 0; i < numSpecs; ++i)
  {
    theNonGroupingSpecs[i].accept(v);
  }

  v.endVisitGroupByClause();
}


/***************************************************************************//**

********************************************************************************/
uint32_t GroupByClause::getStateSizeOfSubtree() const
{
  uint32_t size = 0;

  ulong numSpecs = theGroupingSpecs.size();
  for (ulong i = 0; i < numSpecs; ++i)
  {
    size += theGroupingSpecs[i].getStateSizeOfSubtree();
  }
  
  numSpecs = theNonGroupingSpecs.size();
  for (ulong i = 0; i < numSpecs; ++i)
  {
    size += theNonGroupingSpecs[i].getStateSizeOfSubtree();
  }

  return size;
}


/***************************************************************************//**

********************************************************************************/
void GroupByClause::open(static_context* sctx, PlanState& planState, uint32_t& offset)
{ 
  std::vector<GroupingSpec>::iterator groupIter = theGroupingSpecs.begin();
  std::vector<GroupingSpec>::iterator groupEnd = theGroupingSpecs.end();
  for (; groupIter != groupEnd; ++groupIter)
  {
    groupIter->open(planState, offset);

    if (groupIter->theCollation.size() != 0) 
    {
      groupIter->theCollator = sctx->get_collator(groupIter->theCollation, theLocation);
    }
    else
    {
      groupIter->theCollator = sctx->get_default_collator(theLocation);
    }
  }

  std::vector<NonGroupingSpec>::iterator nongroupIter = theNonGroupingSpecs.begin();
  std::vector<NonGroupingSpec>::iterator nongroupEnd = theNonGroupingSpecs.end();
  for (; nongroupIter != nongroupEnd; ++nongroupIter)
  {
    nongroupIter->open(planState, offset);
  }
}


/***************************************************************************//**

********************************************************************************/
void GroupByClause::reset(PlanState& planState)
{
  std::vector<GroupingSpec>::const_iterator groupIter = theGroupingSpecs.begin();
  std::vector<GroupingSpec>::iterator groupEnd = theGroupingSpecs.end();
  for (; groupIter != groupEnd; ++groupIter)
  {
    groupIter->reset(planState);
  }

  std::vector<NonGroupingSpec>::iterator nongroupIter = theNonGroupingSpecs.begin();
  std::vector<NonGroupingSpec>::iterator nongroupEnd = theNonGroupingSpecs.end();
  for (; nongroupIter != nongroupEnd; ++nongroupIter)
  {
    nongroupIter->reset(planState);
  }
}


/***************************************************************************//**

********************************************************************************/
void GroupByClause::close(PlanState& planState)
{
  std::vector<GroupingSpec>::iterator groupIter =  theGroupingSpecs.begin();
  std::vector<GroupingSpec>::iterator groupEnd = theGroupingSpecs.end();
  for (; groupIter != groupEnd; ++groupIter)
  {
    groupIter->close(planState);
  }

  std::vector<NonGroupingSpec>::iterator nongroupIter = theNonGroupingSpecs.begin();
  std::vector<NonGroupingSpec>::iterator nongroupEnd = theNonGroupingSpecs.end();
  for (; nongroupIter != nongroupEnd; ++nongroupIter)
  {
    nongroupIter->close(planState);
  }
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  FlworState                                                                 //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

FlworState::FlworState() 
  :
  theNumTuples(0),
  theCurTuplePos(0),
  theGroupMap(0),
  theFirstResult(true)
{
}


/*******************************************************************************

********************************************************************************/
FlworState::~FlworState()
{
  clearSortTable();

  if (theGroupMap != NULL)
  {
    clearGroupMap();
    delete theGroupMap;
  }
}


/*******************************************************************************
  Init the state for a certain nb of variables but not the ordering
  @nb_variables  Number of FOR and LET clauses
********************************************************************************/
void FlworState::init(
    PlanState& planState,
    const std::vector<ForLetClause>& forletClauses)
{
  PlanIteratorState::init(planState);

  ulong numVars = forletClauses.size();
  std::vector<long> v(numVars, 0);
  theVarBindingState.swap(v);
  assert(theVarBindingState.size() > 0);

  theTempSeqs.resize(numVars);
  theTempSeqIters.resize(numVars);

  std::vector<ForLetClause>::const_iterator iter = forletClauses.begin();
  std::vector<ForLetClause>::const_iterator end = forletClauses.end();
  std::vector<store::TempSeq_t>::iterator seqiter = theTempSeqs.begin();
  std::vector<store::Iterator_t>::iterator domiter = theTempSeqIters.begin();

  for (; iter != end; ++iter, ++seqiter, ++domiter)
  {
    const ForLetClause& flc = *iter;

    if (flc.theType == ForLetClause::LET)
    {
      (*domiter) = new PlanIteratorWrapper(flc.theInput, planState);
      (*seqiter) = GENV_STORE.createTempSeq(flc.lazyEval());
    }
  }

  theNumTuples = 0;
  theCurTuplePos = 0;
  theFirstResult = true;
}


/*******************************************************************************
  Init the state for a certain nb of variables, ordering and grouping

  @nb_variables  Number of FOR and LET clauses
  @orderSpecs    The OrderSpec which defines how to compare during ordering
  @groupingCollators The GoupingCollators which defines how to compare during grouping
********************************************************************************/
void FlworState::init(
    PlanState& planState,
    TypeManager* tm,
    const std::vector<ForLetClause>& forletClauses,
    std::vector<OrderSpec>* orderSpecs,
    std::vector<GroupingSpec>* groupingSpecs)
{
  init(planState, forletClauses);
 
  if (groupingSpecs != 0)
  {
    GroupTupleCmp cmp(planState.theDynamicContext, tm, groupingSpecs);
    theGroupMap = new GroupHashMap(cmp, 1024, false);
  }
}


/*******************************************************************************
  Resets the state
********************************************************************************/
void FlworState::reset(PlanState& planState)
{
  PlanIteratorState::reset(planState);

  assert(theVarBindingState.size() > 0);

  size_t size = theVarBindingState.size();

  ::memset(&theVarBindingState[0], 0, size * sizeof(long));

  theFirstResult = true;

  if (theOrderResultIter != NULL)
  {
    clearSortTable();
    theDataTable.clear();
    theNumTuples = 0;
    theCurTuplePos = 0;

    theOrderResultIter = 0;
  }

  if (theGroupMap != NULL)
    clearGroupMap();
}


/*******************************************************************************

********************************************************************************/
void FlworState::clearSortTable()
{
  ulong numTuples = theSortTable.size();

  for (ulong i = 0; i < numTuples; ++i)
  {
    theSortTable[i].clear();
  }

  theSortTable.clear();
}

/*******************************************************************************

********************************************************************************/
void FlworState::clearGroupMap()
{
  GroupHashMap::iterator iter = theGroupMap->begin();
  GroupHashMap::iterator end = theGroupMap->end();
  for (; iter != end; ++iter)
  {
    delete (*iter).first;
    delete (*iter).second;
  }
  
  theGroupMap->clear();
}


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  FLWORIterator                                                              //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


/***************************************************************************//**
  Constructor

  @param loc location
  @param forLetClauses For and Lets: Attention the order matters!
  @param whereClause The where-clause iterator. Can be null
  @param orderByClause The order by expressions. Can be null
  @param returnClause The return expression.
  @param isUpdating Whether this flwor is an updating expr or not.

********************************************************************************/
FLWORIterator::FLWORIterator(
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<ForLetClause>& aForLetClauses,
    PlanIter_t& aWhereClause,
    GroupByClause* aGroupByClauses,
    OrderByClause* orderByClause,
    PlanIter_t& aReturnClause,
    bool aIsUpdating)
  :
  Batcher<FLWORIterator>(sctx, loc),
  theForLetClauses(aForLetClauses),
  theNumBindings(aForLetClauses.size()),
  theWhereClause(aWhereClause),
  theGroupByClause(aGroupByClauses),
  theOrderByClause(orderByClause),
  theReturnClause(aReturnClause),
  theIsUpdating(aIsUpdating)
{
  if (theOrderByClause != 0 && theOrderByClause->theOrderSpecs.size() == 0)
  {
    theOrderByClause = 0;
  }
}


/***************************************************************************//**

********************************************************************************/
FLWORIterator::~FLWORIterator()
{
  if (theOrderByClause)
  {
    delete theOrderByClause;
  }

  if (theGroupByClause)
  { 
    delete theGroupByClause;
  }
}


/***************************************************************************//**

********************************************************************************/
void FLWORIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar, (Batcher<FLWORIterator>*)this);
  ar & theForLetClauses;
  ar & theNumBindings;
  ar & theWhereClause; //can be null
  ar & theGroupByClause;
  ar & theOrderByClause;  //can be null
  ar & theReturnClause; 
  ar & theIsUpdating;
}


/*******************************************************************************

********************************************************************************/
bool FLWORIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  ulong curVar = 0;
  store::Item_t curItem;
  std::auto_ptr<store::PUL> pul;

  FlworState* iterState;
  DEFAULT_STACK_INIT(FlworState, iterState, planState);

  assert(iterState->theVarBindingState.size() > 0);

  if (theIsUpdating)
    pul.reset(GENV_ITEMFACTORY->createPendingUpdateList());

  while (true)
  {
    // Here we do the variable bindings from the outer most to the inner most
    while (curVar != theNumBindings)
    {
      // Try to bind the current variable. If the binding of the variable is
      // successfull, we procede with the next binding level. Else, we go a
      // level back and try the previous level
      if (bindVariable(curVar, iterState, planState))
      {
        ++curVar;
      }
      else
      {
        // If there are no more bindings for the outer-most var (curVar == 0),
        // then if we had to Order of Group we need to return the results,
        // otherwise we just need to indicate that we finished by returning NULL.
        if ( curVar == 0 )
        {
          if (theIsUpdating)
          {
            result = pul.release();
            STACK_PUSH(true, iterState);
          }
          else if (theOrderByClause)
          {
            if(theGroupByClause)
            {
              materializeGroupResultForSort(iterState, planState);
            }

            {
              SortTupleCmp cmp(planState.theDynamicContext,
                               theSctx->get_typemanager(),
                               &theOrderByClause->theOrderSpecs);

              if (theOrderByClause->theStable)
              {
                std::stable_sort(iterState->theSortTable.begin(),
                                 iterState->theSortTable.end(),
                                 cmp);
              }
              else
              {
                std::sort(iterState->theSortTable.begin(),
                          iterState->theSortTable.end(),
                          cmp);
              }
            }

            iterState->theCurTuplePos = 0;
            iterState->theNumTuples = iterState->theSortTable.size();

            while (iterState->theCurTuplePos < iterState->theNumTuples)
            {
              iterState->theOrderResultIter.transfer(iterState->theDataTable[iterState->theSortTable[iterState->theCurTuplePos].theDataPos]);

              iterState->theOrderResultIter->open();

              while (iterState->theOrderResultIter->next(result))
              {
                STACK_PUSH(true, iterState);
              }

             iterState->theOrderResultIter->close();

              ++(iterState->theCurTuplePos);
            }
          }
          else if (theGroupByClause)
          {
            iterState->theGroupMapIter = iterState->theGroupMap->begin();

            while (iterState->theGroupMapIter != iterState->theGroupMap->end())
            {
              bindGroupBy(iterState->theGroupMapIter, iterState, planState);

              while(consumeNext(result, theReturnClause, planState)) 
              {
                STACK_PUSH(true, iterState);
              }

              theReturnClause->reset(planState);
              ++iterState->theGroupMapIter;
            }
          }

          goto stop;
        }
        else
        {
          // No more values for the current variable (and the current variable
          // is  not the 1st one). We go back to the previous variable to
          // compute its next value (if any). Then, we will come back to this
          // variable again, and compute all its possible values from the 
          // begining. So, we mark this var's state as "to be reset", so that
          // the domain expr of the var will be reset before we try to compute
          // its new set of values.
          iterState->theVarBindingState[curVar] = -1;
          --curVar;
        }
      }
    } // build next full tuple of var bindings

    // After binding all variables, we check first the where clause
    if (theWhereClause == NULL || evalToBool(theWhereClause, planState))
    {
      // If we do not need to do ordering, grouping, or pul gneration, we 
      // compute and return the items produced by the ReturnClause. Else, we
      // have to  materialize the result.
      if (theGroupByClause)
      {
        matVarsAndGroupBy(iterState, planState);
      }
      else if (theIsUpdating)
      {
        if (!iterState->theFirstResult)
          theReturnClause->reset(planState);

        iterState->theFirstResult = false;

        while(consumeNext(curItem, theReturnClause, planState)) 
        {
          ZORBA_FATAL(curItem->isPul(), "");

          pul->mergeUpdates(curItem);
        }
      }
      else if ( !theOrderByClause )
      {
        if (!iterState->theFirstResult)
          theReturnClause->reset(planState);

        iterState->theFirstResult = false;

        while (consumeNext(result, theReturnClause, planState))
        {
          STACK_PUSH(true, iterState);
        }
      }
      else
      {
        materializeResultForSort(iterState, planState);
      }
    }

    // curVar = theNumBindings - 1 indicates that we have to do the next binding level
    curVar = theNumBindings - 1;
  }

 stop:
  STACK_END(iterState);
}



/***************************************************************************//**
  Compute the next value, if any, for the given var, and bind that value to all
  the references of the variable. Return true if there was a next value, and
  false otherwise.
********************************************************************************/
bool FLWORIterator::bindVariable(
    ulong varNo,
    FlworState* iterState,
    PlanState& planState) const
{
  const ForLetClause& flc = theForLetClauses[varNo];

  long& bindingState = iterState->theVarBindingState[varNo];

  if (bindingState == -1)
  {
    theForLetClauses[varNo].theInput->reset(planState);
    bindingState = 0;
  }

  switch (flc.theType)
  {
  // In the case of a FOR var, we compute the next item of the input and bind
  // it to all the variable references.
  case ForLetClause::FOR :
  {
    store::Item_t item;
    if (!consumeNext(item, flc.theInput, planState))
    {
      return false;
    }

    // We increase the position counter
    ++bindingState;

    std::vector<PlanIter_t>::const_iterator viter = flc.theVarRefs.begin();
    std::vector<PlanIter_t>::const_iterator end = flc.theVarRefs.end();
    for (; viter != end; ++viter)
    {
      static_cast<ForVarIterator*>
      ((*viter).getp())->bind(item.getp(), planState);
    }

    if (!flc.thePosVarRefs.empty())
    {
      store::Item_t posItem;
      GENV_ITEMFACTORY->createInteger(posItem, Integer::parseInt(bindingState));

      std::vector<PlanIter_t>::const_iterator viter = flc.thePosVarRefs.begin();
      std::vector<PlanIter_t>::const_iterator end = flc.thePosVarRefs.end();
      for (; viter != end; ++viter)
      {
        static_cast<ForVarIterator*>
        ((*viter).getp())->bind(posItem.getp(), planState);
      }
    }

    return true;
  }
  case ForLetClause::LET :
  {      
    // If the var is already bound, there is no next value for it, so return false.
    if (bindingState == 1)
    {
      return false;
    }

    store::TempSeq_t tmpSeq = iterState->theTempSeqs[varNo].getp();
    tmpSeq->init(iterState->theTempSeqIters[varNo], false);

    std::vector<PlanIter_t>::const_iterator viter = flc.theVarRefs.begin();
    std::vector<PlanIter_t>::const_iterator end = flc.theVarRefs.end();
    for (; viter != end; ++viter)
    {
      static_cast<LetVarIterator*>
      ((*viter).getp())->bind(tmpSeq, planState);
    }

    bindingState = 1;
    return true;
  }
  default:
  {
    ZORBA_ASSERT(false);
  }
  }

  return false;
}


/***************************************************************************//**
  Evaluates the predicateIter to the EBV. If the predicateIter = NULL true is
  returned.
********************************************************************************/
bool FLWORIterator::evalToBool(
    const PlanIter_t& predicateIter,
    PlanState& planState) const
{
  store::Item_t boolValue;
  if (!consumeNext(boolValue, predicateIter.getp(), planState))
    return false;

  bool value = boolValue->getBooleanValue();
  predicateIter->reset(planState);
  return value;
}


/***************************************************************************//**
  All FOR and LET vars are bound when this method is called. The method computes
  the order-by tuple T and the return-clause sequence R for the current var
  bindings. Then, it inserts the pair (T, I(R)) into theSortArray (where I is
  an iterator over the temp seq storing R).
********************************************************************************/
void FLWORIterator::materializeResultForSort(
    FlworState* iterState,
    PlanState& planState) const
{
  ZORBA_ASSERT(theOrderByClause);

  FlworState::SortTable& sortTable = iterState->theSortTable;
  FlworState::DataTable& dataTable = iterState->theDataTable;

  ulong numTuples = sortTable.size();
  sortTable.resize(numTuples + 1);
  dataTable.resize(numTuples + 1);

  // Create the sort tuple

  std::vector<OrderSpec>& orderSpecs = theOrderByClause->theOrderSpecs;
  ulong numSpecs = orderSpecs.size();

  std::vector<store::Item*>& sortKey = sortTable[numTuples].theKeyValues;
  sortKey.resize(numSpecs);

  for (ulong i = 0; i < numSpecs; ++i)
  {
    store::Item_t sortKeyItem;
    if (consumeNext(sortKeyItem, orderSpecs[i].theDomainIter, planState))
    {
      sortKey[i] = sortKeyItem.release();

      store::Item_t temp;
      if (consumeNext(temp, orderSpecs[i].theDomainIter, planState))
      {
        ZORBA_ERROR_DESC(XPTY0004, "Expected a singleton");
      }
    }
    else
    {
      sortKey[i] = NULL;
    }

    orderSpecs[i].theDomainIter->reset(planState);
  }

  sortTable[numTuples].theDataPos = numTuples;

  store::Iterator_t iterWrapper = new PlanIteratorWrapper(theReturnClause, planState);
  store::TempSeq_t resultSeq = GENV_STORE.createTempSeq(iterWrapper, false, false);
  store::Iterator_t resultIter = resultSeq->getIterator();

  dataTable[numTuples].transfer(resultIter);

  theReturnClause->reset(planState); 
}


/*******************************************************************************

********************************************************************************/
void FLWORIterator::materializeGroupResultForSort(
    FlworState* iterState,
    PlanState& planState) const
{
  GroupHashMap* groupMap = iterState->theGroupMap;

  GroupHashMap::iterator groupMapIter = groupMap->begin();
  GroupHashMap::iterator groupMapEnd = groupMap->end();

  while (groupMapIter != groupMapEnd)
  {
    bindGroupBy(groupMapIter, iterState, planState);
  
    materializeResultForSort(iterState, planState);

    ++groupMapIter;
  }
}


/***************************************************************************//**
  All FOR and LET vars are bound when this method is called. The method computes
  the order-by tuple T and then checks whether T is in the GroupMap already. If
  not, it inserts T in the GroupMap, together with one temp sequence for each of
  the non-grouping vars, storing the current value of the non-grouping var. If
  yes, it appends to each of the temp sequences associated with T the current
  value of each non-grouping var.
********************************************************************************/
void FLWORIterator::matVarsAndGroupBy(
    FlworState* iterState,
    PlanState& planState) const
{
  ZORBA_ASSERT(theGroupByClause);

  GroupTuple* groupTuple = new GroupTuple();
  std::vector<store::Item_t>& lGroupTupleItems = groupTuple->theItems;
  std::vector<store::Item_t>& lGroupTupleValues = groupTuple->theTypedValues;

  std::vector<GroupingSpec> lgroupSpecs = theGroupByClause->theGroupingSpecs;
  std::vector<GroupingSpec>::iterator lSpecIter = lgroupSpecs.begin();
  std::vector<GroupingSpec>::iterator end = lgroupSpecs.end();

  while ( lSpecIter != end )
  {
    lGroupTupleItems.push_back(NULL);
    store::Item_t& tupleItem = lGroupTupleItems.back();
    
    lGroupTupleValues.push_back(NULL);
    store::Item_t& tupleValue = lGroupTupleValues.back();

    bool status = consumeNext(tupleItem, lSpecIter->theInput, planState);

    if(status)
    {
      store::Iterator_t lTypedValueIter;

      tupleItem->getTypedValue(tupleValue, lTypedValueIter);

      if (lTypedValueIter != NULL)
      {
        lTypedValueIter->open();
        if(lTypedValueIter->next(tupleValue))
        {
          store::Item_t temp;
          if (lTypedValueIter->next(temp))
          {
            ZORBA_ERROR_DESC(XPTY0004,
                             "Expected a singleton (atomization has more than one value)" );
          }
        }
      }

      // check that there are no more values for the current grouping column
      store::Item_t temp;
      if (consumeNext(temp, lSpecIter->theInput, planState))
      {
        ZORBA_ERROR_DESC(XPTY0004, "Expected a singleton");
      }
    }

    lSpecIter->reset(planState);
    ++lSpecIter;
  }

  GroupHashMap* groupMap = iterState->theGroupMap;

  std::vector<NonGroupingSpec> outerVars = theGroupByClause->theNonGroupingSpecs;
  std::vector<store::TempSeq_t>* outerVarSequences = 0;
  long numNonGroupingSpecs = outerVars.size();

  if (groupMap->get(groupTuple, outerVarSequences))
  {
    for (long i = 0; i < numNonGroupingSpecs; ++i)
    {
      store::Iterator_t iterWrapper = new PlanIteratorWrapper(outerVars[i].theInput,
                                                              planState);
      (*outerVarSequences)[i]->append(iterWrapper, true);

      outerVars[i].reset(planState);
    }

    delete groupTuple;
  }
  else
  {
    outerVarSequences = new std::vector<store::TempSeq_t>();

    for (long i = 0; i < numNonGroupingSpecs; ++i)
    {
      store::Iterator_t iterWrapper = new PlanIteratorWrapper(outerVars[i].theInput,
                                                              planState);
      store::TempSeq_t result = GENV_STORE.createTempSeq(iterWrapper, true, false);

      outerVarSequences->push_back(result);

      outerVars[i].reset(planState);
    }

    groupMap->insert(groupTuple, outerVarSequences);
  }
}


/***************************************************************************//**
  Binds the values in current tuple of the group map to the var references
  that appear after the groupby clause. 
********************************************************************************/
void FLWORIterator::bindGroupBy( 
    GroupHashMap::iterator groupMapIter,
    FlworState* iterState,
    PlanState& planState) const
{
  // Bind grouping vars
  GroupTuple* groupKey = (*groupMapIter).first;
  std::vector<store::Item_t>::iterator groupKeyIter = groupKey->theItems.begin();

  std::vector<GroupingSpec> groupSpecs = theGroupByClause->theGroupingSpecs;
  std::vector<GroupingSpec>::const_iterator specIter = groupSpecs.begin();
  std::vector<GroupingSpec>::const_iterator specEnd = groupSpecs.end();

  while (specIter != specEnd)
  {
    std::vector<ForVarIter_t>::const_iterator groupVarIter = specIter->theVarRefs.begin();
    std::vector<ForVarIter_t>::const_iterator groupVarEnd = specIter->theVarRefs.end();

    while (groupVarIter != groupVarEnd)
    {
      (*groupVarIter)->bind(*groupKeyIter, planState);
      ++groupVarIter;
    }

    ++specIter;
    ++groupKeyIter;
  }

  // Bind non-grouping vars
  std::vector<store::TempSeq_t>* lVector = (*groupMapIter).second;
  std::vector<store::TempSeq_t>::const_iterator lOuterSeqIter = lVector->begin();
  std::vector<NonGroupingSpec> lNonGroupingSpecs = theGroupByClause->theNonGroupingSpecs;
  std::vector<NonGroupingSpec>::const_iterator lNonGroupingSpecsIter = lNonGroupingSpecs.begin();

  while (lNonGroupingSpecsIter != lNonGroupingSpecs.end())
  {
    std::vector<LetVarIter_t>::const_iterator lOuterVarBindingIter = lNonGroupingSpecsIter->theVarRefs.begin();
    while (lOuterVarBindingIter != lNonGroupingSpecsIter->theVarRefs.end())
    {
      store::TempSeq_t lTmpSeq = *lOuterSeqIter;
      store::Iterator_t lBindIterator = lTmpSeq->getIterator();
      lBindIterator->open();
      ( *lOuterVarBindingIter )->bind(lBindIterator , planState);
      ++lOuterVarBindingIter;
    }
    ++lNonGroupingSpecsIter;
    ++lOuterSeqIter;
  }
}


/*******************************************************************************

********************************************************************************/
void FLWORIterator::openImpl(PlanState& planState, uint32_t& offset)
{
  StateTraitsImpl<FlworState>::createState(planState, theStateOffset, offset);

  FlworState* iterState = StateTraitsImpl<FlworState>::getState(planState,
                                                                theStateOffset);

  std::vector<ForLetClause>::const_iterator iter = theForLetClauses.begin();
  std::vector<ForLetClause>::const_iterator end = theForLetClauses.end();
  for (; iter != end; ++iter)
  {
    iter->theInput->open(planState, offset);
  }

  theReturnClause->open(planState, offset);

  if (theWhereClause != NULL)
    theWhereClause->open(planState, offset);
  
  if (theGroupByClause)
    theGroupByClause->open(theSctx, planState, offset);
  
  if (theOrderByClause)
    theOrderByClause->open(theSctx, planState, offset);

  if (theGroupByClause || theOrderByClause)
  {
    if (theGroupByClause)
    {
      iterState->init(planState,
                      theSctx->get_typemanager(),
                      theForLetClauses,
                      (theOrderByClause ? &theOrderByClause->theOrderSpecs : NULL),
                      &theGroupByClause->theGroupingSpecs); 
    }
    else if (theOrderByClause) 
    {
      iterState->init(planState,
                      theSctx->get_typemanager(),
                      theForLetClauses,
                      &theOrderByClause->theOrderSpecs,
                      0);
    }
  }
  else 
  {
    iterState->init(planState, theForLetClauses);
  }

  // some variables must have been bound
  assert(iterState->theVarBindingState.size() > 0);
}


/*******************************************************************************

********************************************************************************/
void FLWORIterator::resetImpl(PlanState& planState) const
{
  theReturnClause->reset(planState);

  if (theWhereClause != NULL)
    theWhereClause->reset(planState);

  if (theOrderByClause != NULL)
    theOrderByClause->reset(planState);
  
  if (theGroupByClause != NULL)
    theGroupByClause->reset(planState);

  std::vector<ForLetClause>::const_iterator iter = theForLetClauses.begin();
  std::vector<ForLetClause>::const_iterator end = theForLetClauses.end();
  for (; iter != end; ++iter)
  {
    iter->theInput->reset(planState);
  }

  StateTraitsImpl<FlworState>::reset(planState, theStateOffset);
}


/*******************************************************************************

********************************************************************************/
void FLWORIterator::closeImpl(PlanState& planState)
{
  theReturnClause->close(planState);

  if (theWhereClause != 0)
    theWhereClause->close(planState);

  if (theOrderByClause != 0)
    theOrderByClause->close(planState);

  if(theGroupByClause != 0)
    theGroupByClause->close(planState);

  std::vector<ForLetClause>::const_iterator iter = theForLetClauses.begin();
  std::vector<ForLetClause>::const_iterator end = theForLetClauses.end();
  for (; iter != end; ++iter)
  {
    iter->theInput->close(planState);
  }
  
  StateTraitsImpl<FlworState>::destroyState(planState, theStateOffset);
}


/*******************************************************************************

********************************************************************************/
uint32_t FLWORIterator::getStateSize() const  
{
  return sizeof(FlworState); 
}


/*******************************************************************************

********************************************************************************/
uint32_t FLWORIterator::getStateSizeOfSubtree() const
{
  int32_t size = this->getStateSize();

  std::vector<ForLetClause>::const_iterator iter = theForLetClauses.begin();
  std::vector<ForLetClause>::const_iterator end = theForLetClauses.end();
  for (; iter != end; ++iter)
  {
    size += iter->theInput->getStateSizeOfSubtree();
  }

  size += theReturnClause->getStateSizeOfSubtree();

  if (theWhereClause != NULL)
    size += theWhereClause->getStateSizeOfSubtree();

  if (theOrderByClause)
    size += theOrderByClause->getStateSizeOfSubtree();

  if (theGroupByClause)
    size += theGroupByClause->getStateSizeOfSubtree();

  return size;
}


/*******************************************************************************

********************************************************************************/
void FLWORIterator::accept(PlanIterVisitor& v) const
{
  v.beginVisit(*this);

  std::vector<ForLetClause>::const_iterator iter = theForLetClauses.begin();
  std::vector<ForLetClause>::const_iterator end = theForLetClauses.end();
  for (; iter != end; ++iter)
  {
    iter->accept(v);
  }

  if (theWhereClause != NULL)
  {
    v.beginVisitFlworWhereClause(*theWhereClause);
    v.endVisitFlworWhereClause(*theWhereClause);
  }

  if (theGroupByClause)
    theGroupByClause->accept(v);
 
  if (theOrderByClause)
    theOrderByClause->accept(v);

  v.beginVisitFlworReturn(*theReturnClause);
  v.endVisitFlworReturn(*theReturnClause);

  v.endVisit(*this);
}



}
}
