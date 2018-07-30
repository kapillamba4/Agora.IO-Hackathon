#include "jni.h"
#include "string"
using namespace std;
#define CHECK_PTR_RETURN(PTR) \
  {            \
    if(!PTR)   \
      return;  \
  }
#define CHECK_PTR_RETURN_BOOL(PTR) \
  {            \
    if(!PTR)   \
      return false;  \
  }
#define CHECK_EXCEPTION(jni, message)                              \
  if (jni->ExceptionCheck()) {                                     \
    jni->ExceptionDescribe();                                      \
    jni->ExceptionClear();                                         \
  }

#define PRINTDETAIL (printf("ERROR:%s (%d) - <%s>\n",__FILE__,__LINE__,__FUNCTION__),printf)

#define CP(PTR)            \
  {                        \
    if(!PTR){              \
      PRINTDETAIL;         \
      return;              \
    }                      \
  }                        \

#define CPB(PTR)           \
  {                        \
    if(!PTR){              \
      PRINTDETAIL;         \
      return false;        \
    }                      \
  }                        \

#define CPN(PTR)           \
  {                        \
    if(!PTR){              \
      PRINTDETAIL;         \
      return NULL;         \
    }                      \
  }                        \
