#ifndef CZSC_H
#define CZSC_H

#include <ctime>
#include <list>
#include <string>
#include <vector>
#include "tdxUtil.h"

#define WITHDT

enum Direcrion {
  Down = -1 ,
  Nil  = 0,
  Up   = 1 ,
};

// 分型的种类
enum type_FX {
  Bottom = -1 ,
  Top   = 1 ,
};


struct Slice {  // 切片
  int FXtype;      // -1 bottom ; 1 TOP
  int FXpos;       // 分型的位置
  float FXvalue;   // f分型的极值点
  int FXleft;      // 左边的位置 
  int FXright;     // 右边的位置
  
//  
  int endpos;      // end at this position  
  // int endleft;
  float gap;       // 缺口
  bool canbe;      // 当前切片是否可以构成一笔

};

struct Stroke {  // 笔
  int dir;  // 方向
  int beginpos;   // begin position
  int endpos;     // end position
  float lowValue; // lowest value 
  float highValue; // highest value
  float gap;       // 一笔有可能很大会出现当作一段的情况。
};

struct LineSeg {  // 线段
  int dir;     // 方向
  int beginpos;   // begin position 
  int endpos;     // end position
  int lowpos;    // lowest value position 
  int highpos;     // highest value position
  float lowValue; // lowest value 
  float highValue; // highest value
  int refBegin;    // strokes中的开始位置 reference position
  int refEnd;      // strokes中结束位置   reference position 
  float gap;       // 跳空缺口可能作为一段。
  int status;      // 线段延续的状态，0不能构成，1满足最低标准，2被破坏
  int breakMode;   // 当前线段破坏前一线段方式， 1，第一种 2，第二种。（不是当前线段被破坏的方式）
};

struct RawLine{   //
  int dir;         // Direcrion
  // int beginpos;   // begin position 
  // int endpos;     // end position
  float lowValue; // lowest value 
  float highValue; // highest value
  int refBegin;    // strokes中的开始位置 reference position
  int refEnd;      // strokes中结束位置   reference position 
  float gap;       // 跳空缺口可能作为一段。
  int status;      // 线段延续的状态，0不能构成，1满足最低标准，2被破坏
  int breakMode;   // 当前线段破坏前一线段方式， 1，第一种 2，第二种。（不是当前线段被破坏的方式）
};

bool funCanbe(const Slice& s);

bool funNotCanbe(const Slice& s);

class CZSC {
public:
  CZSC();
#ifdef WITHDT
  CZSC(std::string& id,std::string& name,std::string& kind);
  CZSC(std::vector<time_t>& dt,std::vector<float>& high,std::vector<float>& low,std::vector<float>& vol);
  CZSC(std::vector<ohlc_t>& candlestick);
#else // WIThOUT DT
  CZSC(int DataLen,float* high,float* low,float* vol);
#endif

  CZSC &operator=(CZSC &&) = default;
  CZSC &operator=(const CZSC &) = default;
  ~CZSC()=default;


public:
  void printSlices();

  // add2Slices 增加一个元素
#ifdef WITHDT
  void add2Slices(time_t pdt,float phigh,float plow,float pvol);
  void add2Slices(const ohlc_t& candlestick);
#else // WIThOUT DT
  void add2Slices(float phigh,float plow,float pvol);
#endif

  
  bool isContained(int i, int j);// i 位置的 K线是否被包含于 j
  bool higherThan(int i, int j); // i 比 j 更高
  bool lowerThan(int i, int j); // i 比 j 更低

  // bool SliceContained(Slice& s,int curPos);
  // 获得左手边的位置
  // -1 表示没有
  int getFXleftPos(int FXpos,int FXtype);

  // 获得右手边的位置
  // -1 表示没有
  int getFXrightPos(int FXpos,int FXtype);

  // 判断分型后的延申是否构成笔
  bool isCanbeBI(Slice& s);
  //  strictLevel 笔判断是否成一笔的严格程度
  //  最严格 包含关系处理后 顶底分型之间 至少有一根K线；次级的是 中间那K线被包含了也没关系。 
  //  顶分型那K线与底分型那K线 不能是包含关系；在小级别中保持，在大级别中想放松点！
  //  这里实现的B Level
  /*  
  | Level      | 分型之间   | 顶底之间   | 实现与否   |
  |----------- | ---------- | ---------- | ---------- |
  | A          | 松         | 松         | 否         |
  | B          | 松         | 严         | 是         |
  | C          | 严         | 松         | 否         |
  | D          | 严         | 严         | 否         |
   * 
  */

  bool isZig(Slice& s,int curPos);
  void specialParse1();//  经过笔的初步处理后的进一步处理
  // std::list<Slice>::iterator lookforSliceRight( std::list<Slice>& plist, std::list<Slice>::iterator pit);
  // std::list<Slice>::iterator lookforSliceLeft ( std::list<Slice>& plist, std::list<Slice>::iterator pit);
  // void lookforSlice ( std::list<Slice>& plist, std::list<Slice>::iterator pit,
  //                                          std::list<Slice>::iterator& left1,std::list<Slice>::iterator& right1);

  std::list<Slice>::iterator getFirstExceedingRight(std::list<Slice>& plist, std::list<Slice>::iterator pit);
  std::list<Slice>::iterator getFirstExceedingLeft (std::list<Slice>& plist, std::list<Slice>::iterator pit);

  //  线段及后续处理
  /*
   * add2Strokes 将slice 加如strokes 供后续线段或基于笔的走势的分析。
   * */
  void add2Strokes();
  // void add2LineSegs(int i);
  void add2RawLines(int i);
  void rawLinesProcess();
  void printRawLines();
  Stroke slice2Stroke(const Slice& s);
  LineSeg stroke2LineSeg(int i);
  RawLine stroke2RawLine(int i); 
  
  bool strokeExceedLineSeg(const Stroke& stro,const LineSeg& line);
  bool strokeExceedRawLine(const Stroke& stro,const RawLine& line);
  bool rawLineExceedRawLine(const RawLine& line1,const RawLine& line2);
  bool isEigenFX(std::list<RawLine>::iterator left1, std::list<RawLine>::iterator cur1);
  int  getBreakMod(std::list<RawLine>::iterator left1, std::list<RawLine>::iterator cur1);
  void rawLineSpecialParse();
  int specialParse2();//  经过线段的初步处理后的进一步处理
  std::list<RawLine>::iterator rawLinesMerge(std::list<RawLine>::iterator it1,std::list<RawLine>::iterator it2);
  ////// only for testing //////
public:
  void _setStrokes(const std::vector<Stroke>& stroes) {
    this->strokes.clear();
    if(stroes.size() > this->strokes.capacity()) {
      this->strokes.reserve(stroes.size());
    }
    Stroke stro;
    for(const auto& item: stroes) {
      stro = item; 
      this->strokes.push_back(stro);
    }
  }
  //////////////////////////////
#ifdef WITHDT
public:
  void setStkID(const std::string& id);
  void setName(const std::string& name);
  void setKind(const std::string& kind);
  std::string getStkID();
  std::string getName();
  std::string getKind();


#endif // 
private:

#ifdef WITHDT
  std::string stkID = ""; // id 
  std::string name = ""; // 名称标记
  std::string kind = ""; // 周期标记
  std::vector<time_t> dt;
#endif // 
  std::vector<float> high;
  std::vector<float> low;
  std::vector<float> vol;
  std::list<Slice> slices;
  std::vector<Stroke> strokes;
  std::list<RawLine> rawlines;
  std::vector<LineSeg> lineSegs;

  // temp value  
  int _longItemPos;
  int _firstDirection;
  std::list<Slice>::reverse_iterator _lastIt;

};






#endif // !CZSC_H
