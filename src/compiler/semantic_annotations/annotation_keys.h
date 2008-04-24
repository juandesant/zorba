#ifndef ZORBA_ANNOTATION_KEYS_H
#define ZORBA_ANNOTATION_KEYS_H

namespace zorba {

namespace AnnotationKey {

enum {
  IGNORES_SORTED_NODES,
  IGNORES_DUP_NODES,
  PRODUCES_SORTED_NODES,
  PRODUCES_DISTINCT_NODES,
  
  FREE_VARS,

  LET_VAR_NODEID_ANALYSIS,

  EXPENSIVE_OP,
  UNFOLDABLE_OP,
  IMPURE_EXPR,
  
  MAX_ANNOTATION
};

}

}

#endif  // ZORBA_ANNOTATION_KEYS_H

/*
 * Local variables:
 * mode: c++
 * End:
 */
