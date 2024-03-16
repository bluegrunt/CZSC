#include <ctime>
#include <time.h>
#include <vector>
#include "tdxUtil.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <iomanip>

struct std::tm convert_timetm_input(const struct std::tm& outer_day){
  // 将外部日期转为内部格式
  struct std::tm newd(outer_day);
  newd.tm_mon -= 1;//
  newd.tm_year -= 1900;//
  return newd;
}

struct std::tm convert_timetm_output(const struct std::tm& inner_day){
  // 将内部日期转为外部部格式
  struct std::tm newd(inner_day);
  newd.tm_mon += 1;//
  newd.tm_year += 1900;//
  return newd;

}

time_t getLastTimeOfWeek(const time_t& t){
  /*
   *对于给定的时间t 得到该周最后的一个时间
   * */
  struct std::tm tm1;
  localtime_s(&tm1, &t);
  int diff = 6 - tm1.tm_wday; // 
  tm1.tm_mday += diff;
  tm1.tm_hour = 23;
  tm1.tm_min = 59;
  tm1.tm_sec = 59;
  return mktime(&tm1);
  
}


time_t getLastTimeOfMonth(const time_t& t){
  /*
   *对于给定的时间t 得到该月最后的一个时间
   * */
  struct std::tm tm1;
  localtime_s(&tm1, &t);
  tm1.tm_mon += 1;
  tm1.tm_mday = 0;
  tm1.tm_hour = 23;
  tm1.tm_min = 59;
  tm1.tm_sec = 59;
  return mktime(&tm1);
}

time_t getLastTimeOfQuarter(const time_t& t){
  /*
   *对于给定的时间t 得到该季度最后的一个时间
   * */
  struct std::tm tm1;
  localtime_s(&tm1, &t);
  switch (tm1.tm_mon) {
    case 0:  
    case 1:  
    case 2:
      tm1.tm_mon = 2;
      tm1.tm_mday = 31;
      break;
    case 3:  
    case 4:  
    case 5:
      tm1.tm_mon = 5;
      tm1.tm_mday = 30;
break;
    case 6:  
    case 7:  
    case 8:
      tm1.tm_mon = 8;
      tm1.tm_mday = 30;
      break;
    case 9:  
    case 10:  
    case 11:
      tm1.tm_mon = 11;
      tm1.tm_mday = 31;
      break;
  }
  tm1.tm_hour = 23;
  tm1.tm_min = 59;
  tm1.tm_sec = 59;
  return mktime(&tm1);
}


time_t getLastTimeOfYear(const time_t& t){
  /*
   *对于给定的时间t 得到该年最后的一个时间
   * */
  struct std::tm tm1;
  localtime_s(&tm1, &t);
  tm1.tm_mon = 11;
  tm1.tm_mday = 31;
  tm1.tm_hour = 23;
  tm1.tm_min = 59;
  tm1.tm_sec = 59;
  return mktime(&tm1);
}



time_t TdxDay2time(tdxday_t tdxday) {
  /*将tdx日期整形转成time_t
   * tdx日期形如 20231210
   * */
  time_t t;
  struct std::tm tm_day;
  tm_day.tm_year = int(tdxday / 10000);
  int temp = tdxday % 10000;
  tm_day.tm_mon = int(temp/100);
  tm_day.tm_mday = temp % 100 ;
  tm_day.tm_hour = 23;
  tm_day.tm_min = 59;
  tm_day.tm_sec = 59;
  struct std::tm inner_day = convert_timetm_input(tm_day);
  t = mktime(&inner_day);
  return t;
}


std::vector<ohlc_t> readDayBin(std::string& dayFile){
  /*read tdx day binary file 
   * return vector of ohlc
   * */
  std::ifstream file1;
  std::vector<ohlc_t> vec1;
  file1.open(dayFile, std::ios::in | std::ios::binary);
  if (file1) {
    ohlc_t day;
    TdxDayBin day1;
    while (file1.read(reinterpret_cast<char *>(&day1), sizeof(day1))) {
      day.time = TdxDay2time(day1.date) ;
      day.open = day1.open / 100.0;
      day.high = day1.high / 100.0;
      day.low = day1.low / 100.0;
      day.close = day1.close / 100.0;
      day.amt = day1.amt;
      day.vol = float(day1.vol);
      vec1.push_back(day);
    }
  } else {
    std::cout << "Can not open file " << dayFile << std::endl;
  }
  return vec1;
}

std::vector<ohlc_t> convert2week(const std::vector<ohlc_t>& vecDay){
  // convert day ohlc to week ohlc
  std::vector<ohlc_t> vect1;
  std::vector<ohlc_t>::const_iterator ptr_curr;
  ohlc_t item;
  bool waitingForCollect = false;
  time_t lastTimeOfWeek ;
  for (ptr_curr=vecDay.begin(); ; ++ptr_curr) {
    if (ptr_curr == vecDay.end()){
      if (waitingForCollect ) {
        vect1.emplace_back(item);
      }
      break;
    }
    if(ptr_curr==vecDay.begin()) { 
      // 首次进入
      lastTimeOfWeek = getLastTimeOfWeek(ptr_curr->time); 
      item.time = ptr_curr->time;
      item.open = ptr_curr->open;
      item.high = ptr_curr->high;
      item.low = ptr_curr->low;
      item.close = ptr_curr->close;
      item.amt = ptr_curr->amt;
      item.vol = ptr_curr->vol;
      waitingForCollect = true;
    }else{
      if(ptr_curr->time <= lastTimeOfWeek){
        //修改 ohlc
        item.time = ptr_curr->time;
        if (ptr_curr->high > item.high) item.high = ptr_curr->high;
        if (ptr_curr->low  < item.low ) item.low  = ptr_curr->low ;
        item.close = ptr_curr->close;
        item.amt += ptr_curr->amt;
        item.vol += ptr_curr->vol;
        waitingForCollect = true;
      }else{ // 旧的保存， 一个新的产生。
        vect1.emplace_back(item);
        waitingForCollect = false;

        lastTimeOfWeek = getLastTimeOfWeek(ptr_curr->time); 
        item.time = ptr_curr->time;
        item.open = ptr_curr->open;
        item.high = ptr_curr->high;
        item.low = ptr_curr->low;
        item.close = ptr_curr->close;
        item.amt = ptr_curr->amt;
        item.vol = ptr_curr->vol;
        waitingForCollect = true;
        
      }

    }
  }
  return vect1; 
}

std::vector<ohlc_t> convert2month(const std::vector<ohlc_t>& vecDay){
  // convert day ohlc to month ohlc
  std::vector<ohlc_t> vect1;
  std::vector<ohlc_t>::const_iterator ptr_curr;
  ohlc_t item;
  bool waitingForCollect = false;
  time_t lastTimeOfMonth ;
  for (ptr_curr=vecDay.begin(); ; ++ptr_curr) {
    if (ptr_curr == vecDay.end()){
      if (waitingForCollect ) {
        vect1.emplace_back(item);
      }
      break;
    }
    if(ptr_curr==vecDay.begin()) { 
      // 首次进入
      lastTimeOfMonth = getLastTimeOfMonth(ptr_curr->time); 
      item.time = ptr_curr->time;
      item.open = ptr_curr->open;
      item.high = ptr_curr->high;
      item.low = ptr_curr->low;
      item.close = ptr_curr->close;
      item.amt = ptr_curr->amt;
      item.vol = ptr_curr->vol;
      waitingForCollect = true;
    }else{
      if(ptr_curr->time <= lastTimeOfMonth){
        //修改 ohlc
        item.time = ptr_curr->time;
        if (ptr_curr->high > item.high) item.high = ptr_curr->high;
        if (ptr_curr->low  < item.low ) item.low  = ptr_curr->low ;
        item.close = ptr_curr->close;
        item.amt += ptr_curr->amt;
        item.vol += ptr_curr->vol;
        waitingForCollect = true;
      }else{ // 旧的保存， 一个新的产生。
        vect1.emplace_back(item);
        waitingForCollect = false;

        lastTimeOfMonth = getLastTimeOfMonth(ptr_curr->time); 
        item.time = ptr_curr->time;
        item.open = ptr_curr->open;
        item.high = ptr_curr->high;
        item.low = ptr_curr->low;
        item.close = ptr_curr->close;
        item.amt = ptr_curr->amt;
        item.vol = ptr_curr->vol;
        waitingForCollect = true;
        
      }

    }
  }
  return vect1; 

}

std::vector<ohlc_t> convert2quarter(const std::vector<ohlc_t>& vecDay){
  // convert day ohlc to quarter ohlc
  std::vector<ohlc_t> vect1;
  std::vector<ohlc_t>::const_iterator ptr_curr;
  ohlc_t item;
  bool waitingForCollect = false;
  time_t lastTimeOfQuarter ;
  for (ptr_curr=vecDay.begin(); ; ++ptr_curr) {
    if (ptr_curr == vecDay.end()){
      if (waitingForCollect ) {
        vect1.emplace_back(item);
      }
      break;
    }
    if(ptr_curr==vecDay.begin()) { 
      // 首次进入
      lastTimeOfQuarter = getLastTimeOfQuarter(ptr_curr->time); 
      item.time = ptr_curr->time;
      item.open = ptr_curr->open;
      item.high = ptr_curr->high;
      item.low = ptr_curr->low;
      item.close = ptr_curr->close;
      item.amt = ptr_curr->amt;
      item.vol = ptr_curr->vol;
      waitingForCollect = true;
    }else{
      if(ptr_curr->time <= lastTimeOfQuarter){
        //修改 ohlc
        item.time = ptr_curr->time;
        if (ptr_curr->high > item.high) item.high = ptr_curr->high;
        if (ptr_curr->low  < item.low ) item.low  = ptr_curr->low ;
        item.close = ptr_curr->close;
        item.amt += ptr_curr->amt;
        item.vol += ptr_curr->vol;
        waitingForCollect = true;
      }else{ // 旧的保存， 一个新的产生。
        vect1.emplace_back(item);
        waitingForCollect = false;

        lastTimeOfQuarter = getLastTimeOfQuarter(ptr_curr->time); 
        item.time = ptr_curr->time;
        item.open = ptr_curr->open;
        item.high = ptr_curr->high;
        item.low = ptr_curr->low;
        item.close = ptr_curr->close;
        item.amt = ptr_curr->amt;
        item.vol = ptr_curr->vol;
        waitingForCollect = true;
        
      }

    }
  }
  return vect1; 

}

std::vector<ohlc_t> convert2year(const std::vector<ohlc_t>& vecDay){
  // convert day ohlc to year ohlc
  std::vector<ohlc_t> vect1;
  std::vector<ohlc_t>::const_iterator ptr_curr;
  ohlc_t item;
  bool waitingForCollect = false;
  time_t lastTimeOfYear ;
  for (ptr_curr=vecDay.begin(); ; ++ptr_curr) {
    if (ptr_curr == vecDay.end()){
      if (waitingForCollect ) {
        vect1.emplace_back(item);
      }
      break;
    }
    if(ptr_curr==vecDay.begin()) { 
      // 首次进入
      lastTimeOfYear = getLastTimeOfYear(ptr_curr->time); 
      item.time = ptr_curr->time;
      item.open = ptr_curr->open;
      item.high = ptr_curr->high;
      item.low = ptr_curr->low;
      item.close = ptr_curr->close;
      item.amt = ptr_curr->amt;
      item.vol = ptr_curr->vol;
      waitingForCollect = true;
    }else{
      if(ptr_curr->time <= lastTimeOfYear){
        //修改 ohlc
        item.time = ptr_curr->time;
        if (ptr_curr->high > item.high) item.high = ptr_curr->high;
        if (ptr_curr->low  < item.low ) item.low  = ptr_curr->low ;
        item.close = ptr_curr->close;
        item.amt += ptr_curr->amt;
        item.vol += ptr_curr->vol;
        waitingForCollect = true;
      }else{ // 旧的保存， 一个新的产生。
        vect1.emplace_back(item);
        waitingForCollect = false;

        lastTimeOfYear = getLastTimeOfYear(ptr_curr->time); 
        item.time = ptr_curr->time;
        item.open = ptr_curr->open;
        item.high = ptr_curr->high;
        item.low = ptr_curr->low;
        item.close = ptr_curr->close;
        item.amt = ptr_curr->amt;
        item.vol = ptr_curr->vol;
        waitingForCollect = true;
        
      }

    }
  }
  return vect1; 

}

std::string time2str(const time_t& t) {
    char tmpStr[20];
    struct tm tmpTm;
    memset(tmpStr, '\0', sizeof(tmpStr));
    if (t == 0) {
        time_t now;
        time(&now);
        localtime_s(&tmpTm, &now);
    }
    else {
        localtime_s(&tmpTm, &t);
    }
    strftime(tmpStr, sizeof(tmpStr), "%Y-%m-%d %H:%M:%S", &tmpTm);
    //std::string str{ tmpStr };

    return std::string(tmpStr);

}
std::string time2str(const time_t& t, const std::string& fmt) {
    char tmpStr[30];
    struct tm tmpTm;
    memset(tmpStr, '\0', sizeof(tmpStr));
    if (t == 0) {
        time_t now;
        time(&now);
        localtime_s(&tmpTm, &now);
    }
    else {
        localtime_s(&tmpTm, &t);
    }
    strftime(tmpStr, sizeof(tmpStr), fmt.c_str(), &tmpTm);
    //std::string str{ tmpStr };

    return std::string(tmpStr);

}
time_t str2time(const std::string& str, const std::string& fmt) {
    time_t t1;
    struct tm tm1;
    std::istringstream ss(str);
    memset(&tm1, '\0', sizeof(tm1));
    // ss >> std::get_time(&tm1, "%Y-%m-%d %H:%M:%S");
    ss >> std::get_time(&tm1, fmt.c_str());
    if (ss.fail()) return -1; // failed 
    t1 = std::mktime(&tm1);

    return t1;
}

// split
std::vector<std::string> split(const std::string& input, const char& delimiter)
{
    std::vector<std::string> words{};

    std::istringstream iss(input);
    std::copy(std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        std::back_inserter(words)
    );

    return words;
}

/*
*  textfile format: time o h l c vol amt
*  vol and amt is optional.
*/
std::vector<ohlc_t> readTextFile(const std::string& f,
    const std::string& fmt ,
    const char delimiter ,
    const int skip ) {

    std::string line;
    std::vector<std::string> vec1;
    std::vector<ohlc_t> result;
    std::ifstream infile(f);
    int i = 0;
    vec1.reserve(7);
    result.reserve(100);
    ohlc_t kLine;
    // std::string fmt = std::string("%Y/%m/%d");
    while (std::getline(infile, line)) {
        // splite the line 
        // and save into vecotry.
        if (i < skip) continue;
        vec1 = split(strReplace(line, "," , ""), delimiter);
        if (vec1.size() < 5) continue;
        memset(&kLine, '\0', sizeof(kLine));
        kLine.time = str2time(vec1[0], fmt);
        kLine.open = std::stof(vec1[1]);
        kLine.high = std::stof(vec1[2]);
        kLine.low = std::stof(vec1[3]);
        kLine.close = std::stof(vec1[4]);
        if (vec1.size() >= 6) {
            kLine.vol = std::stof(vec1[5]);
        }
        if (vec1.size() >= 7) {
            kLine.amt = std::stof(vec1[6]);
        }
        result.push_back(kLine);

        vec1.clear();
        i++;
    }
    infile.close();
    return result;
}



std::string strReplace(
    const std::string& source,
    const std::string& pattern,
    const std::string& substitute,
    unsigned int limit )
{
    size_t position = 0;
    std::string alteredString = source;
    unsigned int count = 0;

    while ((position = alteredString.find(pattern, position)) != std::string::npos && count < limit)
    {
        alteredString.replace(position, pattern.length(), substitute);
        position += substitute.length();
        count++;
    }

    return alteredString;
}
