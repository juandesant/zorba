
#ifndef XQP_SIMPLE_LOADER_H
#define XQP_SIMPLE_LOADER_H

#include <stack>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include "store/api/item.h"

namespace xqp
{

template <class Object> class rchandle;

class Item;
class NodeItem;
typedef rchandle<Item> Item_t;
typedef rchandle<class NodeItem> NodeItem_t;


class XmlLoader
{
protected:
  xmlSAXHandler       theSaxHandler;
 
  Item_t              theRootNode;
  std::stack<Item_t>  thePath;

  xqpStringStore      theErrors;

public:
  static void	startDocumentSAXFunc(void * ctx);

  static void endDocumentSAXFunc(void * ctx);

  static void startElementNs(
        void * ctx, 
        const xmlChar * localname, 
        const xmlChar * prefix, 
        const xmlChar * URI, 
        int nb_namespaces, 
        const xmlChar ** namespaces, 
        int nb_attributes, 
        int nb_defaulted, 
        const xmlChar ** attributes);
  
  static void endElementNs(
        void * ctx, 
        const xmlChar * localname, 
        const xmlChar * prefix, 
        const xmlChar * URI);

  static void charactersSAXFunc(
        void * ctx,
        const xmlChar * ch,
        int len);

  static void	cdataBlockSAXFunc(
        void * ctx, 
        const xmlChar * value, 
        int len);

  static void commentSAXFunc(
        void * ctx, 
        const xmlChar * value);

  static void	processingInstructionSAXFunc(
        void * ctx, 
        const xmlChar * target, 
        const xmlChar * data);

  static void error(void * ctx, const char * msg, ... );

  static void warning(void * ctx, const char * msg, ... );

public:
  XmlLoader();

  ~XmlLoader();

  Item_t getRootNode() const { return theRootNode; }
  
  Item_t loadXml(std::iostream& xmlStream);

protected:
  long readPacket(std::iostream& stream, char* buf, long size);
};

} /* namespace xqp */

#endif /* XQP_SIMPLE_LOADER_H */


