#include "functions/Misc.h"

#include "system/globalenv.h"
#include "errors/error_manager.h"
#include "runtime/debug/debug_iterators.h"
#include "runtime/misc/MiscImpl.h"

using namespace std;

namespace zorba {

fn_trace_func::fn_trace_func(const signature& sig)
  : function(sig) { }

PlanIter_t fn_trace_func::codegen (const QueryLoc& loc, std::vector<PlanIter_t>& argv, AnnotationHolder &ann) const
{
  if ( !validate_args ( argv ) )
	{
    ZORBA_ERROR_LOC( ZorbaError::XPST0017, loc);
  }
  return new FnTraceIterator ( loc, argv );
}

xqtref_t fn_trace_func::type_check(signature& sig) const
{
  return GENV_TYPESYSTEM.ITEM_TYPE_STAR;
}

bool fn_trace_func::validate_args(std::vector<PlanIter_t>& argv) const
{
  return argv.size() == 2;
}

/*******************************************************************************
  3 The Error Function
********************************************************************************/
fn_error_func_1::fn_error_func_1(const signature& sig) : function (sig)
{
}

PlanIter_t fn_error_func_1::codegen (const QueryLoc& loc, std::vector<PlanIter_t>& argv, AnnotationHolder &ann) const
{
  if (!validate_args(argv))
  {
    ZORBA_ERROR_LOC( ZorbaError::XPST0017, loc);
  }

  return new FnErrorIterator(loc, argv);
}

xqtref_t fn_error_func_1::type_check(signature&) const
{
  return GENV_TYPESYSTEM.QNAME_TYPE_ONE;
}

bool fn_error_func_1::validate_args(vector<PlanIter_t>& argv) const
{
  return (argv.size() == 1);
}

fn_error_func_2::fn_error_func_2(const signature& sig) : function (sig)
{
}

PlanIter_t fn_error_func_2::codegen (const QueryLoc& loc, std::vector<PlanIter_t>& argv, AnnotationHolder &ann) const
{
  if (!validate_args(argv))
  {
    ZORBA_ERROR_LOC( ZorbaError::XPST0017, loc);
  }

  return new FnErrorIterator(loc, argv);
}

xqtref_t fn_error_func_2::type_check(signature&) const
{
  return GENV_TYPESYSTEM.QNAME_TYPE_ONE;
}

bool fn_error_func_2::validate_args(vector<PlanIter_t>& argv) const
{
  return (argv.size() == 2);
}

fn_error_func_3::fn_error_func_3(const signature& sig) : function (sig)
{
}

PlanIter_t fn_error_func_3::codegen (const QueryLoc& loc, std::vector<PlanIter_t>& argv, AnnotationHolder &ann) const
{
  if (!validate_args(argv))
  {
    ZORBA_ERROR_LOC( ZorbaError::XPST0017, loc);
  }

  return new FnErrorIterator(loc, argv);
}

xqtref_t fn_error_func_3::type_check(signature&) const
{
  return GENV_TYPESYSTEM.QNAME_TYPE_ONE;
}

bool fn_error_func_3::validate_args(vector<PlanIter_t>& argv) const
{
  return (argv.size() == 3);
}

}
/* vim:set ts=2 sw=2: */
