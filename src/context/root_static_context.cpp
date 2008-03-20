#include "types/root_typemanager.h"
#include "context/root_static_context.h"

namespace zorba {

static const char *default_ns_initializers [] = {
  "fn", XQUERY_FN_NS,
  "xml", "http://www.w3.org/XML/1998/namespace",
  "xs", "http://www.w3.org/2001/XMLSchema",
  "xsi", "http://www.w3.org/2001/XMLSchema-instance",
  "local", "http://www.w3.org/2005/xquery-local-functions",
  NULL, NULL
};

root_static_context::root_static_context()
    : static_context()
{
  typemgr.reset(new RootTypeManager());
  set_xpath1_0compatib_mode(StaticContextConsts::xpath2_0);
  const char **p = default_ns_initializers;
  for (; *p != NULL; p += 2)
    bind_ns (p [0], p [1]);
  set_default_elem_type_ns ("");		
  set_default_function_namespace (lookup_ns ("fn"));
  set_context_item_static_type(typemgr->create_any_type());
  set_default_collation_uri ("http://www.flworfound.org/collations/IDENTICAL/en/US");
  set_construction_mode(StaticContextConsts::cons_strip);
  set_ordering_mode(StaticContextConsts::ordered);
  set_order_empty_mode(StaticContextConsts::empty_greatest);
  set_boundary_space_mode(StaticContextConsts::strip_space);
  set_inherit_mode(StaticContextConsts::no_inherit_ns);
  set_preserve_mode(StaticContextConsts::no_preserve_ns);
  set_default_collection_type(typemgr->create_any_type());

  set_current_absolute_baseuri("");
}

root_static_context::~root_static_context()
{
}

}
/* vim:set ts=2 sw=2: */
