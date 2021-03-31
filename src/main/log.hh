#ifndef LOG_HH
#define LOG_HH

#include <boost/log/trivial.hpp>

void logInit();

#define Log BOOST_LOG_TRIVIAL

#endif /* LOG_HH */
