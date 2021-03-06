#include "stdafx.h"
#include "HttpServerUtil.h"
#include "cef_form.h"
#include "login_form.h"
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


char* GetLocalPogramVersion()
{
	TCHAR pFilePath[MAX_PATH] = { 0 };
	DWORD dwRet = GetModuleFileName(NULL, pFilePath, MAX_PATH);
	if (dwRet == 0)
	{
		return "";
	}

	//获取版本信息大小
	DWORD dwSize = GetFileVersionInfoSize(pFilePath, NULL);
	if (dwSize == 0)
	{
		return "";
	}

	TCHAR *pBuf = (TCHAR *)malloc(dwSize + 1);
	memset(pBuf, 0, dwSize + 1);

	//获取版本信息
	DWORD dwRtn = GetFileVersionInfo(pFilePath, NULL, dwSize, pBuf);
	if (dwRtn == 0)
	{
		return "";
	}

	LPVOID lpBuffer = NULL;
	UINT uLen = 0;

	dwRtn = VerQueryValue(pBuf, TEXT("\\StringFileInfo\\080404b0\\FileVersion"), &lpBuffer, &uLen);
	if (dwRtn == 0)
	{
		delete pBuf;
		return "";
	}

	int iLen = 2 * wcslen((TCHAR*)lpBuffer);//CString,TCHAR汉字算一个字符，因此不用普通计算长度   
	char* chRtn = (char *)malloc(iLen + 1);
	memset(chRtn, 0, iLen + 1);
	size_t  sLen;
	wcstombs_s(&sLen, chRtn, iLen + 1, (TCHAR*)lpBuffer, iLen + 1);//转换成功返回为非负值

	char* strVersion = (char *)malloc(iLen + 1);
	memset(strVersion, 0, iLen + 1);
	strcpy_s(strVersion, iLen + 1, chRtn);
	delete pBuf;

	return strVersion;
}
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

	string cdssToken = "";
	if (!LoginForm::user_token.empty())
	{
		cdssToken = "CDSSToken:" + LoginForm::user_token;
	}

	CWininetHttp netHttp;
	std::string ret = netHttp.RequestJsonInfo(urlString, Hr_Post, cdssToken + "\r\nContent-Type:application/json;charset=UTF-8\r\n", requestContent);
	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(__FILE__, __LINE__, YLog::INFO, "ret", ret);
}

/*
*
*/

static const char *
inet_ntop_v4(const void *src, char *dst, size_t size)
{
	const char digits[] = "0123456789";
	int i;
	struct in_addr *addr = (struct in_addr *)src;
	u_long a = ntohl(addr->s_addr);
	const char *orig_dst = dst;

	if (size < INET_ADDRSTRLEN) {
		errno = ENOSPC;
		return NULL;
	}
	for (i = 0; i < 4; ++i) {
		int n = (a >> (24 - i * 8)) & 0xFF;
		int non_zerop = 0;

		if (non_zerop || n / 100 > 0) {
			*dst++ = digits[n / 100];
			n %= 100;
			non_zerop = 1;
		}
		if (non_zerop || n / 10 > 0) {
			*dst++ = digits[n / 10];
			n %= 10;
			non_zerop = 1;
		}
		*dst++ = digits[n];
		if (i != 3)
			*dst++ = '.';
	}
	*dst++ = '\0';
	return orig_dst;
}

static const char *
inet_ntop_v6(const void *src, char *dst, size_t size)
{
	const char xdigits[] = "0123456789abcdef";
	int i;
	const struct in6_addr *addr = (struct in6_addr *)src;
	const u_char *ptr = addr->s6_addr;
	const char *orig_dst = dst;
	int compressed = 0;

	if (size < INET6_ADDRSTRLEN) {
		errno = ENOSPC;
		return NULL;
	}
	for (i = 0; i < 8; ++i) {
		int non_zerop = 0;

		if (compressed == 0 &&
			ptr[0] == 0 && ptr[1] == 0 &&
			i <= 5 &&
			ptr[2] == 0 && ptr[3] == 0 &&
			ptr[4] == 0 && ptr[5] == 0) {

			compressed = 1;

			if (i == 0)
				*dst++ = ':';
			*dst++ = ':';

			for (ptr += 6, i += 3;
				i < 8 && ptr[0] == 0 && ptr[1] == 0;
				++i, ptr += 2);

			if (i >= 8)
				break;
		}

		if (non_zerop || (ptr[0] >> 4)) {
			*dst++ = xdigits[ptr[0] >> 4];
			non_zerop = 1;
		}
		if (non_zerop || (ptr[0] & 0x0F)) {
			*dst++ = xdigits[ptr[0] & 0x0F];
			non_zerop = 1;
		}
		if (non_zerop || (ptr[1] >> 4)) {
			*dst++ = xdigits[ptr[1] >> 4];
			non_zerop = 1;
		}
		*dst++ = xdigits[ptr[1] & 0x0F];
		if (i != 7)
			*dst++ = ':';
		ptr += 2;
	}
	*dst++ = '\0';
	return orig_dst;
}

const char * inetntop(int af, const void *src, char *dst, size_t size)
{
	switch (af) {
	case AF_INET:
		return inet_ntop_v4(src, dst, size);
	case AF_INET6:
		return inet_ntop_v6(src, dst, size);
	default:
		errno = EAFNOSUPPORT;
		return NULL;
	}
}

std::string isPortOpen(const std::string &domain, const std::string &port)
{
	addrinfo *result;
	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;   // either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	char addressString[INET6_ADDRSTRLEN];
	const char *retval = nullptr;
	if (0 != getaddrinfo(domain.c_str(), port.c_str(), &hints, &result)) {
		return "";
	}
	for (addrinfo *addr = result; addr != nullptr; addr = addr->ai_next) {
		int handle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (handle == -1) {
			continue;
		}
		if (connect(handle, addr->ai_addr, addr->ai_addrlen) != -1)
		{
			switch (addr->ai_family) {
			case AF_INET:
				retval = inetntop(addr->ai_family, &(reinterpret_cast<sockaddr_in *>(addr->ai_addr)->sin_addr), addressString, INET6_ADDRSTRLEN);
				break;
			case AF_INET6:
				retval = inetntop(addr->ai_family, &(reinterpret_cast<sockaddr_in6 *>(addr->ai_addr)->sin6_addr), addressString, INET6_ADDRSTRLEN);
				break;
			default:
				// unknown family
				retval = nullptr;
			}
			//_close(handle);
			break;
		}
	}
	if (result)
	{
		freeaddrinfo(result);
	}

	return retval == nullptr ? "" : domain + ":" + retval + "\n";
}

//如果返回true，则说明，某个服务处于监听状态，同时说明这个端口号已经被这个服务所占用，
//不能再被其他服务所用
bool isListening(const std::string &ip, const std::string &port) {
	std::string addr = isPortOpen(ip, port);
	if (addr.empty()) {
		return false;
	}
	return true;
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

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	if (!svr.is_valid()) 
	{
		printf("server has an error...\n");
		log.W(__FILE__, __LINE__, YLog::DEBUG, "StartServer", "server has an error");
		return false;
	}

	svr.Get("/", [=](const Request & /*req*/, Response &res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		//res.set_content(shared::tools::UtfToString("欢迎您使用辅助诊断助手服务！"), "text/plain");
		res.set_content("欢迎您使用辅助诊断助手服务！", "text/plain");
	});

	svr.Get("/userId", [=](const Request & /*req*/, Response &res) {
		Json::Value result;
		result["user_id"] = CefForm::strUserName.c_str();
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_content(result.toStyledString(), "application/json");

	});

	svr.Get("/version", [=](const Request & /*req*/, Response &res) {
		res.set_header("Access-Control-Allow-Origin", "*");

		Json::Value result;
		result["app_version"] = versionNum;
		result["app_time"] = GetCompileTime();

		string version = GetLocalPogramVersion();
		result["file_version"] = version;
		result["app_name"] = shared::tools::UtfToString("辅助诊断助手（湘雅附一）");

		res.set_content(result.toStyledString(), "application/json");

	});

	svr.Get("/token", [=](const Request & /*req*/, Response &res) {
		Json::Value result;
		result["user_token"] = LoginForm::user_token.c_str();
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_content(result.toStyledString(), "application/json");

	});

	svr.Get("/slow", [](const Request & /*req*/, Response &res) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		res.set_content("Slow...\n", "text/plain");
	});

	//svr.Get("/request", [](const Request &req, Response &res) {
	//	res.set_content(dump_Request(req), "text/plain");
	//});

	svr.Post("/request", [](const Request &req, Response &res) {
		YLog log(YLog::INFO, "log.txt", YLog::ADD);

		res.set_header("Access-Control-Allow-Origin", "*");

		if (req.body.empty())
		{
			Json::Value result;

			result["error_code"] = 1;
			result["message"] = shared::tools::UtfToString("数据内容为空！");
			res.set_content(result.toStyledString(), "application/json");
		}
		else
		{
			Json::Reader reader;
			Json::Value root;

			// 使用boost库解析json
			if (reader.parse(req.body, root))
			{
				if (root.isNull())
				{
					Json::Value result;

					result["error_code"] = 1;
					result["message"] = shared::tools::UtfToString("数据为空！");
					res.set_content(result.toStyledString(), "application/json");
				}
				else
				{
					if (root.isObject())
					{

						root["client"] = CefForm::strUserName.c_str();

						log.W(__FILE__, __LINE__, YLog::DEBUG, "request", root.toStyledString());

						requestContent = root.toStyledString();

						XMLConfigTool* tool = new XMLConfigTool();
						std::string urlString = tool->GetKnowledgeQueryUrl();
						if (urlString.empty())
						{
							urlString = "http://medical.c2cloud.cn/kgms/ylkg/v1/diag_cdss/emr";
						}
						delete tool;

						bool wait = false;
						if (root.isMember("wait"))
						{
							if (root["wait"].isBool())
							{
								wait = root["wait"].asBool();
							}
						}

						if (mainHwnd > 0 && wait)
						{
							::SendMessage(mainHwnd, WM_SHOWQUERYWINDOW, NULL, NULL);
						}
						

						string cdssToken = "";
						if (!LoginForm::user_token.empty())
						{
							cdssToken = "CDSSToken:" + LoginForm::user_token;
						}

						CWininetHttp netHttp;
						std::string ret = netHttp.RequestJsonInfo(urlString, Hr_Post, cdssToken + "\r\nContent-Type:application/json;charset=UTF-8\r\n", requestContent);
						YLog log(YLog::INFO, "log.txt", YLog::ADD);
						log.W(__FILE__, __LINE__, YLog::INFO, "ret", ret);

						Json::Value result;
						if (ret.empty())
						{
							result["result_code"] = 1;
							result["message"] = shared::tools::UtfToString("失败！");
							res.set_content(result.toStyledString(), "application/json");
						}
						else
						{
							//result["result_code"] = 0;
							//result["message"] = shared::tools::UtfToString("成功！");

							Json::Reader reader_result;
							Json::Value root_result;
							if (reader_result.parse(ret, root_result))
							{
								//result["content"] = root_result;
								res.set_content(root_result.toStyledString(), "application/json");
							}
						}
					

						
					}
					else
					{
						Json::Value result;

						result["error_code"] = 2;
						result["message"] = shared::tools::UtfToString("数据格式错误！");
						res.set_content(result.toStyledString(), "application/json");
					}

				}
			}
			else
			{
				Json::Value result;

				result["error_code"] = 2;
				result["message"] = shared::tools::UtfToString("数据格式错误！");
				res.set_content(result.toStyledString(), "application/json");
			}
		}
	});

	svr.Post("/requestDiagnostic", [](const Request &req, Response &res) {

		YLog log(YLog::INFO, "log.txt", YLog::ADD);
		res.set_header("Access-Control-Allow-Origin", "*");
		if (req.body.empty())
		{
			Json::Value result;

			result["error_code"] = 1;
			result["message"] = shared::tools::UtfToString("数据内容为空！");
			res.set_content(result.toStyledString(), "application/json");
		}
		else
		{
			Json::Reader reader;
			Json::Value root;

			// 使用boost库解析json
			if (reader.parse(req.body, root))
			{
				Json::Value config;
				config["client"] = CefForm::strUserName.c_str();

				if (root.isNull())
				{
					root = config;
				}
				else
				{
					if (root.isObject())
					{

						Json::Value result;

						result["config"] = config;
						result["emr"] = root;

						log.W(__FILE__, __LINE__, YLog::DEBUG, "requestDiagnostic", result.toStyledString());

						res.set_content(result.toStyledString(), "application/json");

						requestContent = result.toStyledString();

						boost::thread requestThread(&RequestRemoteServer);
						requestThread.detach();
					}
					else
					{
						root = config;
						Json::Value result;

						result["error_code"] = 2;
						result["message"] = shared::tools::UtfToString("数据格式错误！");
						res.set_content(result.toStyledString(), "application/json");
					}
					
				}
			}
			else
			{
				Json::Value result;

				result["error_code"] = 2;
				result["message"] = shared::tools::UtfToString("数据格式错误！");
				res.set_content(result.toStyledString(), "application/json");
			}
		}

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

	XMLConfigTool* tool = new XMLConfigTool();
	unsigned int port = tool->GetLocalServerPort();
	if (port <= 0)
	{
		port = 8080;
	}
	delete tool;

	bool state = false;
	//if (!PortUsedTCP(port))
	if (!isListening("127.0.0.1", std::to_string(port)))
	{
		state = svr.listen("localhost", port);
		if (state)
		{
			log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("启动Http服务"), shared::tools::UtfToString("监听服务启动成功!"));
		}
		else
		{
			log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("启动Http服务"), shared::tools::UtfToString("监听服务启动失败!"));
		}
	}
	else
	{
		log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("启动Http服务"), shared::tools::UtfToString("服务端口已被占用!"));
	}

	return state;
}
