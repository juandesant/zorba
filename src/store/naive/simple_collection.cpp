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

#include "zorbaerrors/Assert.h"
#include "zorbaerrors/error_manager.h"
#include "system/globalenv.h"
#include "store/naive/simple_collection.h"
#include "store/naive/sax_loader.h"
#include "store/naive/simple_store.h"
#include "store/naive/store_defs.h"
#include "store/naive/node_items.h"

namespace zorba { namespace simplestore {

/*******************************************************************************

********************************************************************************/
SimpleCollection::SimpleCollection(store::Item_t& uri)
{
  theUri.transfer(uri);
}


/*******************************************************************************

********************************************************************************/
SimpleCollection::~SimpleCollection()
{
}


/*******************************************************************************
  Return an iterator over the nodes of this collection.

  Note: it is allowed to have several concurrent iterators on the same
        collection.

  The "idsNeeded" param tell whether the returned nodes should contain node
  ids (e.g. for document-order sorting) or not. idsNeeded == false is likely
  to be faster. 'idsNeeded' should only be set to true if clients wants to
  sort or compare the items or sub-items generated from the returned iterator.
********************************************************************************/
store::Iterator_t SimpleCollection::getIterator(bool idsNeeded)
{
  return new CollectionIter(this);
}


/*******************************************************************************
  Insert into the collection an xml document or fragment given as text via an
  input stream. Return the root node of the new xml document or fragment.
********************************************************************************/
store::Item_t SimpleCollection::loadDocument(
    std::istream& stream,
    long position)
{
  error::ErrorManager lErrorManager;
  std::auto_ptr<XmlLoader> loader(GET_STORE().getXmlLoader(&lErrorManager));
  xqpStringStore_t docUri;

  store::Item_t root = loader->loadXml(docUri, stream);

  if (lErrorManager.hasErrors()) 
  {
    throw lErrorManager.getErrors().front();
  }

  if (root != NULL)
  {
    SYNC_CODE(AutoLatch lock(theLatch, Latch::WRITE);)

    if(position == -1)
      theXmlTrees.push_back(root);
    else
      theXmlTrees.insert(theXmlTrees.begin() + (position - 1), root);

    BASE_NODE(root)->setCollection(this);
  }

  return root;
}


store::Item_t SimpleCollection::loadDocument(
    std::istream* stream,
    long position)
{
  store::Item_t    docitem;
  std::auto_ptr<std::istream> theStream(stream); // we take the ownership
  //do full loading for now
  docitem = loadDocument(*stream, position);
  return docitem;
}


/*******************************************************************************
  Insert the given node to the collection. If the node is in the collection
  already, this method raises an error.
********************************************************************************/
void SimpleCollection::addNode(
    const store::Item* node,
    long position)
{
  if (!node->isNode())
  {
    ZORBA_ERROR( API0007_COLLECTION_ITEM_MUST_BE_A_NODE);
  }

  SYNC_CODE(AutoLatch lock(theLatch, Latch::WRITE);)

  if(nodePositionInCollection((store::Item*)node) == -1)
  {
    if(position <= 0)
      theXmlTrees.push_back(const_cast<store::Item*>(node));
    else
      theXmlTrees.insert(theXmlTrees.begin() + (position - 1),
                         const_cast<store::Item*>(node));
  }
  else
  {
    ZORBA_ERROR(API0031_NODE_ALREADY_IN_COLLECTION);
  }
}


/*******************************************************************************
  Insert the given node to the collection before or after the aTargetNode. 
********************************************************************************/
void SimpleCollection::addNode(
    const store::Item* node,
    const store::Item* aTargetNode,
    bool before)
{
  if (!node->isNode() || !aTargetNode->isNode())
  {
    ZORBA_ERROR( API0007_COLLECTION_ITEM_MUST_BE_A_NODE);
  }

  SYNC_CODE(AutoLatch lock(theLatch, Latch::WRITE);)

  long targetPos = nodePositionInCollection((store::Item*)aTargetNode);
  if(targetPos == -1)
  {
    ZORBA_ERROR(API0029_NODE_DOES_NOT_BELONG_TO_COLLECTION);
  }

  if(before) 
  {
    ZORBA_ASSERT(targetPos != 0);
    theXmlTrees.insert(theXmlTrees.begin() + (targetPos-1),
                       const_cast<store::Item*>(node));
  }
  else
  {
    theXmlTrees.insert(theXmlTrees.begin() + targetPos,
                       const_cast<store::Item*>(node));
  }
}


/*******************************************************************************
  Insert into the collection the set of nodes returned by the given iterator.
********************************************************************************/
void SimpleCollection::addNodes(
    store::Iterator* nodeIter,
    long position)
{
  store::Item_t node;
  while (nodeIter->next(node))
  {
    addNode(node, position);
  }
}


/*******************************************************************************
  Delete the given node from the collection.
********************************************************************************/
void SimpleCollection::removeNode(const store::Item* node)
{
  if (!node->isNode())
  {
    ZORBA_ERROR( API0007_COLLECTION_ITEM_MUST_BE_A_NODE);
  }

  SYNC_CODE(AutoLatch lock(theLatch, Latch::WRITE);)

  long position = nodePositionInCollection((store::Item*) node);

  if(position != -1)
  {
    if( (ulong)(position-1) >= theXmlTrees.size() )
      ZORBA_ERROR(API0030_NO_NODE_AT_GIVEN_POSITION);

    ZORBA_ASSERT(position != 0);

    theXmlTrees.erase(theXmlTrees.begin() + (position - 1));
  }
  else
  {
    ZORBA_ERROR(API0029_NODE_DOES_NOT_BELONG_TO_COLLECTION);
  }
}


/*******************************************************************************
  Delete the node at the given position from the collection.
********************************************************************************/
void SimpleCollection::removeNode(long position)
{
  SYNC_CODE(AutoLatch lock(theLatch, Latch::WRITE);)

  if (position == -1) 
  {
    if (theXmlTrees.size() == 0)
      ZORBA_ERROR(API0030_NO_NODE_AT_GIVEN_POSITION);

    theXmlTrees.erase(theXmlTrees.end() - 1);
  }
  else if (position <= 0)
  {
    ZORBA_ERROR_DESC(API0030_NO_NODE_AT_GIVEN_POSITION,
        "The given position is not valid. It must be >= -1.");
  }
  else 
  {
    if ((unsigned) position > theXmlTrees.size())
      ZORBA_ERROR(API0030_NO_NODE_AT_GIVEN_POSITION);

    theXmlTrees.erase(theXmlTrees.begin() + (position - 1));
  }
}


/*******************************************************************************
  Return the node at the given position from the collection.
********************************************************************************/
store::Item_t SimpleCollection::nodeAt(long position)
{
  if( position <= 0 || ((ulong)position) > theXmlTrees.size() )
  {
    ZORBA_ERROR(API0030_NO_NODE_AT_GIVEN_POSITION);
  }

  return theXmlTrees[position  - 1];
}


/*******************************************************************************
  Return position of the given node in the collection.
********************************************************************************/
long SimpleCollection::indexOf(const store::Item* node)
{
  if (!node->isNode())
  {
    ZORBA_ERROR(API0007_COLLECTION_ITEM_MUST_BE_A_NODE);
  }

  return nodePositionInCollection((store::Item*) node);
}


/*******************************************************************************

********************************************************************************/
int SimpleCollection::nodePositionInCollection(store::Item* newNode)
{
  if( theXmlTrees.empty() )
    return -1;

  checked_vector<store::Item_t>::iterator it = theXmlTrees.begin();
  checked_vector<store::Item_t>::iterator end = theXmlTrees.end();
  const XmlNode* rNewNode = reinterpret_cast<const XmlNode*>(newNode);

  for (; it != end; ++it)
  {
    //check if the nodes are the same
    if(rNewNode->equals(reinterpret_cast<XmlNode*>(it->getp())))
      return (it - theXmlTrees.begin() + 1);
  }

  return -1;
}


/*******************************************************************************

********************************************************************************/
SimpleCollection::CollectionIter::CollectionIter(
    SimpleCollection* collection)
  :
  theCollection(collection),
  theHaveLock(false)
{
}


/*******************************************************************************

********************************************************************************/
SimpleCollection::CollectionIter::~CollectionIter() 
{
  SYNC_CODE(if (theHaveLock) \
    theCollection->theLatch.unlock();)
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::CollectionIter::open()
{
  SYNC_CODE(theCollection->theLatch.rlock();)
  theHaveLock = true;

  theIterator = theCollection->theXmlTrees.begin();
}


/*******************************************************************************

********************************************************************************/
bool SimpleCollection::CollectionIter::next(store::Item_t& result)
{
  if (!theHaveLock) 
  {
    ZORBA_ERROR_DESC(XQP0000_DYNAMIC_RUNTIME_ERROR,
                     "Collection iterator has not been opened");
  }

  if (theIterator == theCollection->theXmlTrees.end()) 
  {
    result = NULL;
    return false;
  }

  result = (*theIterator).getp();
  ++theIterator;

  return true;
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::CollectionIter::reset()
{
  theIterator = theCollection->theXmlTrees.begin();
}


/*******************************************************************************

********************************************************************************/
void SimpleCollection::CollectionIter::close() 
{
  assert(theHaveLock);
  theHaveLock = false;
  SYNC_CODE(theCollection->theLatch.unlock();)
}

} // namespace simplestore
} // namespace zorba

