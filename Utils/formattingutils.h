#ifndef FORMATTINGUTILS_H
#define FORMATTINGUTILS_H

#include <boost/date_time/posix_time/ptime.hpp>
#include <sstream>
namespace FormattingUtils
{
std::string dateToStr(const boost::posix_time::ptime &time);
std::string dateToTimeStr(const boost::posix_time::ptime &time);
std::string dateToDayStr(const boost::posix_time::ptime &time);

boost::posix_time::ptime parseTime(const std::string& timeAsStr);
boost::posix_time::ptime time_tToPtime(const time_t& time);
};

#endif // FORMATTINGUTILS_H
