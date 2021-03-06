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

#include "stdafx.h"

#include "debugger_runtime.h"

#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <zorba/util/uri.h>

#include "store/api/store.h"

#include "api/zorbaimpl.h"
#include "api/xqueryimpl.h"
#include "api/serializerimpl.h"

#include "context/static_context.h"
#include "compiler/expression/var_expr.h"

#include "runtime/api/plan_wrapper.h"
#include "runtime/debug/debug_iterator.h"
#include "runtime/util/flowctl_exception.h"

#include "zorba/diagnostic_handler.h"
#include "diagnostics/xquery_diagnostics.h"
#include "zorbautils/synchronous_logger.h"

#include "debugger/debugger_protocol.h"
#include "debugger/debugger_communicator.h"
#include "debugger/debugger_commons.h"

#ifdef WIN32
#include <windows.h>
#define sleep(s) Sleep(s*1000)
#endif

namespace zorba {

DebuggerRuntime::DebuggerRuntime(
  XQueryImpl* xqueryImpl,
  std::ostream& oStream,
  Zorba_SerializerOptions& serializerOptions,
  DebuggerCommunicator* communicator,
  itemHandler aHandler,
  void* aCallBackData,
  bool* aInterruptBreak)
  : theQuery(xqueryImpl),
    theOStream(oStream),
    theSerializerOptions(serializerOptions),
    theCommunicator(communicator),
    theWrapper(theQuery->generateWrapper()),
    theExecStatus(QUERY_IDLE),
    thePlanIsOpen(false),
    theSerializer(0),
    theItemHandler(aHandler),
    theCallbackData(aCallBackData),
    theLastContinuationCommand(),
    theInterruptBreak(aInterruptBreak)
{
}

DebuggerRuntime::~DebuggerRuntime()
{
  delete theSerializer;
}

// this is the main loop of the thread
// the runtime thread terminates if this method finishes
void
DebuggerRuntime::run()
{
  theWrapper->open();
  thePlanIsOpen = true;
  runQuery();

  std::stringstream lResult;
  lResult << "<response command=\"" << theLastContinuationCommand.second << "\" "
    << "status=\"stopping\" "
    << "reason=\"ok\" "
    << "transaction_id=\"" << theLastContinuationCommand.first << "\">"
    << "</response>";
  theCommunicator->send(lResult.str());
}

void
DebuggerRuntime::finish()
{
  if (thePlanIsOpen) {
    theWrapper->close();
  }
  terminateRuntime();
}


void
DebuggerRuntime::resetRuntime()
{
  theWrapper = theQuery->generateWrapper();
  thePlanIsOpen = false;
  theExecStatus = QUERY_IDLE;
  reset();
}


ExecutionStatus
DebuggerRuntime::getExecutionStatus() const
{
  AutoLock lLock(theLock, Lock::READ);
  return theExecStatus;
}


void
DebuggerRuntime::runQuery()
{
  theLock.wlock();
  theExecStatus = QUERY_RUNNING;

  try {
    DebuggerCommons* lCommons = getDebbugerCommons();
    lCommons->setRuntime(this);

    theLock.unlock();

    delete theSerializer;
    theSerializer = new serializer(theQuery->theXQueryDiagnostics);
    SerializerImpl::setSerializationParameters(
      *theSerializer, theSerializerOptions);
    if (theItemHandler) {
      theSerializer->serialize(theWrapper.getp(), theOStream,
        theItemHandler, theCallbackData);
    } else {
      theSerializer->serialize(theWrapper.getp(), theOStream);
    }

    theOStream.flush();
  } catch (FlowCtlException&) {
    // Runtime correctly terminated by user interrupt
  } catch (ZorbaException const& e) {
    std::cerr << e << std::endl;
  }
  theLock.wlock();
  theExecStatus = QUERY_TERMINATED;
  theLock.unlock();
}

// ****************************************************************************
// Breakpoints

unsigned int
DebuggerRuntime::addBreakpoint(String& aFileName, unsigned int aLine, bool aEnabled)
{
  AutoLock lLock(theLock, Lock::WRITE);
  DebuggerCommons* lCommons = getDebbugerCommons();
  return lCommons->addBreakpoint(aFileName, aLine, aEnabled);
}

Breakable
DebuggerRuntime::getBreakpoint(unsigned int aId)
{
  AutoLock lLock(theLock, Lock::WRITE);
  DebuggerCommons* lCommons = getDebbugerCommons();
  return lCommons->getBreakpoint(aId);
}

BreakableVector
DebuggerRuntime::getBreakpoints()
{
  AutoLock lLock(theLock, Lock::WRITE);
  return getDebbugerCommons()->getBreakpoints();
}

void
DebuggerRuntime::updateBreakpoint(
  unsigned int aId,
  bool aEnabled,
  std::string aCondition,
  unsigned int aHitValue)
{
  AutoLock lLock(theLock, Lock::WRITE);

  DebuggerCommons* lCommons = getDebbugerCommons();

  if (aCondition == "") {
    lCommons->updateBreakpoint(aId, aEnabled);
  } else {
    lCommons->updateBreakpoint(aId, aEnabled, aCondition, aHitValue);
  }
}

void
DebuggerRuntime::removeBreakpoint(unsigned int aId)
{
  AutoLock lLock(theLock, Lock::WRITE);
  DebuggerCommons* lCommons = getDebbugerCommons();
  lCommons->removeBreakpoint(aId);
}

// ****************************************************************************
// Stack frames

unsigned int
DebuggerRuntime::getStackDepth()
{
  return theWrapper->thePlanState->theStackDepth;
}

std::vector<StackFrameImpl>
DebuggerRuntime::getStackFrames()
{
  // this is only working while execution is suspended
  if (theExecStatus != QUERY_SUSPENDED) {
    std::string lMessage("I can only show a stack if the execution is suspended.");
    throw lMessage;
  }

  std::vector<StackFrameImpl> lFrames;

  DebuggerCommons* lCommons = getDebbugerCommons();
  std::vector<std::pair<QueryLoc, std::string> > lRawFrames = lCommons->getStackFrames();

  QueryLoc lLocation;
  std::string lSignature("query body");

  // add the frames for each function call
  for (std::size_t i = 0 ; i < lRawFrames.size(); i++) {
    lLocation = lRawFrames.at(i).first;
    StackFrameImpl lFrame(lSignature, lLocation);
    lFrames.push_back(lFrame);
    lSignature = lRawFrames.at(i).second;
  }

  // add the top most frame from the current iterator
  const DebugIterator* lIterator = lCommons->getCurrentIterator();
  lLocation = lIterator->loc;
  StackFrameImpl lFrame(lSignature, lLocation);
  lFrames.push_back(lFrame);

  return lFrames;
}

// ****************************************************************************
// Other

void
DebuggerRuntime::suspendRuntime(QueryLoc aLocation, SuspensionCause aCause)
{
  theLock.wlock();
  theExecStatus = QUERY_SUSPENDED;

  std::stringstream lResponse;
  lResponse << "<response "
    << "command=\"" << theLastContinuationCommand.second << "\" "
    << "transaction_id=\"" << theLastContinuationCommand.first << "\" "
    << "reason=\"ok\" status=\"break\" "
    << ">";

  // if available, show the location where the execution is suspended
  // this should be the top-most stack frame
  std::vector<StackFrameImpl> lFrames = getStackFrames();
  if (lFrames.size() > 0) {
    // wrap in CDATA because xml elements might come along
    lResponse << "<![CDATA[";

    StackFrameImpl lFrame = lFrames[lFrames.size() - 1];
    unsigned int lBeginLine = lFrame.getLocation().getLineBegin();
    unsigned int lEndLine = lFrame.getLocation().getLineEnd();
    String lFileName = lFrame.getLocation().getFileName();
    lResponse  << lFrame.getSignature()
      << " at " << lFileName
      << ":" << lBeginLine;

    // show also the current line
    lResponse << std::endl << std::endl;
    lResponse << listSource(lFileName, lBeginLine, lEndLine, true);

    lResponse << "]]>";
  }

  lResponse << "</response>";
  theCommunicator->send(lResponse.str());

  theLock.unlock();
  suspend();
}


void
DebuggerRuntime::startRuntime()
{
  AutoLock lLock(theLock, Lock::WRITE);
  if (theExecStatus == QUERY_RUNNING) {
    return;
  }
  theExecStatus = QUERY_RUNNING;
  start();
}


void
DebuggerRuntime::resumeRuntime()
{
  AutoLock lLock(theLock, Lock::WRITE);
  if (theExecStatus != QUERY_SUSPENDED) {
    return;
  }
  *theInterruptBreak = false;
  theExecStatus = QUERY_RUNNING;
  resume();
}


void
DebuggerRuntime::terminateRuntime()
{
  AutoLock lLock(theLock, Lock::WRITE);
  theExecStatus = QUERY_TERMINATED;
}


void
DebuggerRuntime::detachRuntime()
{
  AutoLock lLock(theLock, Lock::WRITE);
  theExecStatus = QUERY_DETACHED;
  // TODO: something more here?
}


std::vector<std::pair<std::string, std::string> >
DebuggerRuntime::getVariables()
{
  std::vector<std::pair<std::string, std::string> > lVarResult;

  std::vector<std::pair<std::string, std::string> > lVarTmp;
  std::vector<std::pair<std::string, std::string> >::iterator lIter;

  // get the locals and add them to result
  lVarTmp = getVariables(true);
  for (lIter = lVarTmp.begin(); lIter != lVarTmp.end(); lIter++) {
    lVarResult.push_back(*lIter);
  }

  // get the globals and add them to result
  lVarTmp = getVariables(false);
  for (lIter = lVarTmp.begin(); lIter != lVarTmp.end(); lIter++) {
    lVarResult.push_back(*lIter);
  }

  return lVarResult;
};


std::vector<std::pair<std::string, std::string> >
DebuggerRuntime::getVariables(bool aLocals)
{
  DebuggerCommons* lCommons = getDebbugerCommons();
  static_context* lContext = lCommons->getCurrentStaticContext();

  // get all visible variables and filter below
  std::vector<VarInfo*> lVars;
  lContext->getVariables(lVars, false);

  std::vector<std::pair<std::string, std::string> > lVarList;
  std::vector<VarInfo*>::iterator lIte = lVars.begin();
  std::vector<VarInfo*>::iterator lEnd = lVars.end();

  for (; lIte != lEnd; ++lIte)
  {
    // non-global to locals and globals to globals
    if ((aLocals && (*lIte)->getKind() == var_expr::prolog_var) ||
        (!aLocals && (*lIte)->getKind() != var_expr::prolog_var))
    {
      continue;
    }

    std::stringstream lTypeSs;
    std::stringstream lNameSs;


    // read the name ****************************
    const store::Item_t& lNameItem = (*lIte)->getName();
    zstring lLocalName = lNameItem->getLocalName();

    // correct the name of the context item
    if (!aLocals && lLocalName == "$$dot")
    {
      lVarList.push_back(std::pair<std::string, std::string>(".", "item()*"));
      continue;
    }

    bool lHasNS = ! lNameItem->getNamespace().empty();

    // if there is a namespace, append the local name as well
    if (lHasNS) 
    {
      lNameSs << lNameItem->getPrefix().str() << ":";
    }
    lNameSs << lNameItem->getLocalName().str();

    // if there is a namespace, append the namespace URI
    if (lHasNS)
    {
      lNameSs << "\\" << lNameItem->getNamespace().str();
    }

    // read the type ****************************
    xqtref_t lType = (*lIte)->getType();

    if (lType == NULL)
    {
      lTypeSs << "item()*";
    }
    else
    {
      lTypeSs << lType->toSchemaString();
    }


    // add a new result
    lVarList.push_back(
      std::pair<std::string, std::string>(lNameSs.str(), lTypeSs.str())
    );
  }

  return lVarList;
}


void
DebuggerRuntime::stepIn()
{
  DebuggerCommons* lCommons = theWrapper->thePlanState->theDebuggerCommons;
  // Resume and then suspend as soon as the next iterator is reached.
  lCommons->setBreak(true, CAUSE_STEP);

  if (theExecStatus == QUERY_SUSPENDED) {
    resumeRuntime();
  } else {
    startRuntime();
  }
}


void
DebuggerRuntime::stepOver()
{
  DebuggerCommons* lCommons = theWrapper->thePlanState->theDebuggerCommons;
  lCommons->makeStepOver();

  if (theExecStatus == QUERY_SUSPENDED) {
    resumeRuntime();
  } else {
    startRuntime();
  }
}


void
DebuggerRuntime::stepOut()
{
  // this combination doesn't make sense: to step out when the exeecution did not start yet
  if (theExecStatus != QUERY_SUSPENDED) {
    return;
  }

  DebuggerCommons* lCommons = theWrapper->thePlanState->theDebuggerCommons;
  lCommons->makeStepOut();

  resumeRuntime();
}


bool
DebuggerRuntime::getAndClearInterruptBreak()
{
  bool lMustBreak = *theInterruptBreak;
  if (lMustBreak) {
    *theInterruptBreak = false;
  }
  return lMustBreak;
}


std::list<std::pair<zstring, zstring> >
DebuggerRuntime::eval(zstring& aExpr)
{
  // must check if we actually CAN eval (only while in "break" status)
  if (theExecStatus != QUERY_SUSPENDED) {
    std::string lMessage("I can only eval expressions if the execution is suspended.");
    throw lMessage;
  }


  // disable the xml declaration for evals/variables
  Zorba_omit_xml_declaration_t lOldOpt = theSerializerOptions.omit_xml_declaration;
  theSerializerOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
  DebuggerCommons* lCommons = getDebbugerCommons();
  std::list<std::pair<zstring, zstring> > lResults;

  try {
    lResults = lCommons->eval(aExpr, theSerializerOptions);
  } catch (...) {
    // restore the initial omit xml declaration option
    theSerializerOptions.omit_xml_declaration = lOldOpt;
    throw;
  }

  // restore the initial omit xml declaration option
  theSerializerOptions.omit_xml_declaration = lOldOpt;
  return lResults;
}


std::string
DebuggerRuntime::listSource(
  String& lFileName,
  unsigned int aBeginLine,
  unsigned int aEndLine,
  bool aZorbaExtensions)
{
  // these are needed if extensions are enabled
  String lFileUri;
  unsigned int lCurrentLine = 0;

  // for unspecified files determine the file
  if (lFileName == "") {
    // if not started, than it's the main module
    if (theExecStatus != QUERY_SUSPENDED) {
#ifdef WIN32
      lFileName = theQuery->getFileName();
#else
      // TODO: under Linux, when trying to get the file name of the query
      //       the call fails because getFileName tries to get a lock that
      //       is already taken. Therefore the assertion in mutex.cpp:63
      //       terminates the execution
      lFileName = "";
#endif
    }
    // else, the file pointed by the top-most stack frame
    else {
      std::vector<StackFrameImpl> lRawFrames = getStackFrames();
      StackFrameImpl lFrame = lRawFrames.at(lRawFrames.size() - 1);
      lFileName = lFrame.getLocation().getFileName();
      // filter out the eval stack frames. They Start with "eval@"
      // TODO: Unsafe if the stackframe changes notation.
      //       Should use a constant taken from the stack frame logic.
      if (lFileName.substr(0, 5) == "eval@") {
        // TODO: can we get the eval iterator, get its source and surprize the user
        //       with the source code of the eval code. That would be great!
        std::stringstream lSs;
        lSs << "Cannot list source code for stack frames inside an eval expression. "
          << "(Yet! Try again in the next version!)";
        throw lSs.str();
      }
      lCurrentLine = lFrame.getLocation().getLineBegin();
    }
  }

#ifdef WIN32
  // TODO: tolower all the file names/URIs on Windows
  // This must be implemented also in the breakable/breakpoint logic
#endif

  // normalize file uri and file name
  String lPrefix = lFileName.substr(0, 7);
  if (lPrefix == "file://") {
    lFileName = URIHelper::decodeFileURI(lFileName);
    lFileUri = URIHelper::encodeFileURI(lFileName);
  } else {
    if (lPrefix != "http://" && lPrefix != "https:/") {
      lFileUri = URIHelper::encodeFileURI(lFileName);
    } else {
      lFileUri = lFileName;
    }
  }

  // check if this file is actually one file where breakpoints are allowed
  // this prevents the user opening random files in the system
  BreakableVector lBkps = getBreakpoints();
  bool lFoundFile = false;
  for (BreakableVector::size_type j = 0; j < lBkps.size(); j++) {
    Breakable lBkp = lBkps.at(j);
    if (lBkp.getLocation().getFilename().str() == lFileUri.str()) {
      lFoundFile = true;
      break;
    }
  }

  // TODO: if lZorbaExtensions and file not found add search capability like
  // for breakpoints. This ways the useer can specify only a file name suffix

  if (!lFoundFile) {
    std::stringstream lSs;
    lSs << "Cannot open file: " << lFileUri;
    throw lSs.str();
  }

  // if a file is given check if this is the one in the top-most stack frame
  if (lCurrentLine == 0 && aZorbaExtensions && theExecStatus == QUERY_SUSPENDED) {
    std::vector<StackFrameImpl> lRawFrames = getStackFrames();
    StackFrameImpl lFrame = lRawFrames.at(lRawFrames.size() - 1);
    if (lFileUri == lFrame.getLocation().getFileName()) {
      lCurrentLine = lFrame.getLocation().getLineBegin();
    }
  }


  // read the entire file
  std::ifstream lStream(lFileName.c_str());
  std::string lCurrLine;
  std::vector<std::string> lFileContent;

  while (lStream.good()) {
    std::getline(lStream, lCurrLine);
    lFileContent.push_back(lCurrLine);
  }
  unsigned int lLineCount = lFileContent.size();

  // ocmpute the begin and end lines depending on the current line:
  // - 5 lines before and after the current line if available
  // - all the file otherwise
  unsigned int lLines = 5;
  unsigned int lClBl = lCurrentLine == 0  || lCurrentLine <= lLines ? 1 : lCurrentLine - lLines;
  unsigned int lClEl = lCurrentLine == 0  || lCurrentLine >= lLineCount - lLines ? lLineCount : lCurrentLine + lLines;
  aBeginLine = std::min(lLineCount, (aBeginLine == 0 ? lClBl : aBeginLine));
  aEndLine = std::min(lLineCount, (aEndLine == 0 ? lClEl : aEndLine));

  // first, find the breakpoints in this file
  std::map<int, bool> lBreakLines;
  if (aBeginLine <= aEndLine) {
    for (BreakableVector::size_type j = 0; j < lBkps.size(); j++) {
      Breakable lBkp = lBkps.at(j);
      if (lBkp.isSet() && lBkp.getLocation().getFilename().str() == lFileUri.str()) {
        // TODO: one could also check if the line in in the range we want
        lBreakLines[lBkp.getLocation().getLineBegin()] = lBkp.isEnabled();
      }
    }
  }

  // get only the needed lines
  std::stringstream lOut;
  for (unsigned int i = aBeginLine; i <= aEndLine; i++) {
    if (aZorbaExtensions) {
      // get the width of the line number column
      std::stringstream lTmpSs;
      lTmpSs << aEndLine;
      int lWidth = lTmpSs.str().length();

      // prepend line info column to the source
      lOut << std::setw(lWidth) << i << " ";
      std::map<int, bool>::iterator lIter = lBreakLines.find(i);
      if (lIter != lBreakLines.end()) {
        lOut << (lIter->second ? "o" : "x");
      } else {
        lOut << " ";
      }
      lOut << (lCurrentLine == i ? ">" : "|") << " ";
    }
    lOut << lFileContent.at(i - 1);
    if (i != aEndLine) {
      lOut << std::endl;
    }
  }

  return lOut.str();
}


void
DebuggerRuntime::setLastContinuationCommand(int aTransactionID, std::string aCommandName)
{
  theLastContinuationCommand = std::pair<int, std::string>(aTransactionID, aCommandName);
}

DebuggerRuntime*
DebuggerRuntime::clone()
{
  DebuggerRuntime* lNewRuntime = new DebuggerRuntime(
    theQuery,
    theOStream,
    theSerializerOptions,
    theCommunicator,
    theItemHandler,
    theCallbackData,
    theInterruptBreak);

  lNewRuntime->theBreakpoints = theBreakpoints;
  return lNewRuntime;
}


// ****************************************************************************
// Private functions

DebuggerCommons*
DebuggerRuntime::getDebbugerCommons()
{
  return theWrapper->thePlanState->theDebuggerCommons;
}

} // namespace zorba
