#include <zorbatypes/collation_manager.h>
#include "zorbatypes/representations.h"
#include "context/collation_cache.h"
#include "context/static_context.h"

namespace zorba {

  CollationCache::CollationCache(static_context* aStaticContext)
  : theStaticContext(aStaticContext),
    theDefaultCollator(0)
  {}

  CollationCache::~CollationCache() 
  {
    {
      std::map<std::string, XQPCollator*>::iterator lIter = theMap.begin();
      std::map<std::string, XQPCollator*>::iterator lEnd = theMap.end();
      for ( ; lIter != lEnd ; ++lIter)
      {
        delete lIter->second;
      }
    }
    theMap.clear();
    if (theDefaultCollator != 0)
    {
      delete theDefaultCollator;
      theDefaultCollator = 0;
    }
  }

  XQPCollator*
  CollationCache::getCollator(const std::string& aName)
  {
    xqp_string lURI;
    if (!theStaticContext->get_collation_uri(aName, lURI))
    {
      return 0;
    } else {
      CacheMap_t::iterator lFind = theMap.find(lURI);
      if ( lFind != theMap.end() )
      {
        return lFind->second;
      }
      else
      {
        XQPCollator* lCollator = CollationFactory::createCollator(lURI);
        theMap[lURI] = lCollator;
        return lCollator;
      } 
    }
  }

  XQPCollator*
  CollationCache::getDefaultCollator()
  {
    if (theDefaultCollator == 0)
    {
      const xqp_string& lDefaultURI = theStaticContext->default_collation_uri();
      theDefaultCollator = CollationFactory::createCollator(lDefaultURI);
    }
    return theDefaultCollator; 
  }
}
