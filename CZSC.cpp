#include "CZSC.h"
#include "tdxUtil.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <iomanip>
#include <iterator>
#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <time.h>



bool funCanbe(const Slice& s){
  return s.canbe;
}

bool funNotCanbe(const Slice& s){
  return !s.canbe;
}

CZSC::CZSC() {
  int DataLen = 250 ;
  this->dt.reserve(DataLen);
  this->high.reserve(DataLen);
  this->low.reserve(DataLen);
  this->vol.reserve(DataLen);

}

#ifdef WITHDT


CZSC::CZSC(std::string& id,std::string& name,std::string& kind)
  :stkID(id),name(name), kind(kind) {
  int DataLen = 250 ;
  this->dt.reserve(DataLen);
  this->high.reserve(DataLen);
  this->low.reserve(DataLen);
  this->vol.reserve(DataLen);

}


CZSC::CZSC(std::vector<time_t>& pdt,std::vector<float>& phigh,std::vector<float>& plow,std::vector<float>& pvol){
  size_t DataLen = std::min(phigh.size(),plow.size());
  if (DataLen < pdt.size()) DataLen = pdt.size();
  this->dt.reserve(DataLen);
  this->high.reserve(DataLen);
  this->low.reserve(DataLen);
  this->vol.reserve(DataLen);

  for (int i=0; i< DataLen ; i++) {
    this->add2Slices(pdt[i],phigh[i],plow[i],pvol[i]);
  }
  
}


CZSC::CZSC(std::vector<ohlc_t>& candlesticks){
  size_t DataLen = candlesticks.size();

  this->dt.reserve(DataLen);
  this->high.reserve(DataLen);
  this->low.reserve(DataLen);
  this->vol.reserve(DataLen);
  int k = 0;
  for (const ohlc_t& item : candlesticks) {
    this->add2Slices(item);
    k++;
  }
  
}

#else // WIThOUT DT


CZSC::CZSC(int DataLen,float* phigh,float* plow,* pvol) {
  this->high.reserve(DataLen);
  this->low.reserve(DataLen);
  this->vol.reserve(DataLen);

  for (int i=0; i< DataLen ; i++) {
    this->add2Slices(phigh[i],plow[i],pvol[i]);
  }

}

#endif // WITHDT
/*
CZSC::~CZSC() {
#ifdef WITHDT
  this->dt.clear();
#endif
  this->high.clear();
  this->low.clear();
  this->vol.clear();
  this->slices.clear();
  this->strokes.clear();
  this->lineSegs.clear();
}
*/
#ifdef WITHDT
void CZSC::setStkID(const std::string& id){
  this->stkID = id;
}
void CZSC::setName(const std::string& name){
  this->name = name;
}
void CZSC::setKind(const std::string& kind){
  this->kind = kind;
}

std::string CZSC::getStkID(){
  return this->stkID;
}
std::string CZSC::getName(){
  return this->name;
}
std::string CZSC::getKind(){
  return this->kind;
}


#endif // 



// i 位置 是否 被 j位置包含
bool CZSC::isContained(int i, int j) {
  if (high[i] <  high[j] && low[i] > low[j]) return true;
  if (high[i] == high[j] && low[i] > low[j]) return true;
  if (high[i] < high[j] && low[i] == low[j]) return true;
  if (high[i] == high[j] && low[i] == low[j] && vol[i] <= vol[j]) {
    return true;
  }
  
  return false;
}

int CZSC::getFXleftPos(int FXpos,int FXtype){
  int j = FXpos;
  float comValue;
  if(FXtype == Top) {
    j = FXpos;
    comValue = low[j];
    j--;
    while (j>=0 && low[j] >= comValue) {
      comValue = low[j];
      j--;
    }
  }
  if(FXtype == Bottom) {
    j = FXpos;
    comValue = high[j];
    j--;
    while (j>=0 && high[j] <= comValue) {
      comValue = high[j];
      j--;
    }
  }
  
  return j;

}


int CZSC::getFXrightPos(int FXpos,int FXtype){
  int j = FXpos;;
  float comValue;
  if(FXtype == Top) {
    j = FXpos;
    comValue = low[j];
    j++;
    while (j<high.size() && low[j] >=comValue ) {
      comValue = low[j];
      j++;
    }
  }
  if(FXtype == Bottom) {
    j = FXpos;
    comValue = high[j];
    j++;
    while (j<high.size() && high[j] <= comValue ) {
      comValue = high[j];
      j++;
    }
  }
  if(j == high.size()) j = -1; 
  return j;

}

  // 判断分型后的延申是否构成笔的初始判断，
  // 并未考虑缺口的问题。
bool CZSC::isCanbeBI(Slice& s) {
  // if (s.canbe) return true;
  if ( (s.endpos - s.FXpos) < 4 ) return false; // 不足5根
  int j = s.endpos;
  float comValue;

  //  同一笔中，顶分型中最高那K线的区间至少要有一部分高于底分型中最低那K线的区间 （参见77课）
  // if (strictLevel == 'B' || strictLevel == 'D') {
    if((s.FXtype == Bottom) && high[s.endpos] <= high[s.FXpos]) return false;
    if((s.FXtype == Top)    && low[s.endpos]  >= low[s.FXpos])  return false;
  // }

  // 目前实现的都是松笔
  // 顶和底之间至少有一个K线不属于顶分型与底分型.
  if (s.FXtype == Bottom) {
    comValue = low[j];
    j--;
    while(j > s.FXright && low[j] >= comValue) { 
      comValue = low[j];
      j--; 
    }
    if( (j - s.FXright) >1) return true;
  }else {
    comValue = high[j];
    j--;
    while(j > s.FXright && high[j] <= comValue) { 
      comValue = high[j];
      j--; 
    }
    if( (j - s.FXright) >1) return true;

  }
  // 有的地方要求，比如，顶分型后是底分型，那么要求这个底分型的最低点要低于分型的左边的低点。
  //  todo
  return false;
}

// 当前分型延申过程中 是否转折了
bool CZSC::isZig(Slice& s,int curPos){
  int j ;
  if(s.FXtype == Bottom) {
    j = curPos-1;
    while (j>= s.endpos) {
      if(low[curPos] < low[j]) return true;
      j--;
    }
  } 
  if(s.FXtype == Top) {
    j = curPos-1;
    while (j>= s.endpos) {
      if(high[curPos] > high[j]) return true;
      j--;
    }
  } 

  return false;
}

bool CZSC::higherThan(int i, int j) {
  return high[i] > high[j] || (high[i] == high[j] && vol[i] > vol[j])  ;
} // i 比 j 更高
bool CZSC::lowerThan(int i, int j){
  return low[i] < low[j] || (low[i] == low[j] && vol[i] > vol[j]);
} // i 比 j 更低



#ifdef WITHDT
void CZSC::add2Slices(time_t pdt,float phigh,float plow,float pvol){
#else
void CZSC::add2Slices(float phigh, float plow, float pvol) {
#endif

  if(high.size() == 0 ) {
    _longItemPos = 0;
    _firstDirection = Nil;
  } 
#ifdef WITHDT
  dt.push_back(pdt);
#endif
  high.push_back(phigh);
  low.push_back(plow);
  vol.push_back(pvol);
  int curPos = high.size() - 1;
  if(slices.size() ==0 ) {
    if(curPos > 0 ) {
      if(isContained(_longItemPos,curPos)) {
        _longItemPos = curPos;
      }else {
        if(high[curPos] > high[_longItemPos] && low[curPos] > low[_longItemPos] ){
          _firstDirection = Up;
        }else if(high[curPos] < high[_longItemPos] && low[curPos] < low[_longItemPos]){
          _firstDirection = Down;
        }
        if(_firstDirection == Up || _firstDirection == Down){ //  第一次方向的暂时确定 与分型的产生
          Slice s;
          s.FXtype = (_firstDirection == Up ? Bottom : Top) ;
          s.FXpos = _longItemPos;
          s.FXvalue = (s.FXtype == Bottom ? low[s.FXpos] : high[s.FXpos]);
          // 确定分型的左边
          s.FXleft = getFXleftPos(s.FXpos, s.FXtype);
          s.FXright = getFXrightPos(s.FXpos, s.FXtype);  //curPos;
          s.endpos = curPos;
          s.gap = 0; // todo 记录GAP.
          s.canbe = false;

          slices.push_back(s);

        }
      }
    }
  }else {
    _lastIt = slices.rbegin(); 
    // 异常的K线，包含了原来的切片。 
    
    bool justNext = false;  // 前面处理了 则后续不处理
    bool reverseBreakdown = false; // 反向击穿了
    // 新的切片不构成笔，但又被一下子反向击穿了
    while ( _lastIt != slices.rend() &&  _lastIt->canbe == false  &&
          ((_lastIt->FXtype == Bottom && high[curPos] > high[_lastIt->endpos] && low[curPos] < _lastIt->FXvalue) || 
           (_lastIt->FXtype == Top    && low[curPos] < low[_lastIt->endpos]   && high[curPos] > _lastIt->FXvalue) ) ) 
    {
        reverseBreakdown = true;
        slices.pop_back();
        _lastIt = slices.rbegin();

    }
    if (reverseBreakdown) {
        if (_lastIt == slices.rend()) {
            // 列表空了， 记录要比较的 值，并进入下一循环。continue
            _longItemPos = curPos;
            _firstDirection = Nil;
            justNext = true;

        }
        else if (_lastIt->canbe) {
            // 击穿了一个可构成笔的Slice, 后续是否改变方向并不知道，暂时记录该Slice 中
            _lastIt->endpos = curPos;
            justNext = true;
        }
    }


    if (! justNext) {
      // 正常延申
      //if (_lastIt->FXtype == Bottom &&  lowerThan(_lastIt->FXpos, curPos) && higherThan(curPos,_lastIt->endpos))
      if( (_lastIt->FXtype == Bottom && higherThan(curPos, _lastIt->endpos) ) || 
          (_lastIt->FXtype == Top    && lowerThan(curPos, _lastIt->endpos)) )
      {
        _lastIt->endpos = curPos;
        if (!_lastIt->canbe) {
            _lastIt->canbe = isCanbeBI(*_lastIt);
        }
      }else if ( isContained(curPos, curPos - 1) ) { // 当前K 被前一K 包含了， 啥也不处理
            justNext = true;
      }else if (isZig(*_lastIt, curPos)) {           // 遇到转折，新的分型产生
          Slice s;
          s.FXtype = (_lastIt->FXtype == Bottom ? Top : Bottom);
          s.FXpos = _lastIt->endpos;
          s.FXvalue = (s.FXtype == Bottom ? low[s.FXpos] : high[s.FXpos]);
          // 确定分型的左边
          s.FXleft = getFXleftPos(s.FXpos, s.FXtype);
          s.FXright = curPos;
          s.endpos = curPos;
          s.gap = 0; // todo 记录GAP.
          s.canbe = false;

          slices.push_back(s);
          _lastIt = slices.rbegin();

      }

    }

    if(!justNext && slices.size()>2) {
      std::list<Slice>::iterator leftIt1 = slices.end();
      std::list<Slice>::iterator leftIt2 = slices.end();
      while(  slices.size() > 2  ) {
        _lastIt = slices.rbegin(); 
        leftIt1 = slices.end(); 
        leftIt2 = slices.end(); 
        std::advance(leftIt1, -2);
        std::advance(leftIt2, -3);
        if(leftIt1->canbe && leftIt2->canbe ) break;
        // 余下的执行， _1 _2 至少有一个不能成立。
        if(_lastIt->FXtype == Bottom && higherThan(curPos, leftIt1->FXpos) )  {
          if (lowerThan(leftIt2->FXpos, _lastIt->FXpos) ) {
            //slices.pop_back();
            //slices.pop_back();

            leftIt2->endpos = _lastIt->endpos;
            if (! leftIt2->canbe ) {
                leftIt2->canbe = isCanbeBI(*leftIt2);
            }
            slices.erase(leftIt1, slices.end());
            _lastIt = slices.rbegin();
            
            continue; 
          }else if (lowerThan(_lastIt->FXpos,leftIt2->FXpos) && slices.size() > 3 ) {
            std::list<Slice>::iterator leftIt3 = slices.end();
            std::advance(leftIt3,-4);
            if(higherThan(leftIt3->FXpos, leftIt1->FXpos)){
              // leftIt1 leftIt2 删除！
              std::list<Slice>::iterator tempLast = slices.end();
              tempLast--;
              
              leftIt3->endpos = tempLast->FXpos;
              if(! leftIt3->canbe) {
                leftIt3->canbe = isCanbeBI(* leftIt3);
              }
              slices.erase(leftIt2,tempLast);
              continue;   
            }
            break;
          }
        }
        if(_lastIt->FXtype == Top && lowerThan(curPos, leftIt1->FXpos) ) {
          if( higherThan(leftIt2->FXpos, _lastIt->FXpos)) {
            //slices.pop_back();   
            //slices.pop_back();
             
            leftIt2->endpos = _lastIt->endpos;
            if (!leftIt2->canbe) {
                leftIt2->canbe = isCanbeBI(*leftIt2);
            }
            slices.erase(leftIt1, slices.end());

            _lastIt = slices.rbegin();
            continue;
          }else if(higherThan(_lastIt->FXpos, leftIt2->FXpos) && slices.size() >3 ) {
            std::list<Slice>::iterator leftIt3 = slices.end();
            std::advance(leftIt3,-4);
            if(lowerThan(leftIt3->FXpos, leftIt1->FXpos)){
              // leftIt1 leftIt2 删除！ _lastIt 还得保留！
              std::list<Slice>::iterator tempLast = slices.end();
              tempLast--;
              leftIt3->endpos = tempLast->FXpos;
              if (!leftIt3->canbe) {
                  leftIt3->canbe = isCanbeBI(*leftIt3);
              }
              slices.erase(leftIt2,tempLast);
              _lastIt = slices.rbegin();
              continue;   
            }
            break; 
          }
          
        }
        break;

      }  // endwhile
    } //endif


  }


}




#ifdef WITHDT
void CZSC::add2Slices(const ohlc_t& candlestick) {
    this->add2Slices(candlestick.time, candlestick.high, candlestick.low, candlestick.vol);
}
#endif // WITHDT

void CZSC::specialParse1(){
  // 是否有缺口，及后续的处理
  // 仍然有canbe == false 的处理
  // 开头的删除，末尾的留待其发展完成。
  
  std::list<Slice>::iterator itLeft1;
  std::list<Slice>::iterator itRight1;
  std::list<Slice>::iterator itTemp1;
  std::list<Slice>::iterator itTemp2;
  // 去掉开头不能构成笔的Slice
  std::list<Slice>::iterator itHead = this->slices.begin();
  while(! itHead->canbe && slices.size() >1 ){
    itRight1 = getFirstExceedingRight(slices, itHead);
    if(itRight1 == slices.end()) break; // 走势未完成等待。
    if(itRight1->FXtype != itHead->FXtype) {
      slices.pop_front();
      itHead = slices.begin();
      continue;
    }else{
      // 同向合并
      itHead->endpos = itRight1->endpos;
      itHead->canbe  = isCanbeBI(* itHead);
      itTemp2 = itHead; 
      itTemp2++;
      slices.erase(itTemp2,++itRight1);
    }
  }

  // auto lambdaFun1 =  [](const Slice& s) { return !s.canbe;} ;
  int countNotCanbeR = 0;
  int countNotCanbeL = 0;
  int countCanbeR = 0;
  int countCanbeL = 0;
  std::list<Slice>::iterator itNotCanbe;
  size_t old_size = 0;
  char dirMerge = 'U'; // 合并方向
  while(slices.size() > 2){
    itNotCanbe = std::find_if(slices.begin() , slices.end() , funNotCanbe );
    if(itNotCanbe == slices.end()) break; // 没找到未完成的slice 
    // 如果时最后一笔，等待发展
    itTemp2 = slices.end(); itTemp2 --;
    if(itNotCanbe == itTemp2) break; 
    
    old_size = slices.size();
    // 寻找 越过 it 的位置
    itRight1 = getFirstExceedingRight(slices,itNotCanbe) ;
    dirMerge = 'U';
    if(itRight1 == slices.end()) {  // 走势未完成 
      // 虽然未完成，但如果合并左边消亡的canbe 更少，那么合并左边。
      itTemp2 = slices.end(); itTemp2--;
      countNotCanbeR = std::count_if(itNotCanbe, itTemp2, funNotCanbe);
      if(countNotCanbeR == 1 ) {    
        itLeft1 = getFirstExceedingLeft(slices, itNotCanbe); // 高低点的关系
        if(itLeft1 != slices.end() && itLeft1->FXtype == itNotCanbe->FXtype ) {
          countCanbeL = std::count_if(itLeft1, itNotCanbe, funCanbe);
          countCanbeR = std::count_if(itNotCanbe, slices.end(), funCanbe);
          if(countCanbeL < countCanbeR) {
            dirMerge = 'L';
          }

        }
      }
      
    }else {
        if(itRight1->FXtype == itNotCanbe->FXtype) {
          // 同向
          // itNotCanbe 是归到前面呢，还是归到后面呢
          // 刚一超越时，如果itRight1时可构成的，那么前后共可消亡的不可构成的slice
          // 如果当下itRight1- 暂时不可构成，那么可等待其可构成，或者被证明不可构成了。
          // 就看哪种消亡的不可构成slice多，消亡的可构成slice少！
          // countNotCanbeR = std::count_if(itNotCanbe, itRight1, funNotCanbe); 
          //寻找左边的可能
          itLeft1 = getFirstExceedingLeft(slices, itNotCanbe);
          if(itLeft1 == slices.end()) {
            // not found 只能向右合并。
            dirMerge = 'R';
          }else{
            if(itLeft1->FXtype != itNotCanbe->FXtype ) {
              // 异向 只能向右合并。
              dirMerge = 'R';
            }else{
              // 左边也是同向,如果右边有更多的NotCanbe 
              countNotCanbeR = std::count_if(itNotCanbe, itRight1, funNotCanbe);
              if(countNotCanbeR >1) {
                // 向右
                dirMerge = 'R';
              }else {
                // 比较Canbe 的Slice 数量，谁少取谁。
                // itTemp2 = itNotCanbe; itTemp2 ++;
                countCanbeL = std::count_if(itLeft1, itNotCanbe, funCanbe);
                countCanbeR = std::count_if(itNotCanbe, itRight1, funCanbe);
                if(countCanbeL <= countCanbeR) {
                  // 取左合并
                  dirMerge = 'L';
                }else{
                  // 取右合并
                  dirMerge = 'R';
                }

              }
                
            }

          }

        }else{ // 异向
          // countNotCanbeR = std::count_if(itNotCanbe, itRight1, funNotCanbe); 
          //寻找左边的可能
          itLeft1 = getFirstExceedingLeft(slices, itNotCanbe);
          if(itLeft1 == slices.end()) {
            // 右边异向突破，但左边没有突破
            // 那就只能将左边全部干掉
            itTemp1 = itNotCanbe ; itTemp1 ++;
            slices.erase(slices.begin(), itTemp1); //
            // todo: message yellow
          }else {
            if( itLeft1->FXtype != itNotCanbe->FXtype){
              // 都是 异向超出 删除 itLeft1 之后到 itRight1之间的。
              itLeft1->endpos = itRight1->endpos;
              if(! itLeft1->canbe) {
                itLeft1->canbe = isCanbeBI(* itLeft1);
              }
              itTemp1 = itLeft1 ; itTemp1 ++;
              itTemp2 = itRight1; itTemp2 ++;
              slices.erase(itTemp1,itTemp2);

            }else{
              // 取左合并
              dirMerge = 'L';
              
            }
          }
          
        }
    }
    if(dirMerge == 'R') {
      itNotCanbe->endpos = itRight1->endpos;
      itNotCanbe->canbe = isCanbeBI(* itNotCanbe );
      itTemp1 = itNotCanbe ; itTemp1 ++;
      itTemp2 = itRight1; itTemp2 ++;
      slices.erase(itTemp1,itTemp2); // 删除之后的元素

    }else if(dirMerge == 'L') {
      itLeft1->endpos = itNotCanbe->endpos;
      if(! itLeft1->canbe) {
        itLeft1->canbe = isCanbeBI(* itLeft1);
      }
      itTemp1 = itLeft1 ; itTemp1 ++;
      itTemp2 = itNotCanbe; itTemp2 ++;
      slices.erase(itTemp1,itTemp2);

    }

    // 找来找去 没变化
    if(slices.size() == old_size) {
      break;  
      // yellow todo // adding message  yellow
    }
  } // endwhile
  this->_lastIt = slices.rbegin(); 
}

void CZSC::add2Strokes(){
  // add canbe slices to strokes
  this->strokes.clear();
  strokes.reserve(slices.size());
  for(const auto& item : this->slices){
    if(!item.canbe) break;
  this->strokes.push_back(slice2Stroke(item));

  }

}


void CZSC::rawLinesProcess(){
  /**/
  if(this->vol.size() == 0)  vol.push_back(0.0); // only for test to avoid crash.
  for(int i = 0;i<this->strokes.size();i++) {
    add2RawLines(i);
  }

}
void CZSC::printRawLines() {
  std::cout << "=========== RAWLINES as below============" << std::endl;
  std::cout << "dir\tbegin\tend\tstatus\tlowValue\thighValue" << std::endl;
  for(const auto& item : this->rawlines) {
    std::cout <<std::setw(2) << item.dir << '\t' << item.refBegin << '\t'<< item.refEnd << '\t'
        << item.status << '\t'<< item.lowValue << '\t'<< item.highValue << std::endl;
  }
}
void CZSC::add2RawLines(int i){
  std::list<RawLine>::iterator lastIt,lastIt_1,lastIt_2;
  if(this->rawlines.size()<2) {
    this->rawlines.push_back(stroke2RawLine(i));
  }else{
    lastIt   = this->rawlines.end() ; lastIt--;
    lastIt_1 = this->rawlines.end() ; lastIt_1--; lastIt_1--; 
    if(lastIt_1->status == 2) { // 前面的线段已被破坏，不用再比较什么
        // directly add 
        this->rawlines.push_back(stroke2RawLine(i));
        // if(this->rawlines.size() >=2) {
        //   lastIt   = this->rawlines.end() ; lastIt--;
        //   lastIt_1 = this->rawlines.end() ; lastIt_1--; lastIt_1--; 
        //   lastIt->breakMode = getBreakMod(lastIt_1, lastIt);
        // }
    }else {
      const Stroke& stro = this->strokes[i];
      if(strokeExceedRawLine(stro, * lastIt_1)) { // 超越
        if(lastIt->status == 0 || (lastIt->status == 1 && lastIt->breakMode == 2 )) { // lastIt_1与当前夹着的是 单根 或者多根，多根时第二种类型的破坏
          rawlines.pop_back();  //弹出，归并
          lastIt_1->refEnd = i;
          if(lastIt_1->status ==0 ) lastIt_1->status = 1;
          if(lastIt_1->dir == Up ) {
            if(stro.highValue > lastIt_1->highValue ) lastIt_1->highValue = stro.highValue;
          }else {
            if(stro.lowValue < lastIt_1->lowValue)    lastIt_1->lowValue  = stro.lowValue;
          }
        }else{
          // directly add 
          this->rawlines.push_back(stroke2RawLine(i));
          // if(this->rawlines.size() >=2) {
          //   lastIt   = this->rawlines.end() ; lastIt--;
          //   lastIt_1 = this->rawlines.end() ; lastIt_1--; lastIt_1--; 
          //   lastIt->breakMode = getBreakMod(lastIt_1, lastIt);
          // }
        } 
        size_t old_size = 0;
        bool   bCombine = false;
        while(this->rawlines.size()>=3) { // 进一步的归并
          old_size = this->rawlines.size();
          bCombine = false;
          lastIt   = this->rawlines.end() ; lastIt--;
          lastIt_1 = this->rawlines.end() ; lastIt_1--; lastIt_1--; 
          lastIt_2 = this->rawlines.end() ; lastIt_2--; lastIt_2--; lastIt_2--;
          if(lastIt_2->status == 2) break;
          if(rawLineExceedRawLine(*lastIt,*lastIt_2)) {
            if(lastIt_1->status == 0) {
              bCombine = true;
            }else if(lastIt_2->status != 2 && lastIt_1->status == 1 && lastIt_1->breakMode == 2) { // 前一线段对前前线段是第二种类型的破坏，且当前超越
              // 考察分型是否成立
              if(isEigenFX(lastIt_1, lastIt)) {
                // 标记线段被破坏
                lastIt_2->status = 2;
                lastIt_1->status = 2;
                break;
              } else {
                bCombine = true;
              }
            } 
          }else{
            break;
          }

          if(bCombine == true) {
              lastIt_2->refEnd = lastIt->refEnd;
              if(lastIt_2->status ==0 ) lastIt_2->status = 1;
              if(lastIt_2->dir == Up) {
                if(lastIt->highValue > lastIt_2->highValue) lastIt_2->highValue = lastIt->highValue;
              }else if(lastIt_2->dir == Down) {
                if(lastIt->lowValue < lastIt_2->lowValue )  lastIt_2->lowValue  = lastIt->lowValue ;
              }
              rawlines.pop_back();  
              rawlines.pop_back(); 
          }

          if(this->rawlines.size() == old_size) break;
        } // endwhile
        if(this->rawlines.size() >=2) {
          lastIt   = this->rawlines.end() ; lastIt--;
          lastIt_1 = this->rawlines.end() ; lastIt_1--; lastIt_1--; 
          if(lastIt->breakMode != 1) lastIt->breakMode = getBreakMod(lastIt_1, lastIt);
          if(this->rawlines.size() ==2 ) {
            if(lastIt->breakMode == 1) {
              lastIt_1->status = 2;
            }
          }else{ //>= 3
            lastIt_2 = this->rawlines.end() ; lastIt_2--; lastIt_2--; lastIt_2--;
            if(lastIt_2->status == 1 && lastIt_1->breakMode == 2){
              if(isEigenFX(lastIt_1, lastIt)) {  // 前线段对前前线段是第二种类型的破坏，考察分型是否成立
                lastIt_2->status = 2;
              }
            }
            if(lastIt_2->status == 2 && lastIt->breakMode == 1) { // 当前线段对前一段是笔破坏，且前前线段已被破坏
              lastIt_1->status = 2;
            }
          } // endelse size() >=3
        }

      }else {
        // directly add 
        this->rawlines.push_back(stroke2RawLine(i));
        
      }

    }
  }
  if(this->rawlines.size() >=2) {
    lastIt   = this->rawlines.end() ; lastIt--;
    lastIt_1 = this->rawlines.end() ; lastIt_1--; lastIt_1--; 
    lastIt->breakMode = getBreakMod(lastIt_1, lastIt);
  }


}

/*
 * 特殊线段的处理
 * */
void CZSC::rawLineSpecialParse(){
  std::list<RawLine>::iterator tmpIt,tmpIt_1,tmpIt_2;
  // 新的延申处理 类似 79 课 ， 3-8 的自然延申
  size_t old_size = 0;
  if(rawlines.size() >2) {
    tmpIt = rawlines.end(); tmpIt--;
    tmpIt_1 = tmpIt  ; tmpIt_1--;
    tmpIt_2 = tmpIt_1; tmpIt_2--;
    while(rawlines.size() >2) {
      if(tmpIt->status != 0 && tmpIt_1->status == 0 && tmpIt_2->status == 0) {
        tmpIt_2->refEnd = tmpIt->refEnd;
        tmpIt_2->status = 1;//tmpIt->status;
        if(tmpIt_2->dir == Down && tmpIt->lowValue < tmpIt_2->lowValue) {
          tmpIt_2->lowValue = tmpIt->lowValue;
        }else if(tmpIt_2->dir == Up && tmpIt->highValue > tmpIt_2->highValue) {
          tmpIt_2->highValue = tmpIt->highValue;
        }
        rawlines.erase(tmpIt_1,++tmpIt);
        tmpIt = tmpIt_2;
        if(tmpIt == rawlines.begin()) break;
        tmpIt_1 = tmpIt  ; tmpIt_1--;
        if(tmpIt->breakMode != 1) tmpIt->breakMode = getBreakMod(tmpIt_1, tmpIt);
        if(tmpIt_1->status == 1 && tmpIt->breakMode == 1) {
          tmpIt_1->status =2; // 第一钟类型的破坏，结束前一段。
        }
        if(tmpIt_1 == rawlines.begin()) break;
        tmpIt_2 = tmpIt_1; tmpIt_2--;
        
      }else if(tmpIt->status != 0 && tmpIt_1->status != 0 && tmpIt_2->status == 0) {
        if(tmpIt->breakMode != 1) tmpIt->breakMode = getBreakMod(tmpIt_1,tmpIt);
        if(tmpIt->status == 2 || tmpIt->breakMode == 1) {
          tmpIt_2->refEnd = tmpIt->refEnd;
          tmpIt_2->status = 1;//tmpIt->status;
          if(tmpIt_2->dir == Down && tmpIt->lowValue < tmpIt_2->lowValue) {
            tmpIt_2->lowValue = tmpIt->lowValue;
          }else if(tmpIt_2->dir == Up && tmpIt->highValue > tmpIt_2->highValue) {
            tmpIt_2->highValue = tmpIt->highValue;
          }
          rawlines.erase(tmpIt_1,++tmpIt);
          tmpIt = tmpIt_2;
          if(tmpIt == rawlines.begin()) break;
          tmpIt_1 = tmpIt  ; tmpIt_1--;
          if(tmpIt->breakMode != 1) tmpIt->breakMode = getBreakMod(tmpIt_1, tmpIt);
          if(tmpIt_1->status == 1 && tmpIt->breakMode == 1) {
            tmpIt_1->status =2; // 第一钟类型的破坏，结束前一段。
          }
          if(tmpIt_1 == rawlines.begin()) break;
          tmpIt_2 = tmpIt_1; tmpIt_2--;
        }else {
          tmpIt--;
          if(tmpIt == rawlines.begin()) break;
          tmpIt_1 = tmpIt  ; tmpIt_1--;
          if(tmpIt_1 == rawlines.begin()) break;
          tmpIt_2 = tmpIt_1; tmpIt_2--;
        }
      }else{
        tmpIt--;
        if(tmpIt == rawlines.begin()) break;
        tmpIt_1 = tmpIt  ; tmpIt_1--;
        if(tmpIt_1 == rawlines.begin()) break;
        tmpIt_2 = tmpIt_1; tmpIt_2--;
      }
    } // endwhile

  }

  // 笔破坏后先破笔的端点。
  for(std::list<RawLine>::iterator it=rawlines.begin(); it!=rawlines.end();it++){
    //bool bProcess = false;
    if(it->status == 0 && (it->breakMode == 1 || it==rawlines.begin() )) {
      tmpIt = it;
      tmpIt++; if(tmpIt == rawlines.end()) break;
      tmpIt++; if(tmpIt == rawlines.end()) break;

      if(rawLineExceedRawLine(*tmpIt, *it)) {
        if(it->dir == Down) {
          it->lowValue = tmpIt->lowValue;
        }else if(it->dir == Up) {
          it->highValue = tmpIt->highValue;
        }
        it->refEnd = tmpIt->refEnd;
        it->status = 1;
        tmpIt_1 = it;
        rawlines.erase(++tmpIt_1,++tmpIt);
        if(it != rawlines.begin()) {
          tmpIt_1 = it; tmpIt_1--;
          if(tmpIt_1->status == 1) tmpIt_1->status =2 ;
        }

      }
    } 
  }
  // 第二钟类型的处理
   if(rawlines.size() >2) {
    tmpIt = rawlines.end(); tmpIt--;
    tmpIt_1 = tmpIt  ; tmpIt_1--;
    tmpIt_2 = tmpIt_1; tmpIt_2--;
    while(rawlines.size() >2) {
      if(tmpIt_2->status ==1 && tmpIt_1->breakMode == 2) {
        if(isEigenFX(tmpIt_1, tmpIt)) {
          tmpIt_2->status = 2;
          tmpIt_1->status = 2; 
          // goto nextone.
          tmpIt--;
          if(tmpIt == rawlines.begin()) break;
          tmpIt_1 = tmpIt  ; tmpIt_1--;
          if(tmpIt_1 == rawlines.begin()) break;
          tmpIt_2 = tmpIt_1; tmpIt_2--;
        }else {
          tmpIt_2->refEnd = tmpIt->refEnd;
          if(tmpIt_2->dir == Down && tmpIt->lowValue < tmpIt_2->lowValue) {
            tmpIt_2->lowValue = tmpIt->lowValue;
          }else if(tmpIt_2->dir == Up && tmpIt->highValue > tmpIt_2->highValue) {
            tmpIt_2->highValue = tmpIt->highValue;
          }
          rawlines.erase(tmpIt_1,++tmpIt);
          tmpIt = tmpIt_2;
          if(tmpIt == rawlines.begin()) break;
          tmpIt_1 = tmpIt  ; tmpIt_1--;
          if(tmpIt->breakMode == 0) tmpIt->breakMode = getBreakMod(tmpIt_1, tmpIt);
          if(tmpIt_1->status == 1 && tmpIt->breakMode == 1) {
            tmpIt_1->status =2; // 第一钟类型的破坏，结束前一段。
          }
          if(tmpIt_1 == rawlines.begin()) break;
          tmpIt_2 = tmpIt_1; tmpIt_2--;

        }
      }else{
        tmpIt--;
        if(tmpIt == rawlines.begin()) break;
        tmpIt_1 = tmpIt  ; tmpIt_1--;
        if(tmpIt_1 == rawlines.begin()) break;
        tmpIt_2 = tmpIt_1; tmpIt_2--;
      }
    } // endwhile

  }
  // 第一钟类型的破坏，状态更新
  if(rawlines.size() >1) {
    tmpIt = rawlines.end(); tmpIt--;
    tmpIt_1 = tmpIt  ; tmpIt_1--;
    // tmpIt_2 = tmpIt_1; tmpIt_2--;
    while(rawlines.size() >1) {
      if(tmpIt->breakMode != 1) tmpIt->breakMode = getBreakMod(tmpIt_1, tmpIt);
      if(tmpIt->breakMode ==1 && tmpIt->status != 0 && tmpIt_1->status == 1) {
        tmpIt_1->status =2; // 笔破坏马上成立
      }
      tmpIt--;
      if(tmpIt == rawlines.begin()) break;
      tmpIt_1 = tmpIt  ; tmpIt_1--;
    }
  }
}

/*
 * 线段的特殊处理
 * */
int CZSC::specialParse2() {
  std::list<RawLine>::iterator it,itLeft1,itLeft2,itRight1,itRight2,itTemp,itTempLeft;
  it= rawlines.begin();
  int firstPos = 0;
  while(it != rawlines.end()) {
    if(it->status != 0 ) {
      break;
    }
    firstPos ++;
    it++;
  }
  
  if(it == rawlines.end()) {
    return 0;
  }

  itTemp = rawlines.end();
  itLeft2 = it;
  float comValue ;
  if(it->dir == Down) {
    comValue = it->highValue;
  }else if(it->dir== Up) {
    comValue = it->lowValue;
  }
  while(itLeft2 != rawlines.end()) {
    itLeft2-- ; if(itLeft2 == rawlines.end()) break;
    itLeft2-- ; if(itLeft2 == rawlines.end()) break;
    if(it->dir == Down && itLeft2->highValue >= comValue) {
      comValue = itLeft2->highValue;
      itTemp = itLeft2;
    }else if(it->dir == Up && itLeft2->lowValue <= comValue) {
      comValue = itLeft2->lowValue;
      itTemp = itLeft2;
    }
  }
  if (itTemp != rawlines.end()) { // if found
    // merge itTemp to it
    it = rawLinesMerge(itTemp, it);
  }
  size_t dist = std::distance(rawlines.begin(), it);
  // 开头有不规则的
  if(dist == 1) {
    rawlines.pop_front();
  }else if(dist == 2) {
    itLeft2 = rawlines.begin();
    const Stroke& stro1 = strokes[itLeft2->refEnd];
    const Stroke& stro2 = strokes[it->refBegin];
    if((it->dir == Up   && stro1.lowValue <= stro2.highValue ) || 
       (it->dir == Down && stro1.highValue>= stro2.lowValue) ) {
      // 有重叠保留，并合并
      it= rawLinesMerge(itLeft2, it);
    }else {
      // 没重叠去掉
      rawlines.pop_front();
      rawlines.pop_front();
    }
  }else if(dist >=3 ) {
    itLeft1 = it ; itLeft1-- ;
    itLeft2 = rawlines.begin();
    if(itLeft2->dir != itLeft1->dir) {
      rawlines.pop_front();
      itLeft2 = rawlines.begin();
    }
    itLeft2 = rawLinesMerge(itLeft2, itLeft1);
  }

  itLeft1 = it;
  it++;
  while(it != rawlines.end()) {
    if(it->status == 0 ) {
      // 开始寻找转折点
      if(it->breakMode != 1) it->breakMode = getBreakMod(itLeft1, it);
      itTemp = it;
      while(true) {
        itTempLeft = itTemp;
        itTemp++; if(itTemp == rawlines.end()) break;
        if(it->breakMode ==1 && rawLineExceedRawLine(*itTemp, *it)) break;  // 笔破坏后直接破底
        if(itTemp->status != 0 && itTemp->dir != itLeft1->dir) break;
      }
      if(itTemp == rawlines.end()) {
        return 0;
      }
      // if found.
      if(it->breakMode ==1) {
        // 前一段肯定结束了
        itLeft1->status = 2;
        // itRight1 = it; itRight1++;
        it = rawLinesMerge(it, itTempLeft);
      }else if(it->breakMode == 2){
        // itLeft1 可能仍然在延续
      }

    }
    itLeft1 = it;
    it++;
  }

  return 0;
}

/*
 * 将it1 到it2（包括）rawlines merge 在一起。
 * return it1，删除其他
 * */
std::list<RawLine>::iterator CZSC::rawLinesMerge(std::list<RawLine>::iterator it1,std::list<RawLine>::iterator it2) {
  assert(it1->dir == it2->dir);
  std::list<RawLine>::iterator itTemp1 ;
  std::list<RawLine>::iterator itTemp2 ;
  float comValue1 = it1->lowValue;
  float comValue2 = it1->highValue;
  itTemp1 = it1; 
  while (true) {
    itTemp1++; if(itTemp1 != rawlines.end()) break;
    if(itTemp1->highValue > it1->highValue ) {
      it1->highValue = itTemp1->highValue;
    }
    if(itTemp1->lowValue < it1->lowValue) {
      it1->lowValue = itTemp1->lowValue;
    }
    if(itTemp1 == it2 ) break;
  }
  it1->refEnd = it2->refEnd;
  itTemp1 = it1; itTemp1++;
  itTemp2 = it2; itTemp2++;
  rawlines.erase(itTemp1,itTemp2);

  return it1;
}


/*
 * 获取当前线段破坏前一线段的方式
 * */
int  CZSC::getBreakMod(std::list<RawLine>::iterator left1, std::list<RawLine>::iterator cur1){
  // if(this->rawlines.size() < 2) return 0;
  if(left1->refEnd == left1->refBegin) return 0;
  bool bFound = false;
  const Stroke& stro = this->strokes[cur1->refBegin];
  for(int j = left1->refEnd -1 ; j > left1->refBegin ;j -= 2 ) {
    // if(this->strokes[j].dir != cur1->dir) continue;
    if((cur1->dir == Down && this->strokes[j].highValue >= stro.lowValue  && this->strokes[j].highValue < stro.highValue) || 
       (cur1->dir == Up   && this->strokes[j].lowValue  <= stro.highValue && this->strokes[j].lowValue  > stro.lowValue )) {
      bFound = true;
      break;
    }
  }

  if(bFound) return 1; // 一种类型破坏：笔破坏

  return 2;
}


/*
 *判特征序列分型是否成立 仅适用于前一段对前前一段的第二种类型的破坏。
 * */
bool CZSC::isEigenFX(std::list<RawLine>::iterator left1, std::list<RawLine>::iterator cur1){
  bool bFound = false;
  float comValue;
  int j =0;
  if(cur1->dir == Down) {
    comValue  = this->strokes[cur1->refBegin].lowValue;
    for(j = left1->refEnd - 1;j > left1->refBegin;j -= 2) { //寻找左边顶分型的构成
      // if(this->strokes[j].dir != cur1->dir) continue;
      if(this->strokes[j].lowValue < comValue) {
        bFound = true;
        break;
      }else if(this->strokes[j].lowValue > comValue) {
        comValue = this->strokes[j].lowValue;
      } // TODO 相等情况的处理！
   
    }
    if(bFound == false) return false;
    // 右边的寻找
    bFound = false;
    comValue  = this->strokes[cur1->refBegin].lowValue;
    j = 0;
    for(j = cur1->refBegin+2;j<= cur1->refEnd;j += 2) {
      // if(this->strokes[j].dir != cur1->dir) continue;
      if(this->strokes[j].lowValue < comValue) {
        bFound = true;
        break;
      }else if(this->strokes[j].lowValue > comValue) {
        comValue = this->strokes[j].lowValue;
      } // TODO 相等情况的处理！
    } 
    if(bFound == false) return false;
    // 左右都找到，判断是否超越
    if (this->strokes[j].lowValue < left1->lowValue || 
        ( this->strokes[j].lowValue == left1->lowValue && vol[strokes[j].endpos] > vol[strokes[left1->refBegin].beginpos])){
      return false;
    }
    return true;
    
  }else if (cur1->dir == Up) {
    bFound = false;
    comValue  = this->strokes[cur1->refBegin].highValue;
    for(j = left1->refEnd - 1;j > left1->refBegin;j -= 2) { //寻找左边顶分型的构成
      // if(this->strokes[j].dir != cur1->dir) continue;
      if(this->strokes[j].highValue > comValue) {
        bFound = true;
        break;
      }else if(this->strokes[j].highValue < comValue) {
        comValue = this->strokes[j].lowValue;
      } // TODO 相等情况的处理！
   
    }
    if(bFound == false) return false;
    // 右边的寻找
    bFound = false;
    comValue  = this->strokes[cur1->refBegin].highValue;
    j = 0;
    for(j = cur1->refBegin+2;j<= cur1->refEnd;j += 2) {
      // if(this->strokes[j].dir != cur1->dir) continue;
      if(this->strokes[j].highValue > comValue) {
        bFound = true;
        break;
      }else if(this->strokes[j].highValue < comValue) {
        comValue = this->strokes[j].lowValue;
      } // TODO 相等情况的处理！
    } 
    if(bFound == false) return false;
    // 左右都找到，判断是否超越
    if (this->strokes[j].highValue > left1->highValue || 
        ( this->strokes[j].highValue == left1->highValue && vol[strokes[j].endpos] > vol[strokes[left1->refBegin].beginpos])){
      return false;
    }
    return true;

  }
  return false;
}

Stroke CZSC::slice2Stroke(const Slice& s){
  Stroke stro;
  stro.dir = s.FXtype == Bottom ? Up : Down ;
  stro.beginpos = s.FXpos ;
  stro.endpos   = s.endpos;
  stro.lowValue  = s.FXtype == Bottom ? s.FXvalue : low[s.endpos] ;
  stro.highValue = s.FXtype == Bottom ? high[s.endpos] : s.FXvalue;
  stro.gap = s.gap;
  return stro;
  
}

bool CZSC::strokeExceedLineSeg(const Stroke& stro,const LineSeg& line){
  if(stro.dir == Up   && higherThan(stro.endpos, line.highpos)) return true;
  if(stro.dir == Down && lowerThan (stro.endpos, line.lowpos))  return true;
  return false;
}


/*TODO 测试相等情况下的判断*/
bool CZSC::strokeExceedRawLine(const Stroke& stro,const RawLine& line){
  if(stro.dir == Up && line.dir == Up) {
    if(stro.highValue > line.highValue && stro.lowValue  > line.lowValue) {
      return true; 
    }else if(stro.highValue < line.highValue || stro.lowValue  < line.lowValue){
      return false;
    }else {
      if( (stro.highValue > line.highValue || (stro.highValue == line.highValue  && vol[stro.endpos] > vol[this->strokes[line.refEnd].endpos]) ) && 
          (line.lowValue  < stro.lowValue  || (line.lowValue  == stro.lowValue   && vol[this->strokes[line.refBegin].beginpos] > vol[stro.beginpos]))
        ) { 
        return true; 
      }
    }
  }else if(stro.dir == Down && line.dir == Down) {
    if(stro.lowValue < line.lowValue && stro.highValue < line.highValue) {
      return true; 
    }else if(stro.lowValue > line.lowValue || stro.highValue > line.highValue) {
      return false;
    }else {
      if( (stro.lowValue  < line.lowValue  || (stro.lowValue == line.lowValue   && vol[stro.endpos] > vol[this->strokes[line.refEnd].endpos])) && 
          (line.highValue > stro.highValue || (line.highValue == stro.highValue && vol[this->strokes[line.refBegin].beginpos] > vol[stro.beginpos]))
         ) {
        return true;
      }
    }
    
  }
  return false;
}

/*TODO 相等情况下的判断*/
bool CZSC::rawLineExceedRawLine(const RawLine& line1,const RawLine& line2){
  if(line1.dir == Up && line2.dir == Up) {
    if(line1.highValue > line2.highValue && line1.lowValue  > line2.lowValue){
      return true;
    }else if(line1.highValue < line2.highValue || line1.lowValue < line2.lowValue) {
      return false;
    }else {
      if( (line1.highValue > line2.highValue || (line1.highValue == line2.highValue  && 
              vol[this->strokes[line1.refEnd].endpos] > vol[this->strokes[line2.refEnd].endpos]) ) && 
          (line2.lowValue  < line1.lowValue  || (line2.lowValue  == line1.lowValue   && 
              vol[this->strokes[line2.refBegin].beginpos] > vol[this->strokes[line1.refBegin].beginpos]))
        ) { 
        return true; 
      }
    } 
  }else if(line1.dir == Down && line2.dir == Down) {
    if(line1.lowValue < line2.lowValue && line1.highValue < line2.highValue){
      return true;
    }else if( line1.lowValue > line2.lowValue || line1.highValue > line2.highValue){
      return false;
    }else {
      if( (line1.lowValue  < line2.lowValue  || (line1.lowValue == line2.lowValue   && vol[this->strokes[line1.refEnd].endpos] > vol[this->strokes[line2.refEnd].endpos])) && 
          (line2.highValue > line1.highValue || (line2.highValue == line1.highValue && vol[this->strokes[line2.refBegin].beginpos] > vol[this->strokes[line1.refBegin].beginpos]))
         ) {
        return true;
      }

    }
  }
  return false;
}

RawLine CZSC::stroke2RawLine(int i){
  Stroke stro = this->strokes[i];
  RawLine line;
  line.dir = stro.dir;
  // line.beginpos = stro.beginpos;
  // line.endpos = stro.endpos;
  line.lowValue  = stro.lowValue;
  line.highValue = stro.highValue;
  line.refBegin  = i;
  line.refEnd    = i;
  line.gap       = 0.0;
  line.status    = 0;
  line.breakMode = 0;
  return line;
  
}


LineSeg CZSC::stroke2LineSeg(int i){
  Stroke stro = this->strokes[i];
  LineSeg line;
  line.dir = stro.dir;
  line.beginpos = stro.beginpos;
  line.endpos = stro.endpos;
  line.lowValue = stro.lowValue;
  line.highValue = stro.highValue;
  line.refBegin = i;
  line.refEnd   = i;
  line.gap = 0.0 ;
  line.status    = 0;
  line.breakMode = 0;
  return line;
}


void CZSC::printSlices(){
  int i = 0;
  std::cout << "FX as below:" << std::endl;
  for (const Slice& item : this->slices) {
    i = item.FXpos;
    std::cout 
#ifdef WITHDT
      << time2str(dt[i], "%Y-%m-%d") << " " 
#endif
      << (item.canbe ? "Y" : "N") << " " 
      << std::setw(2)<< item.FXtype << " " 
      << std::setw(4)<< item.FXpos  << " "
      << std::setw(4)<< item.endpos << std::endl;
  }

}

std::list<Slice>::iterator CZSC::getFirstExceedingRight(std::list<Slice>& plist, std::list<Slice>::iterator pit){
  // 向右寻找第一次突破的地方
  std::list<Slice>::iterator it=pit;
  if(pit == plist.end()) return plist.end();
  do{
    it++; if(it == plist.end()) break;
    if(pit->FXtype == Bottom) {
      if(it->FXtype != pit->FXtype && lowerThan (it->endpos,pit->FXpos))  return it; 
      if(it->FXtype == pit->FXtype && higherThan(it->endpos,pit->endpos)) return it; 

    } else if (pit->FXtype == Top){
      if(it->FXtype != pit->FXtype && higherThan(it->endpos,pit->FXpos))  return it; 
      if(it->FXtype == pit->FXtype && lowerThan (it->endpos,pit->endpos)) return it; 
      
    }
  }while(true);
  return plist.end();
}
std::list<Slice>::iterator CZSC::getFirstExceedingLeft(std::list<Slice>& plist, std::list<Slice>::iterator pit){
  if(pit == plist.end()) return plist.end();
  if(pit == plist.begin()) return plist.end();

  std::list<Slice>::iterator it=pit;
  do{
    // if(it == plist.begin()) break;  //  or convert to reverse_iterator.
    it--;
    if(pit->FXtype == Bottom) {
      if(it->FXtype != pit->FXtype && higherThan(it->FXpos,pit->endpos))  return it; 
      if(it->FXtype == pit->FXtype && lowerThan (it->FXpos,pit->FXpos))   return it; 

    } else if (pit->FXtype == Top){
      if(it->FXtype != pit->FXtype && lowerThan (it->FXpos,pit->endpos))  return it; 
      if(it->FXtype == pit->FXtype && higherThan(it->FXpos,pit->FXpos))   return it; 
      
    }
  }while(it != plist.begin());
  
  return plist.end();
}


//  yellow TODO 重新计算分笔，但不增加 yellow

