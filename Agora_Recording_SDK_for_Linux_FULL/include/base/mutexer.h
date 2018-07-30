#ifndef __MUTEXER_H__
#define __MUTEXER_H__

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace agora {
namespace base {

class Mutexer
{
 public:
  Mutexer();

  virtual ~Mutexer();

  virtual void lock();
  virtual void unlock();
  virtual bool trylock();

 private:
#ifdef _WIN32
  CRITICAL_SECTION crit;
#else
  pthread_mutex_t mutex_;
#endif
};

}
}

#endif//__MUTEXER_H__
