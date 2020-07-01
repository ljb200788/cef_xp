// basic.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "main.h"
#include "basic_form.h"
#include "login_form.h"
#include "ylog.h"
#include "WininetHttp.h"
#include "XMLConfigTool.h"
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <rapidjson\writer.h>
#include <rapidjson\stringbuffer.h>
#include "version_form.h"
#include "rest_form.h"
#include "crashdump.h"
#pragma comment(lib, "dbghelp.lib")

using namespace rapidjson;
using namespace std;

CrashDump dump;
void* m_pBaseMapFile = NULL;

extern "C" LSTATUS MyRegGetValueA(HKEY hkey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
{
	typedef LSTATUS(WINAPI *REGGETVALUEA)(HKEY hkey, LPCSTR lpSubKey, LPCSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
	REGGETVALUEA LoadLibAddy;
	LoadLibAddy = (REGGETVALUEA)GetProcAddress(GetModuleHandle(L"advapi32.dll"), "RegGetValueA");
	DWORD type;

	if (LoadLibAddy)
	{
		return LoadLibAddy(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
	}
	else
	{
		if (dwFlags == RRF_RT_REG_SZ)
		{
			type = REG_SZ;
		}
		else if (dwFlags == RRF_RT_REG_DWORD)
		{
			type = REG_DWORD;
		}
		return RegQueryValueExA(hkey, lpValue, NULL, &type, (LPBYTE)pvData, pcbData);
	}
}

extern "C" LSTATUS MyRegGetValueW(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
{
	typedef LSTATUS(WINAPI *REGGETVALUEW)(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValue, DWORD dwFlags, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
	REGGETVALUEW LoadLibAddy;
	LoadLibAddy = (REGGETVALUEW)GetProcAddress(GetModuleHandle(L"advapi32.dll"), "RegGetValueW");
	DWORD type;


	if (LoadLibAddy)
	{
		return LoadLibAddy(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
	}
	else
	{
		if (dwFlags == RRF_RT_REG_SZ)
		{
			type = REG_SZ;
		}
		else if (dwFlags == RRF_RT_REG_DWORD)
		{
			type = REG_DWORD;
		}
		return RegQueryValueExW(hkey, lpValue, NULL, &type, (LPBYTE)pvData, pcbData);
	}
}

bool checkExeRun()
{
	HANDLE m_hMapFile = OpenFileMapping(FILE_MAP_WRITE, FALSE, _T("DiagnosticAssistant"));
	if (m_hMapFile)
	{
		m_pBaseMapFile = MapViewOfFile(m_hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		size_t position = 0;
		char read_chars[1024] = { 0 };

		//读数据
		memcpy(read_chars, m_pBaseMapFile, 1024);

		YLog log(YLog::INFO, "log.txt", YLog::ADD);
		log.W(filename(__FILE__), __LINE__, YLog::ERR, "program_state", read_chars);

		MessageBox(NULL, _T("已运行该程序！"), _T("提示"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);

		return true;
	}
	else
	{
		HANDLE m_hMapFile = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 0x1000, _T("DiagnosticAssistant"));
		m_pBaseMapFile = MapViewOfFile(m_hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

		char write_chars[] = "DiagnosticAssistant";
		const size_t write_chars_size = sizeof(write_chars);
		memcpy(m_pBaseMapFile, write_chars, write_chars_size);

		return false;
	}


}

bool CheckUpdate()
{
	XMLConfigTool* tool = new XMLConfigTool();
	std::string g_update_url = tool->GetUpdateUrl();
	delete tool;

	if (g_update_url.empty())
	{
		return false;
	}

	std::string url = g_update_url + "?appCode=2&versionNum=" + versionNum;

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	//log.W(__FILE__, __LINE__, YLog::INFO, "updateUrl", url);

	CWininetHttp netHttp;
	std::string ret = netHttp.RequestJsonInfo(url, Hr_Post, "Content-Type:application/json;charset=utf-8", "{}");

	if (!ret.empty())
	{
		//log.W(__FILE__, __LINE__, YLog::INFO, "Result", ret);

		rapidjson::Document doc;
		doc.Parse(ret.c_str());

		if (!doc.IsNull())
		{
			if (doc.HasMember("needUpdate"))
			{
				Value& m = doc["needUpdate"];
				if (m.IsBool())
				{
					if (m.GetBool())
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}


void AddAutoStart()
{
	HKEY hKey;
	std::string strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	try
	{
		//1、找到系统的启动项  
		if (RegOpenKeyEx(HKEY_CURRENT_USER, nbase::UTF8ToUTF16(strRegPath).c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) ///打开启动项       
		{
			//2、得到本程序自身的全路径
			//TCHAR strExeFullDir[MAX_PATH];
			//GetModuleFileName(NULL, strExeFullDir, MAX_PATH);

			std::wstring exe_dir = QPath::GetAppPath();
			exe_dir += L"DiagnosticAssistant.exe";

			//3、判断注册表项是否已经存在
			TCHAR strDir[MAX_PATH] = {};
			DWORD nLength = MAX_PATH;
			long result = MyRegGetValueW(hKey, nullptr, nbase::UTF8ToUTF16("DiagnosticAssistantStart").c_str(), RRF_RT_REG_SZ, 0, strDir, &nLength);


			//4、不存在
			if (result != ERROR_SUCCESS || _tcscmp(exe_dir.c_str(), strDir) != 0)
			{
				//5、添加一个子Key,并设置值，"GISRestart"是应用程序名字（不加后缀.exe） 
				RegSetValueEx(hKey, nbase::UTF8ToUTF16("DiagnosticAssistantStart").c_str(), 0, REG_SZ, (LPBYTE)exe_dir.c_str(),
					(lstrlen(exe_dir.c_str()) + 1) * sizeof(TCHAR));
				//6、关闭注册表
				RegCloseKey(hKey);
			}
		}
		else
		{
			std::cout << "警告\n系统参数错误,不能随系统启动" << std::endl;
		}
	}
	catch (std::exception e)
	{
	}
}

void CancelAutoStart()
{
	HKEY hKey;
	std::string strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	try
	{
		//1、找到系统的启动项  
		if (RegOpenKeyEx(HKEY_CURRENT_USER, nbase::UTF8ToUTF16(strRegPath).c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
		{
			//2、删除值
			RegDeleteValue(hKey, nbase::UTF8ToUTF16("DiagnosticAssistantStart").c_str());
			//2、删除值
			RegDeleteValue(hKey, nbase::UTF8ToUTF16("DiagnosticAssistant").c_str());

			//3、关闭注册表
			RegCloseKey(hKey);
		}
	}
	catch (std::exception e)
	{
	}
}

void  CheckAutoStart()
{
	XMLConfigTool* tool = new XMLConfigTool();
	bool autoStartState = tool->GetAutoStartConfig();
	delete tool;

	if (autoStartState)
	{
		AddAutoStart();
	}
	else
	{
		CancelAutoStart();
	}
}
/*
设置兼容模式运行
*/
void SetCompatibleMode()
{
	HKEY hKey;
	std::string strSubKey = "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers";
	long lRet = ::RegOpenKeyEx(HKEY_CURRENT_USER, nbase::UTF8ToUTF16(strSubKey).c_str(), 0, KEY_WRITE, &hKey);
	if (lRet == ERROR_SUCCESS)
	{
		TCHAR achValue[] = { _T("WIN7RTM") };
		TCHAR strExeFullDir[MAX_PATH];
		GetModuleFileName(NULL, strExeFullDir, MAX_PATH);

		lRet = ::RegSetValueEx(hKey, strExeFullDir, NULL, REG_SZ, (LPBYTE)&achValue, sizeof(achValue));
		RegCloseKey(hKey);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// 将 bin\\cef 目录添加到环境变量，这样可以将所有 CEF 相关文件放到该目录下，方便管理
	// 在项目属性->连接器->输入，延迟加载 nim_libcef.dll
	nim_cef::CefManager::GetInstance()->AddCefDllToPath();

	HRESULT hr = ::OleInitialize(NULL);
	if (FAILED(hr))
		return 0;

	// 初始化 CEF
	CefSettings settings;
	if (!nim_cef::CefManager::GetInstance()->Initialize(nbase::win32::GetCurrentModuleDirectory() + L"cef\\", settings, kEnableOffsetRender))
	{
		return 0;
	}

	// 创建主线程
	MainThread thread;

	// 执行主线程循环
	thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);

    // 清理 CEF
    nim_cef::CefManager::GetInstance()->UnInitialize();

	::OleUninitialize();

	return 0;
}

static string  getCurrentTimeStr()
{
	struct tm t;   //tm结构指针
	time_t now;  //声明time_t类型变量
	time(&now);

	localtime_s(&t, &now);
	char ch[64] = { 0 };
	strftime(ch, sizeof(ch) - 1, "%Y-%m-%d %H:%M:%S", &t);     //年-月-日 时-分-秒
	return ch;
}

void MainThread::Init()
{

	std::wstring theme_dir = QPath::GetAppPath();

	SetCurrentDirectory(theme_dir.c_str());


	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	if (checkExeRun())
	{
		PostQuitMessage(0L);
		return;
	}

	CheckAutoStart();

	nbase::ThreadManager::RegisterThread(kThreadMain);

	// 获取资源路径，初始化全局参数
	// 默认皮肤使用 resources\\themes\\default
	// 默认语言使用 resources\\lang\\zh_CN
	// 如需修改请指定 Startup 最后两个参数
	//std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();
	ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), false);

	log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("启动程序"), getCurrentTimeStr());

	//SetCompatibleMode();

	if (CheckUpdate())
	{
		VersionForm* window = new VersionForm();
		window->Create(NULL, VersionForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
		window->CenterWindow();
		window->ShowWindow();
	}
	else
	{
		XMLConfigTool* tool = new XMLConfigTool();
		bool needLogin = tool->GetNeedLoginConfig();
		bool autoLogin = tool->GetAutoLoginConfig();
		delete tool;
		if (needLogin)
		{
			if (GetLoginInfo() > 0 && autoLogin)
			{
				BasicForm* window = new BasicForm();
				window->Create(NULL, BasicForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
				window->ShowWindow();
			}
			else
			{
				LoginForm* window = new LoginForm();
				window->Create(NULL, LoginForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
				window->ShowWindow();
			}

		}
		else
		{
			LoginForm::user_name = "huayuan";
			BasicForm* window = new BasicForm();
			window->Create(NULL, BasicForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
			window->ShowWindow();
		}
	}

	// 创建一个默认带有阴影的居中窗口
	//RestForm* window = new RestForm();
	//window->Create(NULL, RestForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
	//window->CenterWindow();
	//window->ShowWindow();
}

void MainThread::Cleanup()
{
	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("退出程序"), getCurrentTimeStr());

	ui::GlobalManager::Shutdown();
	SetThreadWasQuitProperly(true);
	nbase::ThreadManager::UnregisterThread();

	if (m_pBaseMapFile)
	{
		UnmapViewOfFile(m_pBaseMapFile);
	}
}
