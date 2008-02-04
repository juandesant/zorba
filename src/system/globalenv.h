#ifndef XQP_GLOBALENV_H
#define XQP_GLOBALENV_H

#include <memory>
#include "mapm/m_apm.h"

namespace xqp {

class TypeSystem;

class GlobalEnvironment {
  public:
    static void init();
    static void destroy();
    static GlobalEnvironment& getInstance();

    TypeSystem& getTypeSystem();
  private:
    GlobalEnvironment();

    std::auto_ptr<TypeSystem> m_typesystem;
    M_APM                     m_mapm; // this is a pointer type

    static GlobalEnvironment *m_globalEnv;
};

#define GENV_TYPESYSTEM GlobalEnvironment::getInstance().getTypeSystem()

}

#endif /* XQP_GLOBALENV_H */
/* vim:set ts=2 sw=2: */
