#ifndef YLOG_YLOG_H_
#define YLOG_YLOG_H_

#include <string>
#include <fstream>
#include <cassert>
#include <ctime>
#include "XMLConfigTool.h"

#define filename(x) strrchr(x,'\\')?strrchr(x,'\\')+1:x

class YLog{
 private:
  std::ofstream of_;
  int minlevel_;
 public:
  enum Type {
    ADD = 0,
    OVER
  };
  enum Level {
    INFO  = 0,
	DEBUG = 1,
    ERR
  };
  YLog(const int level, const std::string &logfile, const int type = YLog::OVER) : minlevel_(level) {
    assert((this->ERR == level || this->INFO == level || || this->DEBUG == level) && "Logfile create failed, please check the level(YLog::ERR or YLog::INFO.");
    if (type == this->ADD) {
      this->of_.open(logfile.c_str(),std::ios_base::out|std::ios_base::app);
    } else if (type == this->OVER) {
      this->of_.open(logfile.c_str(),std::ios_base::out|std::ios_base::trunc);
    } else {
      assert(0 && "Logfile create failed, please check the type(YLog::OVER or YLog::ADD).");
    }
    assert(this->of_.is_open() && "Logfile create failed, please check the logfile's name and path.");
    return;
  }
  ~YLog(){
    if (this->of_.is_open()) {
      this->of_.close();
    }
    return;
  }
  template<typename T> void W(const std::string &codefile, const int codeline, const int level, const std::string &info, const T &value) {
    assert(this->of_.is_open() && "Logfile write failed.");
    if (this->ERR == level) 
	{
		this->of_ << "[ERROR] ";
    }
	else if (this->DEBUG == level)
	{
		XMLConfigTool* tool = new XMLConfigTool();
		if (!tool->GetLogDebugConfig())
		{
			delete tool;
			return;
		}
		this->of_ << "[DEBUG] ";
		delete tool;
	}
	else if (this->INFO == level) 
	{
		if (this->INFO == this->minlevel_) {
			this->of_ << "[INFO] ";
		 } 
		else 
		{
			return;
		}
    } 
	else 
	{
		assert(0 && "Log write failed, please check the level(YLog::ERR or YLog::INFO.");
    }
    time_t sectime = time(NULL);
    tm tmtime;
#ifdef _WIN32
#if _MSC_VER<1600
    tmtime = *localtime(&sectime);
#else
    localtime_s(&tmtime, &sectime);
#endif
#else
    localtime_r(&sectime, &tmtime);
#endif

	char data[256] = { 0 };
	sprintf_s(data, "%04d-%02d-%02d %02d:%02d:%02d", tmtime.tm_year + 1900, tmtime.tm_mon + 1, tmtime.tm_mday, tmtime.tm_hour, tmtime.tm_min, tmtime.tm_sec);
	this->of_ << data << ' ' << codefile << '(' << codeline << ") " << info << ":\n" << value << std::endl;

    /*this->of_ << tmtime.tm_year+1900 << '-' << tmtime.tm_mon+1 << '-' << tmtime.tm_mday <<
      ' ' << tmtime.tm_hour << ':' << tmtime.tm_min << ':' << tmtime.tm_sec <<
      ' '<< codefile << '(' << codeline << ") " << info << ":\n" << value << std::endl;*/
    return;
  }

  template<typename T> void V(const std::string &codefile, const int codeline, const int level, const std::string &info, const T &value) {
	  assert(this->of_.is_open() && "Logfile write failed.");
	  if (this->ERR == level) {
		  this->of_ << "[ERROR] ";
	  }
	  else if (this->INFO == level) {
		  if (this->INFO == this->minlevel_) {
			  this->of_ << "[INFO] ";
		  }
		  else {
			  return;
		  }
	  }
	  else {
		  assert(0 && "Log write failed, please check the level(YLog::ERR or YLog::INFO.");
	  }

	  this->of_ << value << std::endl;

	  return;
  }
};
#endif // YLOG_YLOG_H_ 
