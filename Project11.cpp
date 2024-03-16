#include <cstddef>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <time.h>
#include <vector>
#include "tdxUtil.h"
#include "CZSC.h"
#include <fstream>

void TestPlugin1(int DataLen,float* pfOUT,float* pfINa,float* pfINb,float* pfINc);



//////////////////////////////////////////////////////////////
void test2(){
  time_t t;
  struct std::tm tm_outer;
  struct std::tm tm_inner;
  memset(&tm_outer,'\0' ,sizeof(tm_outer));
  tm_outer.tm_year = 2023 ;
  char stime[30];
  
  
  for (int i = 1; i<=12; ++i) {
    tm_outer.tm_mon  = i ;
    // tm_outer.tm_sec  = 1 ;
    tm_inner = convert_timetm_input(tm_outer);
    tm_inner.tm_mday = 1;  // 指定 tm_mday =0 ，表示上一月最后一日了，
    t = mktime(&tm_inner);
    memset(stime, '\0', sizeof(stime));
    ctime_s(stime, sizeof(stime), &t);
    std::cout << i <<" ctime = " << stime << std::endl;
  }
}

void testTime() {
  time_t now;
  char stime[30];
  struct std::tm nowLocal;
  now = time(NULL);
  std::cout << "now = " << now << std::endl;
  localtime_s(&nowLocal, &now);
  struct std::tm outer_day = convert_timetm_output(nowLocal);
  std::cout << "today is " << nowLocal.tm_year + 1900 << " "
            << nowLocal.tm_mon + 1 << " " << nowLocal.tm_mday << std::endl;
  std::cout << "Today is " << outer_day.tm_year << outer_day.tm_mon << outer_day.tm_mday << std::endl;

  time_t t1 = TdxDay2time(20231127);
  std::cout << "20231211 " << t1 << std::endl;
  time_t t2 = TdxDay2time(20371231);
  std::cout << "20371231 " << t2 << std::endl;
  time_t t3 = TdxDay2time(20991231);
  std::cout << "20991231 " << t3 << std::endl;

  time_t t4 = 1702396799;
  memset(stime, '\0', sizeof(stime));
  ctime_s(stime, sizeof(stime), &t4);
  std::cout << "Today " << stime << std::endl;
  time_t t5 = getLastTimeOfWeek(t4);
  time_t t6 = getLastTimeOfMonth(t4);
  time_t t7 = getLastTimeOfWeek(t1);
  memset(stime, '\0', sizeof(stime));
  ctime_s(stime, sizeof(stime), &t5);
  std::cout << "Last day of week " << stime << std::endl;
  memset(stime, '\0', sizeof(stime));
  ctime_s(stime, sizeof(stime), &t6);
  std::cout << "Last day of month " << stime << std::endl;

  memset(stime, '\0', sizeof(stime));
  ctime_s(stime, sizeof(stime), &t1);
  std::cout << "20231130: " << stime << std::endl;

  memset(stime, '\0', sizeof(stime));
  ctime_s(stime, sizeof(stime), &t7);
  std::cout << "Last day of week " << stime << std::endl;

  // char* const buffer[100];
  // ctime_s(buffer, sizeof(buffer), t1);
  std::cout << "------------------------------------" << std::endl;
  // test2();


}

void printVector(const std::vector<ohlc_t>& vect1) {
  int i = 0 ;
  size_t n = vect1.size();
  std::cout << "sizeof vector is :"<< n << std::endl;
  for (const ohlc_t &day : vect1) {
    ++i;
    //if (i < (n - 60)) continue;
    std::cout << time2str(day.time,"%Y-%m-%d") << " ";
    std::cout << day.open  << " ";
    std::cout << day.high << " ";
    std::cout << day.low  << " ";
    std::cout << day.close  << " ";
    std::cout << day.vol << " " ;
    std::cout << day.amt << " " << std::endl;
  }

}

//////////////////////////////////////////////////////////////////////
/*
 * test readDayBin
 * */

std::vector<Stroke> readTextFile2Stroke(const std::string& f,const char delimiter = '\t',const int skip = 0){
  std::string line;
  std::vector<std::string> vec1;
  std::vector<Stroke> result;
  std::ifstream infile(f);
  int i = 0;
  vec1.reserve(6);
  result.reserve(100);
  Stroke stro;
  while (std::getline(infile, line)) {
      // splite the line 
      // and save into vecotry.
      if (i < skip) continue;
      vec1 = split(strReplace(line, "," , ""), delimiter);
      if (vec1.size() < 6) continue;
      memset(&stro, '\0', sizeof(stro));
      stro.dir = std::stoi(vec1[0]);
      stro.beginpos = std::stoi(vec1[1]);
      stro.endpos = std::stoi(vec1[2]);
      stro.lowValue = std::stof(vec1[3]);
      stro.highValue = std::stof(vec1[4]);
      stro.gap = std::stof(vec1[5]);
      if(stro.lowValue > stro.highValue) {
        std::cout << "Error: low Value is higher than high value low:" << stro.lowValue << " high:"<< stro.highValue << std::endl;
        return result;
      }
      result.push_back(stro);

      vec1.clear();
      i++;
  }
  infile.close();
  return result;
 
}

const std::string TEST_DATA_PATH = "D:\\cwork\\c\\Project11\\test";


void testBin(const std::string& testFile){
  std::string dayFile = TEST_DATA_PATH + "\\" + testFile;
  std::vector<ohlc_t> v_day = readDayBin(dayFile);
  std::vector<ohlc_t> v_week = convert2week(v_day); 
  std::vector<ohlc_t> v_month = convert2month(v_day); 
  std::vector<ohlc_t> v_quarter = convert2quarter(v_day); 
  std::vector<ohlc_t> v_year = convert2year(v_day);
  std::cout << "Day data is as below of file : " << dayFile << std::endl;
  // v_month[101].low = 1025.0;
  printVector(v_month);
  std::cout << "==================================" << std::endl;
  CZSC czsc1 = CZSC(v_month);
  czsc1.printSlices();
  czsc1.add2Strokes();
  czsc1.rawLinesProcess();
  czsc1.printRawLines();
}

#ifdef WITHDT
void test01(const std::string& testFile){
  std::string dayFile = TEST_DATA_PATH + "\\" + testFile;
  std::vector<ohlc_t> v_month = readTextFile(dayFile);
  std::cout << "Day data is as below of file : " << dayFile << std::endl;
  // v_month[101].low = 1025.0;
  printVector(v_month);
  std::cout << "==================================" << std::endl;
  CZSC czsc1 = CZSC(v_month);
  czsc1.setName("SH999999");
  czsc1.setKind("month");
  czsc1.specialParse();
  std::cout << czsc1.getName() << " as below" << std::endl;
  czsc1.printSlices();
  czsc1.add2Strokes();
  czsc1.rawLinesProcess();
  czsc1.printRawLines();
}
#endif

// test 线段 from Stroke
void test11(const std::string& testFile){
  std::string dayFile = TEST_DATA_PATH + "\\" + testFile;
  std::vector<Stroke> stroes = readTextFile2Stroke(dayFile);
  std::cout << "Day data is as below of file : " << dayFile << std::endl;
  // v_month[101].low = 1025.0;
  std::cout << "dir\tbeginpos\tendpos\tlowValue\thighValue\tgap" << std::endl;
  for(const auto& item : stroes) {
    std::cout << std::setw(2)<< item.dir << '\t' << item.beginpos << '\t'<< item.endpos 
              << '\t'<< item.lowValue << '\t'<< item.highValue << '\t'<< item.gap << std::endl;
  }
  std::cout << "==================================" << std::endl;
  CZSC czsc1 = CZSC();
  czsc1.setName("SH999999");
  czsc1.setKind("month");
  std::cout << czsc1.getName() << " as below" << std::endl;
  czsc1._setStrokes(stroes);
  // czsc1.add2Strokes();
  czsc1.rawLinesProcess();
  czsc1.rawLineSpecialParse();
  czsc1.printRawLines();
}



//////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[]) {
  // testBin("sh999999.day");
  // test01("test01.txt");
  // test01("test02.txt");
  test11("test03.txt");
  std::cout << "==================================" << std::endl;
  return 0;
}



