#ifndef RA_LOG_H
#define RA_LOG_H

#include <iostream>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

#define RA_ROOT_LOG_SETLEVEL(level) log4cplus::Logger::getRoot().setLogLevel(level ## _LOG_LEVEL)
#define RA_LOG_CONFIG(filename) log4cplus::PropertyConfigurator::doConfigure(filename)
#define RA_LOG_CONFIG_WITH_CONTENT(istream)                 \
    do {                                                    \
        log4cplus::PropertyConfigurator config(istream);    \
        config.configure();                                 \
    } while (0)

#define RA_LOG_DECLARE() static log4cplus::Logger _logger
#define RA_LOG_SETUP(n,c) log4cplus::Logger c::_logger = log4cplus::Logger::getInstance("ra." #n "."  #c)
#define RA_LOG_SETUP_TEMPLATE(n,c, T)  \
    template <typename T>              \
    log4cplus::Logger c<T>::_logger = log4cplus::Logger::getInstance("ra." #n "."  #c)

#define RA_LOG(level, format, args...) LOG4CPLUS_MACRO_FMT_BODY(_logger, level ## _LOG_LEVEL, format, ##args)

#endif //RA_LOG_H
