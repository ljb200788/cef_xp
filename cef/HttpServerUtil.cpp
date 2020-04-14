#include "stdafx.h"
#include "HttpServerUtil.h"
#include "cef_form.h"
#include "WininetHttp.h"
#include <chrono>
#include <cstdio>
#include <json\reader.h>
#include <json\value.h>
#include "ylog.h"
#include <boost/thread.hpp>
#include "XMLConfigTool.h"
#include <tcpmib.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib") 

using namespace httplib;

std::string requestContent = "";

Server svr;

HWND mainHwnd = 0;

std::string dump_headers(const Headers &headers) {
	std::string s;
	char buf[BUFSIZ] = {0};

	for (auto it = headers.begin(); it != headers.end(); ++it) {
		const auto &x = *it;
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}

std::string dump_Request(const Request &request) {
	std::string s;
	char buf[BUFSIZ] = { 0 };

	for (auto it = request.params.begin(); it != request.params.end(); ++it) {
		const auto &x = *it;
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}
	return s;
}

void RequestRemoteServer()
{

	XMLConfigTool* tool = new XMLConfigTool();
	std::string urlString = tool->GetAssistantDiagnoseUrl();
	if (urlString.empty())
	{
		urlString = "http://medical.c2cloud.cn/kgms/ylkg/v1/diag_cdss/emr";
	}
	delete tool;

	if (mainHwnd > 0)
	{
		::SendMessage(mainHwnd, WM_SHOWTOASTWINDOW, NULL, NULL);
	}

	CWininetHttp netHttp;
	std::string ret = netHttp.RequestJsonInfo(urlString, Hr_Post,
		"Content-Type:application/json;charset=utf-8", requestContent);
	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(__FILE__, __LINE__, YLog::INFO, "ret", ret);
}


const static BOOL PortUsedTCP(ULONG uPort)
{
	MIB_TCPTABLE TcpTable[100];
	DWORD nSize = sizeof(TcpTable);
	if (NO_ERROR == GetTcpTable(&TcpTable[0], &nSize, TRUE))
	{
		DWORD nCount = TcpTable[0].dwNumEntries;
		if (nCount > 0)
		{
			for (DWORD i = 0; i<nCount; i++)
			{
				MIB_TCPROW TcpRow = TcpTable[0].table[i];
				DWORD temp1 = TcpRow.dwLocalPort;
				int temp2 = temp1 / 256 + (temp1 % 256) * 256;
				if (temp2 == uPort)
				{
					return TRUE;
				}
			}
		}
		return FALSE;
	}
	return FALSE;
}

CHttpServerUtil::CHttpServerUtil()
{
}


CHttpServerUtil::~CHttpServerUtil()
{
	if (svr.is_running())
	{
		svr.stop();
	}
}

string GetCompileTime()
{
	struct tm lpCompileTime;

	char arrDate[20] = {0};
	sprintf_s(arrDate, "%s", __DATE__);

	string date = arrDate;
	if (date.substr(0, 3).find("Jan") != string::npos)
	{
		lpCompileTime.tm_mon = 1;
	}
	else if (date.substr(0, 3).find("Feb") != string::npos)
	{
		lpCompileTime.tm_mon = 2;
	}
	else if (date.substr(0, 3).find("Mar") != string::npos)
	{
		lpCompileTime.tm_mon = 3;
	}
	else if (date.substr(0, 3).find("Apr") != string::npos)
	{
		lpCompileTime.tm_mon = 4;
	}
	else if (date.substr(0, 3).find("May") != string::npos)
	{
		lpCompileTime.tm_mon = 5;
	}
	else if (date.substr(0, 3).find("Jun") != string::npos)
	{
		lpCompileTime.tm_mon = 6;
	}
	else if (date.substr(0, 3).find("Jul") != string::npos)
	{
		lpCompileTime.tm_mon = 7;
	}
	else if (date.substr(0, 3).find("Aug") != string::npos)
	{
		lpCompileTime.tm_mon = 8;
	}
	else if (date.substr(0, 3).find("Sep") != string::npos)
	{
		lpCompileTime.tm_mon = 9;
	}
	else if (date.substr(0, 3).find("Oct") != string::npos)
	{
		lpCompileTime.tm_mon = 10;
	}
	else if (date.substr(0, 3).find("Nov") != string::npos)
	{
		lpCompileTime.tm_mon = 11;
	}
	else if (date.substr(0, 3).find("Dec") != string::npos)
	{
		lpCompileTime.tm_mon = 12;
	}

	lpCompileTime.tm_mday = atoi(date.substr(4, 2).c_str());
	lpCompileTime.tm_year = atoi(date.substr(7, 4).c_str());

	sscanf_s(__TIME__, "%d:%d:%d", &lpCompileTime.tm_hour,&lpCompileTime.tm_min, &lpCompileTime.tm_sec);
	lpCompileTime.tm_isdst = lpCompileTime.tm_wday = lpCompileTime.tm_yday = 0;

	char input[200] = { 0 };
	sprintf_s(input, "%04d-%02d-%02d %02d:%02d:%02d", lpCompileTime.tm_year, lpCompileTime.tm_mon,
		lpCompileTime.tm_mday, lpCompileTime.tm_hour, lpCompileTime.tm_min, lpCompileTime.tm_sec);
	string compileTimeStr = input;

	return compileTimeStr;
}

bool  CHttpServerUtil::StartServer()
{
	mainHwnd = m_MainHwnd;

	if (!svr.is_valid()) 
	{
		printf("server has an error...\n");
		return false;
	}

	svr.Get("/", [=](const Request & /*req*/, Response &res) {
	});

	svr.Get("/userId", [=](const Request & /*req*/, Response &res) {


	});

	svr.Get("/version", [=](const Request & /*req*/, Response &res) {

	});

	svr.Get("/token", [=](const Request & /*req*/, Response &res) {


	});

	svr.Get("/slow", [](const Request & /*req*/, Response &res) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		res.set_content("Slow...\n", "text/plain");
	});

	svr.Get("/request", [](const Request &req, Response &res) {
		res.set_content(dump_Request(req), "text/plain");
	});

	svr.Post("/requestDiagnostic", [](const Request &req, Response &res) {


	});

	svr.Get("/dump", [](const Request &req, Response &res) {
		res.set_content(dump_headers(req.headers), "text/plain");
	});

	svr.Get("/stop",[&](const Request & /*req*/, Response & /*res*/) { svr.stop(); });

	svr.set_error_handler([](const Request & /*req*/, Response &res) {
		const char *fmt = "<p>Error Code: <span style='color:red;'>%d</span></p>";
		char buf[BUFSIZ];
		snprintf(buf, sizeof(buf), fmt, res.status);
		res.set_content(buf, "text/html");
	});

	svr.set_logger([](const Request &req, const Response &res) {
		YLog log(YLog::INFO, "log.txt", YLog::ADD);
		log.W(__FILE__, __LINE__, YLog::DEBUG, "set_logger_method", req.method);
		log.W(__FILE__, __LINE__, YLog::DEBUG, "set_logger_path", req.path);
		log.W(__FILE__, __LINE__, YLog::DEBUG, "set_logger_param", dump_Request(req));
		log.W(__FILE__, __LINE__, YLog::DEBUG, "set_logger_body", req.body);
		log.W(__FILE__, __LINE__, YLog::DEBUG, "set_logger_headers", dump_headers(req.headers));
	});

	svr.set_base_dir("./resources");
	//svr.set_base_dir("./cef/html");
	//svr.set_base_dir("./cef", "/mount");

	XMLConfigTool* tool = new XMLConfigTool();
	unsigned int port = tool->GetLocalServerPort();
	if (port <= 0)
	{
		port = 8080;
	}
	delete tool;

	bool state = false;
	if (!PortUsedTCP(port))
	{
		state = svr.listen("127.0.0.1", port);
	}
	return state;
}
