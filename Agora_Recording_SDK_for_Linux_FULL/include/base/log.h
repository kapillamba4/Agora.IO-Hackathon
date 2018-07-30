#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <stdarg.h>
#include <cassert>
#include <syslog.h>
#include "base/atomic.h"
#include "base/mutexer.h"

namespace agora {
namespace base {

enum log_levels {
    DEBUG_LOG  = LOG_DEBUG,    /* 7 debug-level messages */
    INFO_LOG  = LOG_INFO,     /* 6 informational */
    NOTICE_LOG  = LOG_NOTICE,   /* 5 normal but significant condition */
    WARN_LOG  = LOG_WARNING,  /* 4 warning conditions */
    ERROR_LOG  = LOG_ERR,      /* 3 error conditions */
    FATAL_LOG  = LOG_CRIT,     /* 2 critical conditions */
};

enum log_facility {
    CRON_LOG_FCLT = LOG_CRON,
    DAEMON_LOG_FCLT = LOG_DAEMON,
    FTP_LOG_FCLT = LOG_FTP,
    NEWS_LOG_FCLT = LOG_NEWS,
    AUTH_LOG_FCLT = LOG_AUTH,		/* DEPRECATED */
    SYSLOG_LOG_FCLT = LOG_SYSLOG,
    USER_LOG_FCLT = LOG_USER,
    UUCP_LOG_FCLT = LOG_UUCP,

    LOCAL0_LOG_FCLT = LOG_LOCAL0,
    LOCAL1_LOG_FCLT = LOG_LOCAL1,
    LOCAL2_LOG_FCLT = LOG_LOCAL2,
    LOCAL3_LOG_FCLT = LOG_LOCAL3,
    LOCAL4_LOG_FCLT = LOG_LOCAL4,
    LOCAL5_LOG_FCLT = LOG_LOCAL5,
    LOCAL6_LOG_FCLT = LOG_LOCAL6,
    LOCAL7_LOG_FCLT = LOG_LOCAL7,
    
};

#define LOG_FACILITY_MASK  LOG_FACMASK 

class log_config {
public:
    static inline void enable_debug(bool enabled) {
        if (enabled) {
            log_config::enabled_level = DEBUG_LOG;
        } else {
            log_config::enabled_level = INFO_LOG;
        }
    }

    static bool set_drop_cannel(uint32_t cancel) {
        if (cancel > DROP_COUNT) {
            drop_cancel = DROP_COUNT;
            return false;
        }

        drop_cancel = cancel;
        return true;
    }

    static inline bool log_enabled(log_levels level) {
        if (level <= enabled_level) {
            return true;
        } 
        else
            return false;// skip overflow to enable debug info ability


        ++dropped_count;
        return (dropped_count % DROP_COUNT < drop_cancel);
    }

    /**
     *  get log_config current facility 
     *
     *  @return uint32: get log_config current facility .
     */
    static uint32_t getFacility() { return log_config::facility; }

    /**
     * change log_config Facility per your specific purpose like agora::base::LOCAL5_LOG_FCLT
     * Default:USER_LOG_FCLT. 
     * eg,
     * agora::base::log_config::setFacility(agora::base::USER_LOG_FCLT);
     *
     *  @param fac facility setting
     */
    static void setFacility(uint32_t fac) { log_config::facility = fac & LOG_FACILITY_MASK; }

    static inline void lock(){ logger_mutex.lock(); }
    static inline void unlock(){ logger_mutex.unlock(); }
    static inline bool trylock(){ return logger_mutex.trylock(); }

private:
    static Mutexer logger_mutex;
    static int enabled_level;
    static uint64_t dropped_count;

    static uint32_t drop_cancel;
    const static uint32_t DROP_COUNT = 1000;
    static uint32_t facility;
};



void open_log();
inline void close_log() {
    ::closelog();
}
void log_dir(const char* logdir, log_levels level, const char* format, ...);
void log(log_levels level, const char* format, ...);


}
}

#define LOG(level, fmt, ...) log(agora::base::level ## _LOG, \
        "(%d) %s:%d: " fmt, getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_DIR(logdir, level, fmt, ...) log_dir(logdir, agora::base::level ## _LOG, \
        "(%d) %s:%d: " fmt, getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_DIR_IF(logdir, level, cond, fmt, ...) \
    if (cond) { \
        log_dir(logdir, agora::base::level ## _LOG,  \
                "(%d) %s:%d: " fmt, getpid(), __FILE__, __LINE__, ##__VA_ARGS__); \
    }

#define LOG_IF(level, cond, ...) \
    if (cond) { \
        LOG(level,  __VA_ARGS__); \
    }

#define LOG_EVERY_N(level, N, ...) \
{  \
    static unsigned int count = 0; \
    if (++count % N == 0) \
    LOG(level, __VA_ARGS__); \
}



//#define DEBUG_MODE

#ifdef DEBUG_MODE

#define SPLIT_ARR 50
#define DEBUG_LOG_RAW_DATA(logdir, level, STR, msg, size, ...) do { \
    uint32_t i;    \
    std::string output(#STR);    \
    output.reserve(size*3);    \
    output.append(" "); \
    for (uint32_t i = 0; i < size/SPLIT_ARR+1; i++) {    \
        uint32_t end = size > (i+1) * SPLIT_ARR?  SPLIT_ARR:(size - (i * SPLIT_ARR)); \
        for(uint32_t j = 0;j < end;j++)    \
        {    \
            uint32_t index = i * SPLIT_ARR + j;    \
            output.append(std::to_string(msg[index])+"_"); \
        }    \
        output.append("@"); \
    }    \
    output.append("\n"); \
    log_dir(logdir, agora::base::level ## _LOG, \
            "(%d) %s:%d: %s" , getpid(), __FILE__, __LINE__,output.c_str(), ##__VA_ARGS__); \
}while(0)


#define DEBUG_RAW_DATA(STR, msg, size) do {  \
    unsigned int i;  \
    printf("%s", #STR); \
    for (i = 0; i < size; i++) {  \
        if(i % 50 == 0)  \
            printf("\n");\
        else \
            printf("%x", msg[i]); \
    }    \
     \
}while(0)
#else
#define DEBUG_RAW_DATA(STR, msg, size)
#define DEBUG_LOG_RAW_DATA(logdir, level, STR, msg, size, ...)
#endif

