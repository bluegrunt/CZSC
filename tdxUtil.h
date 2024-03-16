#ifndef TDXUTIL_H
#define TDXUTIL_H
// #include "CZSC.h"
#include <ctime>
#include <time.h>
#include <vector>
#include <string>
// Tdx types
typedef int tdxday_t;  // tdx日期 int ，如 20231211

// tdx日期数据格式
struct TdxDayBin {
  int date;
  int open;
  int high;
  int low;
  int close;
  float amt;
  int vol;
  int unkown;
};

// 应用的格式
struct ohlc_t {
  time_t time ;
  float open;
  float high;
  float low;
  float close;
  float amt;
  float vol;
  // int unkown;
};


// 将自然日期的 struct tm 转成 内部用的 struct tm
struct std::tm convert_timetm_input(const struct std::tm& outer_day);

//  将内部日期转为外部部格式
struct std::tm convert_timetm_output(const struct std::tm& inner_day);


// 对于给定的时间t 得到该周最后的一个时间
time_t getLastTimeOfWeek(const time_t& t);

time_t getLastTimeOfMonth(const time_t& t);

time_t getLastTimeOfQuarter(const time_t& t);

// 对于给定的时间t 得到该年最后的一个时间
time_t getLastTimeOfYear(const time_t& t);


//  
time_t TdxDay2time(tdxday_t tdxday) ;

// 将tdx日期整形转成time_t
// @parameter tdx日期形如 20231210
// @return vector of ohlc_t
std::vector<ohlc_t> readDayBin(std::string& dayFile);


// convert day ohlc to week ohlc
std::vector<ohlc_t> convert2week(const std::vector<ohlc_t>& vecDay);
// convert day ohlc to month ohlc
std::vector<ohlc_t> convert2month(const std::vector<ohlc_t>& vecDay);
// convert day ohlc to quarter ohlc
std::vector<ohlc_t> convert2quarter(const std::vector<ohlc_t>& vecDay);
// convert day ohlc to year ohlc
std::vector<ohlc_t> convert2year(const std::vector<ohlc_t>& vecDay);

std::string time2str(const time_t& t);
std::string time2str(const time_t& t,const std::string& fmt);
time_t str2time(const std::string& str, const std::string& fmt);

std::vector<std::string> split(const std::string& input, const char& delimiter);
std::vector<ohlc_t> readTextFile(const std::string& f, 
                                 const std::string& fmt = std::string("%Y/%m/%d"), 
                                 const char delimiter = '\t', 
                                 const int skip = 0);
std::string strReplace(
    const std::string& source,
    const std::string& pattern,
    const std::string& substitute,
    unsigned int limit = UINT_MAX);

#endif // !TDXUTIL_H

