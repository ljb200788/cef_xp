﻿#include "stdafx.h"
#include "basic_form.h"
#include "XMLConfigTool.h"
#include "ylog.h"
#include <json\value.h>
#include <json\reader.h>
#include "tool.h"
#include "util.h"
#include "WininetHttp.h"
#include <boost/thread.hpp>
#include <Netlistmgr.h>
#include "HttpServerUtil.h"
#include "result_form.h"
#include <tlhelp32.h>
#include "resource.h"
#include "login_form.h"

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "version.lib")

using namespace std;

const std::wstring BasicForm::kClassName = L"Basic";

XMLConfigTool* tool = new XMLConfigTool();

ResultForm* result_form = NULL;
HWND toastHwnd = 0;
bool  isUserOnline = false;

//是否找到门诊病历窗口
bool  illHisWndFindState = false;

//是否找到住院病历窗口
bool  hospitalWndFindState = false;

//门诊病历窗口句柄
HWND  illHisWnd = 0;
HWND  mainHisWnd = 0;
HWND  mainHisParentWnd = 0;
HWND  prescribeWnd = 0;
HWND  rwWnd = 0;

//人卫查询结果句柄
HWND  rwResultWnd = 0;

// 获取屏幕大小
int iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
int iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

//托盘菜单
//win32程序使用的是HMENU，如果是MFC程序可以使用CMenu
HMENU hMenu;

Json::Value g_value;

//定义托盘图标对象
NOTIFYICONDATA m_trayIcon;

volatile bool isTreadState = true;
volatile bool isTreadMessageState = true;
volatile bool isTreadRWKnowledgeState = true;
volatile bool isTreadRecordState = true;

bool  isVKSpacePressState = false;
bool  isNetOfficeWndFind = false;

int hotkeyId8 = 1;
int hotkeyId7 = 2;
int hotkeyId6 = 3;
int hotkeyId5 = 4;
int hotkeyId4 = 5;
int hotkeyId3 = 6;
int hotkeyId2 = 7;
int hotkeyId1 = 8;

CHttpServerUtil* serverUtil = new CHttpServerUtil();

std::map<string, HWND> windowMap;

string paraString = "";
string urlString = "";

//主诉
string  ocrMainSuit = "";
//现病史
string  ocrIllnessHis = "";
//过敏史
string  ocrAllergyHis = "";
//家庭史
string  ocrFamilyHis = "";
//既往史
string  ocrPastHis = "";
//个人史
string  ocrPersonalHis = "";


BOOL IsNetConnected()
{
	bool needCheckInternet = tool->GetNeedCheckInternetConfig();

	if (!needCheckInternet)
	{
		return TRUE;
	}

	CoInitialize(NULL);
	//  通过NLA接口获取网络状态
	IUnknown *pUnknown = NULL;
	BOOL	bOnline = TRUE;//是否在线  
	HRESULT Result = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL, IID_IUnknown, (void **)&pUnknown);
	if (SUCCEEDED(Result))
	{
		INetworkListManager *pNetworkListManager = NULL;
		if (pUnknown)
			Result = pUnknown->QueryInterface(IID_INetworkListManager, (void **)&pNetworkListManager);
		if (SUCCEEDED(Result))
		{
			VARIANT_BOOL IsConnect = VARIANT_FALSE;
			if (pNetworkListManager)
				Result = pNetworkListManager->get_IsConnectedToInternet(&IsConnect);
			if (SUCCEEDED(Result))
			{
				bOnline = (IsConnect == VARIANT_TRUE) ? true : false;
			}
		}
		if (pNetworkListManager)
			pNetworkListManager->Release();
	}
	if (pUnknown)
		pUnknown->Release();
	CoUninitialize();
	return bOnline;
}

void RunHttpServer()
{

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	if (serverUtil->StartServer())
	{
		//log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("启动Http服务"), shared::tools::UtfToString("启动成功"));
	}
	else
	{
		log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("启动Http服务"), shared::tools::UtfToString("启动失败,端口已被占用"));
	}

	if (serverUtil)
	{
		delete serverUtil;
		serverUtil = NULL;
	}
}


void ReceiveMessage(void*& data)
{

	int i = 0;
	bool  isVisible = false;
	while (isTreadMessageState)
	{
		try
		{
			boost::this_thread::sleep(boost::posix_time::microseconds(200));
			BasicForm* ptrForm = (BasicForm*)data;
			if (ptrForm != NULL)
			{

				if (isVisible)
				{
					if (ptrForm->JudgeCursorOut())
					{
						ptrForm->AutoHiddenWindow(80);
					}
					else
					{
						ptrForm->AutoShowWindow();
					}
				}

				if (i >= 10)
				{
					if (IsWindowVisible(ptrForm->GetHWND()) && !isVisible)
					{
						//获取屏幕大小
						int iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
						int iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

						ui::UiRect rect = ptrForm->GetPos();

						SetWindowPos(ptrForm->GetHWND(), HWND_TOPMOST, iScreenWidth - rect.GetWidth() + 30, iScreenHeight / 2, rect.GetWidth(), rect.GetHeight(), SWP_SHOWWINDOW);

						isVisible = true;
					}

				}
				i++;
			}
		}
		catch (exception e)
		{
			YLog log(YLog::INFO, "log.txt", YLog::ADD);
			log.W(__FILE__, __LINE__, YLog::INFO, "exception", e.what());
		}
	}
}

BOOL CALLBACK EnumChildProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	char szTitle[MAX_PATH] = { 0 };
	char szTitle1[MAX_PATH] = { 0 };
	char szClass[MAX_PATH] = { 0 };
	int nMaxCount = MAX_PATH;

	LPSTR lpClassName = szClass;
	LPSTR lpWindowName = szTitle;

	GetWindowTextA(hwnd, lpWindowName, nMaxCount);
	GetClassNameA(hwnd, lpClassName, nMaxCount);

	YLog log(YLog::INFO, "log.txt", YLog::ADD);

	//log.W(__FILE__, __LINE__, YLog::INFO, "hwnd", hwnd);
	//log.W(__FILE__, __LINE__, YLog::INFO, "lpWindowName", shared::tools::UtfToString(lpWindowName));
	//log.W(__FILE__, __LINE__, YLog::INFO, "lpClassName", lpClassName);

	string className = lpClassName;
	string windowNameCurrent = lpWindowName;

	if (className.find("WebOfficeWnd") != string::npos)
	{
		isNetOfficeWndFind = true;
	}

	if (windowNameCurrent.find("电子病历 - Internet Explorer") != string::npos)
	{
		hospitalWndFindState = true;
	}

	if (strcmp(lpClassName, "WindowsForms10.Window.8.app.0.33c0d9d") == 0)
	{
		//log.W(__FILE__, __LINE__, YLog::INFO, "hwnd", hwnd);
		//log.W(__FILE__, __LINE__, YLog::INFO, "parent", GetParent(hwnd));
		GetClassNameA(GetParent(hwnd), lpClassName, nMaxCount);
		GetWindowTextA(GetParent(hwnd), lpWindowName, nMaxCount);
		if (strcmp(lpWindowName, " 人卫inside") == 0)
		{
			rwResultWnd = GetParent(hwnd);
		}
		else
		{
			if (GetParent(hwnd) > 0)
			{
				rwWnd = GetParent(hwnd);
			}

		}
		//log.W(__FILE__, __LINE__, YLog::INFO, "lpWindowName", shared::tools::UtfToString(lpWindowName));
		//log.W(__FILE__, __LINE__, YLog::INFO, "lpClassName", lpClassName);
	}
	size_t found;
	{
		HWND parent = GetParent(hwnd);
		if (parent)
		{
			LPSTR lpWindowName1 = szTitle1;

			GetWindowTextA(parent, lpWindowName1, nMaxCount);
			string windowName = lpWindowName1;
			if (!windowName.empty())
			{
				found = windowName.find("门诊病历");
				if (found != string::npos)
				{
					found = className.find("pbdw126");
					if (found != string::npos)
					{
						string windowName = lpWindowName;
						found = windowName.find("none");
						if (found != string::npos)
						{
							//log.W(__FILE__, __LINE__, YLog::INFO, "hwnd", hwnd);
							//log.W(__FILE__, __LINE__, YLog::INFO, "lpWindowName", shared::tools::UtfToString(lpWindowName));
							//log.W(__FILE__, __LINE__, YLog::INFO, "lpClassName", lpClassName);
							//print_window2(hwnd,1);

							for (int id = 1; id < 0x00000F; id++)
							{
								HWND h = ::GetDlgItem(hwnd, id);
								if (h != NULL)
								{
									int len = SendMessage(h, WM_GETTEXTLENGTH, 0, 0);
									TCHAR* buffer = new TCHAR[len + 1];
									::SendMessage(h, WM_GETTEXT, (WPARAM)(len + 1), (LPARAM)buffer);//第三个和第四个参数是缓存大小和缓存指针
									wstring dd = buffer;

									if (!dd.empty())
									{

										if (id == 10)
										{
											ocrMainSuit = nbase::UTF16ToUTF8(buffer);
										}
										else if (id == 11)
										{

											POINT point;
											GetCursorPos(&point);
											//log.W(__FILE__, __LINE__, YLog::INFO, "x", point.x);
											//log.W(__FILE__, __LINE__, YLog::INFO, "y", point.y);

											if (point.y == 280)
											{
												ocrIllnessHis = nbase::UTF16ToUTF8(buffer);
											}
											else if (point.y == 320)
											{
												ocrPastHis = nbase::UTF16ToUTF8(buffer);
											}
											else if (point.y == 360)
											{
												ocrPersonalHis = nbase::UTF16ToUTF8(buffer);
											}
											else if (point.y == 400)
											{
												ocrFamilyHis = nbase::UTF16ToUTF8(buffer);
											}
											else if (point.y == 430)
											{
												ocrAllergyHis = nbase::UTF16ToUTF8(buffer);
											}
											//log.W(__FILE__, __LINE__, YLog::INFO, "id", id);
											//log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("文本信息"), nbase::UTF16ToUTF8(buffer));
										}


									}
								}
							}
						}

					}

				}
			}
		}
	}

	string windowName = lpWindowName;
	if (!windowName.empty())
	{
		found = windowName.find("门诊病历");
		if (found != string::npos)
		{
			illHisWnd = hwnd;
			illHisWndFindState = true;
		}

		found = windowName.find("处方明细");
		if (found != string::npos)
		{
			prescribeWnd = hwnd;
		}

		found = windowName.find("欢迎使用湖南省基层医疗卫生机构管理信息系统");
		if (found != string::npos)
		{
			mainHisParentWnd = hwnd;

			//log.W(__FILE__, __LINE__, YLog::INFO, "lpWindowName", shared::tools::UtfToString(lpWindowName));
			//log.W(__FILE__, __LINE__, YLog::INFO, "lpClassName", lpClassName);

			if (strcmp(lpClassName, "FNHELP126") == 0)
			{
				HWND parent = GetParent(mainHisParentWnd);
				//log.W(__FILE__, __LINE__, YLog::INFO, "parent", parent);

				GetWindowTextA(parent, lpWindowName, nMaxCount);
				GetClassNameA(parent, lpClassName, nMaxCount);
				//log.W(__FILE__, __LINE__, YLog::INFO, "lpWindowName", shared::tools::UtfToString(lpWindowName));
				//log.W(__FILE__, __LINE__, YLog::INFO, "lpClassName", lpClassName);
				if (strcmp(lpClassName, "FNWND3126") == 0)
				{
					mainHisParentWnd = parent;
				}
			}

		}

		found = windowName.find("门诊医生工作站");
		if (found != string::npos)
		{
			//log.W(__FILE__, __LINE__, YLog::INFO, "lpWindowName", shared::tools::UtfToString(lpWindowName));
			//log.W(__FILE__, __LINE__, YLog::INFO, "lpClassName", lpClassName);

			HWND parent = GetParent(hwnd);

			mainHisWnd = GetParent(parent);

			GetWindowTextA(mainHisWnd, lpWindowName, nMaxCount);
			GetClassNameA(mainHisWnd, lpClassName, nMaxCount);
			//log.W(__FILE__, __LINE__, YLog::INFO, "lpWindowName", shared::tools::UtfToString(lpWindowName));
			//log.W(__FILE__, __LINE__, YLog::INFO, "lpClassName", lpClassName);
		}

	}
	return TRUE;
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	/*
	Remarks
	The EnumWindows function does not enumerate child windows,
	with the exception of a few top-level windows owned by the
	system that have the WS_CHILD style.
	*/
	char szTitle[MAX_PATH] = { 0 };
	char szClass[MAX_PATH] = { 0 };
	int nMaxCount = MAX_PATH;

	LPSTR lpClassName = szClass;
	LPSTR lpWindowName = szTitle;

	GetWindowTextA(hwnd, lpWindowName, nMaxCount);
	GetClassNameA(hwnd, lpClassName, nMaxCount);

	EnumChildWindows(hwnd, EnumChildProc, lParam);

	return TRUE;
}

void  GetAllWindowState()
{
	//每次遍历窗口前初始化各窗口标识状态
	illHisWndFindState = false;
	hospitalWndFindState = false;
	prescribeWnd = 0;
	illHisWnd = 0;
	mainHisWnd = 0;
	rwResultWnd = 0;
	EnumWindows(EnumWindowsProc, 0);
}

BasicForm::BasicForm()
{
}

BasicForm::~BasicForm()
{
}

std::wstring BasicForm::GetSkinFolder()
{
	return L"cef";
}

std::wstring BasicForm::GetSkinFile()
{
	//return L"cef.xml";
	return L"main.xml";
}

std::wstring BasicForm::GetWindowClassName() const
{
	return kClassName;
}

void BasicForm::AutoHiddenWindow(int iNum)
{
	//获取屏幕大小
	iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	//RECT m_rtWindow;
	GetWindowRect(m_hWnd, &m_rtWindow);
	m_iWindowWidth = m_rtWindow.right - m_rtWindow.left;
	m_iWindowHeight = m_rtWindow.bottom - m_rtWindow.top;

	//left
	if (m_rtWindow.right >= iNum && m_rtWindow.left < 0 && m_rtWindow.top > 0)
	{
		// 获取窗口范围
		GetWindowRect(m_hWnd, &m_rtWindow);
		// 移动窗口
		MoveWindow(m_hWnd, 10, m_rtWindow.top, m_iWindowWidth, m_iWindowHeight, TRUE);
	}

	//top
	if (m_rtWindow.top < 20 && m_rtWindow.bottom >= iNum)
	{
		// 获取窗口范围
		GetWindowRect(m_hWnd, &m_rtWindow);
		// 移动窗口
		MoveWindow(m_hWnd, m_rtWindow.left, 50, m_iWindowWidth, m_iWindowHeight, TRUE);
	}

	//bottom
	if (m_rtWindow.top > 0 && m_rtWindow.bottom > (iScreenHeight - 50))
	{
		// 获取窗口范围
		GetWindowRect(m_hWnd, &m_rtWindow);
		// 移动窗口
		MoveWindow(m_hWnd, m_rtWindow.left, (m_rtWindow.top - m_iWindowHeight - 30), m_iWindowWidth, m_iWindowHeight, TRUE);
	}

	//right
	if (m_rtWindow.right >= (iScreenWidth - 10) && m_rtWindow.left <= iScreenWidth - iNum && m_rtWindow.top > 0)
	{
		// 获取窗口范围
		GetWindowRect(m_hWnd, &m_rtWindow);
		// 移动窗口
		MoveWindow(m_hWnd, m_rtWindow.left + 2, m_rtWindow.top, m_iWindowWidth, m_iWindowHeight, TRUE);


		if (btn_doctor != NULL)
		{
			btn_doctor->SetVisible(false);
		}
		if (btn_doctor1 != NULL)
		{
			btn_doctor1->SetVisible(true);
			box_tool->SetVisible(false);
		}

	}
}

void BasicForm::AutoShowWindow()
{
	// 获取屏幕大小
	iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT m_rtWindow;
	GetWindowRect(this->m_hWnd, &m_rtWindow);
	m_iWindowWidth = m_rtWindow.right - m_rtWindow.left;
	m_iWindowHeight = m_rtWindow.bottom - m_rtWindow.top;

	// 向右弹出
	/*while (m_rtWindow.left < 0)
	{
	// 移动窗口
	MoveWindow(this->m_hWnd, m_rtWindow.left + 2, m_rtWindow.top, m_iWindowWidth, m_iWindowHeight, FALSE);
	// 获取窗口范围
	GetWindowRect(this->m_hWnd, &m_rtWindow);
	}

	// 向下收弹出
	while (m_rtWindow.top < 0)
	{
	// 移动窗口
	MoveWindow(this->m_hWnd, m_rtWindow.left, m_rtWindow.top + 2, m_iWindowWidth, m_iWindowHeight, FALSE);
	// 获取窗口范围
	GetWindowRect(this->m_hWnd, &m_rtWindow);
	}*/

	// 向左弹出
	while (m_rtWindow.right > iScreenWidth)
	{
		// 移动窗口
		MoveWindow(this->m_hWnd, m_rtWindow.left - 2, m_rtWindow.top, m_iWindowWidth, m_iWindowHeight, FALSE);
		// 获取窗口范围
		GetWindowRect(this->m_hWnd, &m_rtWindow);

		if (btn_doctor != NULL)
		{
			btn_doctor->SetVisible(true);
			box_tool->SetVisible(true);
		}

		if (btn_doctor1 != NULL)
		{
			btn_doctor1->SetVisible(false);
		}
	}

}

bool BasicForm::JudgeCursorOut()
{
	// 获取鼠标位置
	POINT point;
	GetCursorPos(&point);

	// 获取窗口范围
	RECT m_rtWindow;
	GetWindowRect(this->m_hWnd, &m_rtWindow);

	// 判断
	if (point.x >= m_rtWindow.left && point.x <= m_rtWindow.right && point.y >= m_rtWindow.top && point.y <= m_rtWindow.bottom)
	{
		return false;  // 鼠标还在窗口内
	}
	else
	{
		return true;   // 鼠标离开窗口
	}
}

void BasicForm::ShowDevTool()
{
	if (IsNetConnected())
	{
		std::string url = tool->GetDevToolUrl();
		if (url.size() > 0)
		{
			if (m_hiddenWindow == NULL)
			{
				m_hiddenWindow = new CefForm();
				m_hiddenWindow->SetMaxFlag(false);
				m_hiddenWindow->SetHiddenFlag(true);
				m_hiddenWindow->SetNavigateUrl(url);
				m_hiddenWindow->SetMainWndHWND(GetHWND());
				m_hiddenWindow->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
				m_hiddenWindow->CenterWindow();
				m_hiddenWindow->ShowWindow(false, false);
			}
			else
			{
				m_hiddenWindow->SetNavigateUrl(url);
				m_hiddenWindow->RefreshNavigateUrl();
			}

		}
	}
}
ui::Control* BasicForm::CreateControl(const std::wstring& pstrClass)
{
	// 扫描 XML 发现有名称为 CefControl 的节点，则创建一个 ui::CefControl 控件
	if (pstrClass == L"CefControl")
	{
		if (nim_cef::CefManager::GetInstance()->IsEnableOffsetRender())
			return new ui::CefControl;
		else
			return new ui::CefNativeControl;
	}

	return NULL;
}

void GetToolConfigThreadFun(void*& data)
{
	std::string toolConfigUrl = tool->GetToolConfigUrl();

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	if (!toolConfigUrl.empty())
	{
		bool isSuccess = false;
		while (!isSuccess)
		{
			CWininetHttp netHttp;
			std::string ret = netHttp.RequestJsonInfo(toolConfigUrl, Hr_Get,
				"Content-Type:application/json;charset=utf-8", "{}");

			log.W(__FILE__, __LINE__, YLog::DEBUG, "GetToolConfigThreadFun", ret);

			if (!ret.empty())
			{
				Json::Reader reader;
				Json::Value value;

				if (reader.parse(ret, value))
				{
					if (value.isArray())
					{
						g_value = value;
						isSuccess = true;
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	}

	BasicForm* ptrForm = (BasicForm*)data;
	if (ptrForm != NULL)
	{
		::SendMessage(ptrForm->GetHWND(), WM_SETTOOLBTNURL, 0, 0);
	}
}

/*
判断是否安装.NETFramework
*/
bool IsFrameworkInstalled()
{
	HKEY hKey;
	LPCTSTR path1 = TEXT("SOFTWARE\\Microsoft\\.NETFramework\\v2.0.50727");
	LPCTSTR path2 = TEXT("SOFTWARE\\Microsoft\\.NETFramework\\v4.0.30319");
	LONG lResult1 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path1, 0, KEY_READ, &hKey);
	::RegCloseKey(hKey);
	LONG lResult2 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path1, 0, KEY_READ, &hKey);
	::RegCloseKey(hKey);

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	if (lResult1 != ERROR_SUCCESS || lResult2 != ERROR_SUCCESS)
	{
		log.W(__FILE__, __LINE__, YLog::INFO, ".NETFramework", shared::tools::UtfToString("未安装.NETFramework"));
		return false;
	}
	else
	{
		log.W(__FILE__, __LINE__, YLog::DEBUG, ".NETFramework", shared::tools::UtfToString("已安装.NETFramework"));
		return true;
	}
}

DWORD GetProcessIDByName(WCHAR* pName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		return 0;
	}
	PROCESSENTRY32 pe = { sizeof(pe) };
	for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe)) {
		if (wcscmp(pe.szExeFile, pName) == 0) {
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
	}
	CloseHandle(hSnapshot);
	return 0;
}

void CheckRWKnowledgeFun(void*& data)
{
	isTreadRWKnowledgeState = true;
	while (isTreadRWKnowledgeState)
	{
		DWORD pid = GetProcessIDByName(L"RWKnowledge.exe");
		YLog log(YLog::INFO, "log.txt", YLog::ADD);

		if (pid == 0)
		{
			std::string exePath = nbase::UTF16ToUTF8(QPath::GetAppPath());
			std::string rwPath = exePath + "RWKnowledge";
			if (!shared::tools::FilePathIsExist(rwPath, true))
			{
				log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("人卫文件夹不存在"), rwPath);
				return;
			}
			else
			{
				std::string rwKnowledgePath = exePath + "RWKnowledge\\RWKnowledge.exe";

				ShellExecute(NULL, _T("open"), nbase::UTF8ToUTF16(rwKnowledgePath.c_str()).c_str(), NULL, NULL, SW_SHOWNORMAL);

				log.W(__FILE__, __LINE__, YLog::INFO, "RWKnowledge.exe", "restart");
			}

			std::this_thread::sleep_for(std::chrono::seconds(3));

			if (CefForm::g_ptr_rw_cef != NULL)
			{
				CefForm::g_ptr_rw_cef->RefreshNavigateUrl();
			}

		}

		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
}

void WriteStartRegFile()
{
	ofstream fout("open.reg", ios::out);
	if (!fout)
	{
		cout << "文件打开失败！" << endl;
		exit(1);
	}
	fout << "Windows Registry Editor Version 5.00\n";
	fout << "[HKEY_CLASSES_ROOT\\Diagnosticshell]\n";
	fout << "@=\"Open Diagnostics\"\n";
	fout << "\"URL Protocol\" = \"\"\n";
	fout << "[HKEY_CLASSES_ROOT\\Diagnosticshell\\DefaultIcon]\n";

	std::string exeDir = nbase::UTF16ToUTF8(QPath::GetAppPath());
	std::string exePath = exeDir + "DiagnosticAssistant.exe";

	exePath = shared::tools::replaceAllMark(exePath, "\\", "\\\\");

	std::string data = "@=\"" + exePath + "\"\n";
	fout << data;
	fout << "[HKEY_CLASSES_ROOT\\Diagnosticshell\\shell]\n";
	fout << "[HKEY_CLASSES_ROOT\\Diagnosticshell\\shell\\open]\n";
	fout << "[HKEY_CLASSES_ROOT\\Diagnosticshell\\shell\\open\\command]\n";

	data = "@=\"\\\"" + exePath + "\\\" \\\"%1\\\"\"\n";
	fout << data;

	fout.close();

	//std::string regdir = exeDir + "open.reg";
	//std::string cmd = "regedit /s " + regdir;
	//system(cmd.c_str());
	//WinExec(cmd.c_str(), SW_HIDE);
	//ShellExecute(NULL, L"open", L"open.reg", NULL, NULL, SW_HIDE);

	HKEY hKey = NULL;
	TCHAR * subKey = _T("Diagnosticshell");
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	DWORD dwDisposition;
	long result = RegCreateKeyEx(HKEY_CLASSES_ROOT, subKey, 0, NULL,
		dwOptions, KEY_WRITE, NULL, &hKey, &dwDisposition);
	if (result != ERROR_SUCCESS)
	{

	}
	else
	{
		/*if (dwDisposition == REG_OPENED_EXISTING_KEY)
		{

		}
		else if (dwDisposition == REG_CREATED_NEW_KEY)
		{

		}*/

		try
		{
			if (RegOpenKeyEx(HKEY_CLASSES_ROOT, subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) ///打开启动项       
			{

				std::string regStr = "";
				std::wstring exe_dir = nbase::UTF8ToUTF16(regStr);

				//3、判断注册表项是否已经存在
				TCHAR strDir[MAX_PATH] = {};
				//5、添加一个子Key,并设置值，"GISRestart"是应用程序名字（不加后缀.exe） 
				RegSetValueEx(hKey, _T("URL Protocol"), 0, REG_SZ, (LPBYTE)exe_dir.c_str(),
					(lstrlen(exe_dir.c_str()) + 1) * sizeof(TCHAR));
				//6、关闭注册表
				RegCloseKey(hKey);
			}
			else
			{
				std::cout << "警告\n系统参数错误,不能随系统启动" << std::endl;
			}
		}
		catch (std::exception e)
		{
		}

		subKey = _T("Diagnosticshell\\shell\\open\\command");
		result = RegCreateKeyEx(HKEY_CLASSES_ROOT, subKey, 0, NULL,
			dwOptions, KEY_WRITE, NULL, &hKey, &dwDisposition);

		try
		{
			//1、找到系统的启动项  
			if (RegOpenKeyEx(HKEY_CLASSES_ROOT, subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) ///打开启动项       
			{
				std::string regExePath = exeDir + "DiagnosticAssistant.exe";
				std::string regStr = "\"" + regExePath + "\" \"%1\"";
				std::wstring exe_dir = nbase::UTF8ToUTF16(regStr);

				//3、判断注册表项是否已经存在
				TCHAR strDir[MAX_PATH] = {};
				//5、添加一个子Key,并设置值，"GISRestart"是应用程序名字（不加后缀.exe） 
				RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)exe_dir.c_str(),
					(lstrlen(exe_dir.c_str()) + 1) * sizeof(TCHAR));
				//6、关闭注册表
				RegCloseKey(hKey);
			}
			else
			{

			}
		}
		catch (std::exception e)
		{
		}
	}

}

void BasicForm::InitWindow()
{
	// 监听鼠标单击事件
	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&BasicForm::OnClicked, this, std::placeholders::_1));

	btn_doctor = FindControl(L"doctor");
	btn_doctor1 = FindControl(L"doctor1");
	if (btn_doctor1)
	{
		//默认隐藏提示图标
		btn_doctor1->SetVisible(false);
		btn_doctor1->SetBkImage(L"doctor-hi-disable.png");
	}

	if (btn_doctor)
	{
		btn_doctor->SetBkImage(L"doctor-disable.png");
	}

	box_tool = dynamic_cast<ui::Box*>(FindControl(L"toolBox"));

	tool_btn1 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting1"));
	tool_btn2 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting2"));
	tool_btn3 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting3"));
	tool_btn4 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting4"));
	tool_btn5 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting5"));
	tool_btn6 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting6"));

	LONG styleValue = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
	styleValue &= ~(WS_EX_APPWINDOW);//当窗口可见时将一个顶层窗口放置在任务栏上
	styleValue |= WS_EX_TOOLWINDOW; //工具条窗口样式
	SetWindowLong(m_hWnd, GWL_EXSTYLE, styleValue);

	AddTrayIcon();

	SetIcon(107);

	CreateTrayMenu();

	SetTaskbarTitle(L"辅助诊断助手");

	std::string exeDir = nbase::UTF16ToUTF8(QPath::GetAppPath());
	std::string regPath = exeDir + "open.reg";

	if (!shared::tools::FilePathIsExist(regPath, false))
	{
		WriteStartRegFile();
	}

	//20M文件 20*1000**1000
	shared::tools::ClearFile(L"log.txt", 20000000);

	serverUtil->m_MainHwnd = GetHWND();

	boost::thread serverThread(&RunHttpServer);
	serverThread.detach();

	boost::thread receive_thread(boost::bind(&ReceiveMessage, (void*)this));
	receive_thread.detach();

	boost::thread toolConfigThread(boost::bind(&GetToolConfigThreadFun, (void*)this));
	toolConfigThread.detach();

	if (IsFrameworkInstalled())
	{
		if (tool->GetRwStartConfig())
		{
			boost::thread rwThread(boost::bind(&CheckRWKnowledgeFun, (void*)this));
			rwThread.detach();
		}
	}

	ShowDevTool();

	CefForm::g_main_hwnd = GetHWND();

	SetForegroundWindow(m_hWnd);

	ShowWindow(true);

	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void BasicForm::ExitApp()
{
	/*
	UnregisterHotKey(GetHWND(), hotkeyId8);
	UnregisterHotKey(GetHWND(), hotkeyId7);
	UnregisterHotKey(GetHWND(), hotkeyId6);
	UnregisterHotKey(GetHWND(), hotkeyId5);
	UnregisterHotKey(GetHWND(), hotkeyId4);
	UnregisterHotKey(GetHWND(), hotkeyId3);
	UnregisterHotKey(GetHWND(), hotkeyId1);
	*/

	ShowWindow(SW_HIDE);

	isTreadState = false;
	isTreadMessageState = false;
	isTreadRWKnowledgeState = false;

	m_trayIcon.hIcon = NULL;
	Shell_NotifyIcon(NIM_DELETE, &m_trayIcon);

	if (serverUtil)
	{
		delete serverUtil;
		serverUtil = NULL;
	}

	if (tool)
	{
		delete tool;
		tool = NULL;
	}

	PostQuitMessage(0L);
}

LRESULT BasicForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExitApp();
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

/*
门诊病历界面是否打开
*/
bool IsOutpatientMedicalRecordInterfaceOpen()
{
	DWORD pid = GetProcessIDByName(L"bmhip.exe");

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	if (pid == 0)
	{
		return false;
	}

	GetAllWindowState();

	if (!illHisWndFindState)
	{
		return false;
	}
	else
	{
		if (illHisWnd > 0)
		{
			if (!::IsWindowVisible(illHisWnd))
			{
				return false;
			}


			if (prescribeWnd <= 0)
			{
				return false;
			}

			if (!::IsWindowVisible(prescribeWnd))
			{
				return false;
			}
		}
	}
	return true;
}

void RequestDiseases()
{

	CWininetHttp netHttp;
	std::string ret = netHttp.RequestJsonInfo(urlString, Hr_Post,
		"Content-Type:application/json;charset=utf-8", paraString);
	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(__FILE__, __LINE__, YLog::INFO, "ret", ret);

	if (!ret.empty())
	{
		Json::Reader reader;
		Json::Value value;

		if (reader.parse(ret, value))
		{
			if (value.isMember("errorMessage"))
			{
				if (value["errorMessage"].isString())
				{
					string errorMessage = value["errorMessage"].asString();
				}
			}
		}
	}
}

void BmhipThreadFun()
{
	time_t start = time(NULL);
	time_t nowSecond = start;
	while ((nowSecond - start) < 3)
	{
		nowSecond = time(NULL);
	}

	SetCursorPos(320, 280);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 320, 280, 0, 0);
	boost::this_thread::sleep(boost::posix_time::microseconds(500));
	GetAllWindowState();


	SetCursorPos(320, 320);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 320, 320, 0, 0);
	boost::this_thread::sleep(boost::posix_time::microseconds(500));
	GetAllWindowState();


	SetCursorPos(320, 360);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 320, 360, 0, 0);
	boost::this_thread::sleep(boost::posix_time::microseconds(500));
	GetAllWindowState();

	SetCursorPos(320, 400);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 320, 400, 0, 0);
	boost::this_thread::sleep(boost::posix_time::microseconds(500));
	GetAllWindowState();

	SetCursorPos(320, 430);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 320, 430, 0, 0);
	boost::this_thread::sleep(boost::posix_time::microseconds(500));
	GetAllWindowState();

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("主诉"), ocrMainSuit);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("现病史"), ocrIllnessHis);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("既往史"), ocrPastHis);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("个人史"), ocrPersonalHis);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("家族史"), ocrFamilyHis);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("过敏史"), ocrAllergyHis);


	Json::Value rootPara;
	rootPara["zs"] = ocrMainSuit;
	rootPara["xbs"] = ocrIllnessHis;
	rootPara["jws"] = ocrPastHis;
	rootPara["allergyHis"] = ocrAllergyHis;
	rootPara["jzs"] = ocrFamilyHis;
	rootPara["grs"] = ocrPersonalHis;

	std::string stdDiagnoseUrl = tool->GetAssistantDiagnoseUrl();
	if (stdDiagnoseUrl.empty())
	{
		stdDiagnoseUrl = "http://medical.c2cloud.cn/kgms/ylkg/v1/diag_cdss/emr";
	}

	std::string url = stdDiagnoseUrl;
	urlString = url;


	Json::Value resultPara;
	resultPara["emr"] = rootPara;

	Json::Value config;
	config["client"] = CefForm::strUserName.c_str();
	config["is_push_mode"] = true;
	config["push_emrs"] = true;
	config["push_emr_count"] = 20;
	resultPara["config"] = config;

	paraString = resultPara.toStyledString();

	log.W(__FILE__, __LINE__, YLog::INFO, shared::tools::UtfToString("请求参数"), resultPara.toStyledString());

	boost::thread requestThread(&RequestDiseases);
	requestThread.detach();
}

/*
自动获取湖南基层医疗卫生系统门诊医生工作站的病历信息
*/
void  BasicForm::GetBmhipInfo()
{
	DWORD pid = GetProcessIDByName(L"bmhip.exe");

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	//log.W(__FILE__, __LINE__, YLog::INFO, "bmhip.exe", pid);

	if (pid == 0)
	{
		ShowBalloonTip(_T("请先打开湖南基层医疗卫生系统！"), _T("辅助诊断助手"));
		return;
	}

	GetAllWindowState();

	if (!illHisWndFindState)
	{
		ShowBalloonTip(_T("请先打开门诊医生工作站门诊病历界面！"), _T("辅助诊断助手"));
		return;
	}
	else
	{
		if (illHisWnd > 0)
		{
			if (!::IsWindowVisible(illHisWnd))
			{
				ShowBalloonTip(_T("请确保门诊病历界面是否显示在桌面上！"), _T("辅助诊断助手"));
				return;
			}


			if (prescribeWnd <= 0)
			{
				ShowBalloonTip(_T("请先打开病历界面填写患者的病历！"), _T("辅助诊断助手"));
				return;
			}

			if (!::IsWindowVisible(prescribeWnd))
			{
				ShowBalloonTip(_T("请先打开患者的病历！"), _T("辅助诊断助手"));
				return;
			}

			toastHwnd = shared::Toast::ShowToast(_T("正在请求中，请稍候！"), 5000, m_hWnd);
			log.W(__FILE__, __LINE__, YLog::INFO, "mainHisWnd", mainHisWnd);


			RECT rect;
			GetWindowRect(mainHisWnd, &rect);
			::SetWindowPos(mainHisWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			SetForegroundWindow(mainHisWnd);

			boost::thread threadBmhip(&BmhipThreadFun);
			threadBmhip.detach();
		}
	}
}
bool BasicForm::OnClicked(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();

	if (name == L"proxy_setting1")
	{
		if (IsNetConnected())
		{
			if (windowMap.count(m_navUrl1) > 0 && windowMap[m_navUrl1] > 0)
			{
				::SendMessage(windowMap[m_navUrl1], WM_SYSCOMMAND, SC_RESTORE, NULL);
				RECT rect;
				GetWindowRect(windowMap[m_navUrl1], &rect);
				::SetWindowPos(windowMap[m_navUrl1], HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			}
			else
			{
				CefForm* window = new CefForm();
				window->SetNavigateUrl(m_navUrl1);
				window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
				window->CenterWindow();
				window->ShowWindow();
			}
		}
		else
		{
			shared::Toast::ShowToast(_T("网络未连接！"), 3000, GetHWND());
		}
	}
	else if (name == L"proxy_setting2")
	{
		if (IsNetConnected())
		{
			if (windowMap.count(m_navUrl2) > 0 && windowMap[m_navUrl2] > 0)
			{
				::SendMessage(windowMap[m_navUrl2], WM_SYSCOMMAND, SC_RESTORE, NULL);
				RECT rect;
				GetWindowRect(windowMap[m_navUrl2], &rect);
				::SetWindowPos(windowMap[m_navUrl2], HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			}
			else
			{
				CefForm* window = new CefForm();
				window->SetNavigateUrl(m_navUrl2);
				window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
				window->CenterWindow();
				window->ShowWindow();
			}

		}
		else
		{
			shared::Toast::ShowToast(_T("网络未连接！"), 3000, GetHWND());
		}
	}
	else if (name == L"proxy_setting3")
	{
		if (IsNetConnected())
		{
			if (windowMap.count(m_navUrl3) > 0 && windowMap[m_navUrl3] > 0)
			{
				::SendMessage(windowMap[m_navUrl3], WM_SYSCOMMAND, SC_RESTORE, NULL);
				RECT rect;
				GetWindowRect(windowMap[m_navUrl3], &rect);
				::SetWindowPos(windowMap[m_navUrl3], HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			}
			else
			{
				CefForm* window = new CefForm();
				window->SetNavigateUrl(m_navUrl3);
				window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
				window->CenterWindow();
				window->ShowWindow();
			}
		}
		else
		{
			shared::Toast::ShowToast(_T("网络未连接！"), 3000, GetHWND());
		}
	}
	else if (name == L"proxy_setting4")
	{
		if (IsNetConnected())
		{
			if (IsOutpatientMedicalRecordInterfaceOpen())
			{
				GetBmhipInfo();
			}
			else
			{
				if (windowMap.count(m_navUrl4) > 0 && windowMap[m_navUrl4] > 0)
				{
					::SendMessage(windowMap[m_navUrl4], WM_SYSCOMMAND, SC_RESTORE, NULL);
					RECT rect;
					GetWindowRect(windowMap[m_navUrl4], &rect);
					::SetWindowPos(windowMap[m_navUrl4], HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
				}
				else
				{
					CefForm* window = new CefForm();
					window->SetNavigateUrl(m_navUrl4);
					window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
					window->CenterWindow();
					window->ShowWindow();
				}
			}
		}
		else
		{
			shared::Toast::ShowToast(_T("网络未连接！"), 3000, GetHWND());
		}
	}
	else if (name == L"proxy_setting5")
	{
		if (IsNetConnected())
		{
			if (windowMap.count(m_navUrl5) > 0 && windowMap[m_navUrl5] > 0)
			{
				::SendMessage(windowMap[m_navUrl5], WM_SYSCOMMAND, SC_RESTORE, NULL);
				RECT rect;
				GetWindowRect(windowMap[m_navUrl5], &rect);
				::SetWindowPos(windowMap[m_navUrl5], HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			}
			else
			{
				CefForm* window = new CefForm();
				window->SetNavigateUrl(m_navUrl5);
				window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
				window->CenterWindow();
				window->ShowWindow();
			}
		}
		else
		{
			shared::Toast::ShowToast(_T("网络未连接！"), 3000, GetHWND());
		}

	}
	else if (name == L"proxy_setting6")
	{
		if (IsNetConnected())
		{
			if (windowMap.count(m_navUrl6) > 0 && windowMap[m_navUrl6] > 0)
			{
				::SendMessage(windowMap[m_navUrl6], WM_SYSCOMMAND, SC_RESTORE, NULL);
				RECT rect;
				GetWindowRect(windowMap[m_navUrl6], &rect);
				::SetWindowPos(windowMap[m_navUrl6], HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			}
			else
			{
				CefForm* window = new CefForm();
				//window->SetMaxFlag(false);
				window->SetNavigateUrl(m_navUrl6);
				window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
				//window->CenterWindow();
				//window->SetInitSize(800, 450);
				window->ShowWindow();

				//ui::UiRect rect = window->GetPos();
				//MoveWindow(window->GetHWND(), rect.left, rect.top, 800, 450, true);
				window->CenterWindow();
			}

		}
		else
		{
			shared::Toast::ShowToast(_T("网络未连接！"), 3000, GetHWND());
		}

	}
	return true;
}

void BasicForm::UpdateHideStateNoticeMenu()
{
	ModifyMenu(hMenu, 0, MF_UNCHECKED | MF_BYPOSITION, WM_ONSHOWNOTICE, _T("显示调试窗"));
	ModifyMenu(hMenu, 1, MF_CHECKED | MF_BYPOSITION, WM_ONHIDENOTICE, _T("隐藏调试窗"));
}

BOOL BasicForm::ShowBalloonTip(wstring szMsg, wstring szTitle, DWORD dwInfoFlags, UINT uTimeout)
{
	m_trayIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_trayIcon.uFlags = NIF_INFO;
	m_trayIcon.uVersion = NOTIFYICON_VERSION;
	m_trayIcon.uTimeout = uTimeout;
	m_trayIcon.dwInfoFlags = dwInfoFlags;

	wcscpy_s(m_trayIcon.szInfo, szMsg.c_str());
	wcscpy_s(m_trayIcon.szInfoTitle, szTitle.c_str());

	return 0 != Shell_NotifyIcon(NIM_MODIFY, &m_trayIcon);
}

/*
启动线程每隔一秒检测人卫结果页面并将它置为桌面顶端
*/
void CheckRWKnowledgeResultFun(void*& data)
{
	while (true)
	{
		GetAllWindowState();
		std::this_thread::sleep_for(std::chrono::seconds(1));

		if (rwResultWnd > 0)
		{
			SendMessage(rwResultWnd, WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			SetForegroundWindow(rwResultWnd);
			return;
		}
	}
}

void CheckRWKnowledgeSetWindowRect()
{
	while (true)
	{
		GetAllWindowState();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (rwResultWnd > 0)
		{
			if (result_form)
			{
				if (IsWindowVisible(result_form->GetHWND()))
				{
					
					RECT rect;
					GetWindowRect(result_form->GetHWND(), &rect);

					if (rect.right > iScreenWidth)
					{
						::SetWindowPos(rwResultWnd, HWND_TOPMOST, rect.left + 120, rect.top + 50, iScreenWidth - rect.left - 150, rect.bottom - rect.top - 50, SWP_NOACTIVATE);
					}
					else
					{
						::SetWindowPos(rwResultWnd, HWND_TOPMOST, rect.left + 120, rect.top + 50, rect.right - rect.left - 120, rect.bottom - rect.top - 50, SWP_NOACTIVATE);
					}
				}
			}
			return;
		}
	}
}


LRESULT BasicForm::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	//如果在图标中单击左键则还原
	if (lParam == WM_LBUTTONDOWN)
	{
		ShowWindow(true);
		::SetFocus(m_hWnd);
		SetForegroundWindow(m_hWnd);
		ModifyMenu(hMenu, 7, MF_CHECKED | MF_BYPOSITION, WM_ONSHOW, _T("显示主窗口"));
		ModifyMenu(hMenu, 8, MF_UNCHECKED | MF_BYPOSITION, WM_ONMIN, _T("隐藏主窗口"));

		//显示主窗口
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	//如果在图标中单击右键则弹出声明式菜单
	if (lParam == WM_RBUTTONDOWN)
	{
		//获取鼠标坐标
		POINT pt;
		GetCursorPos(&pt);
		//右击后点别地可以清除“右击出来的菜单”
		SetForegroundWindow(m_hWnd);

		//弹出菜单,并把用户所选菜单项的标识符返回
		int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, m_hWnd, NULL);
		//如果标识符是WM_ONCLOSE则关闭
		if (cmd == WM_ONCLOSE)
		{
			//退出程序
			ExitApp();
		}
		else if (cmd == WM_ONSHOW)
		{
			ModifyMenu(hMenu, 7, MF_CHECKED | MF_BYPOSITION, WM_ONSHOW, _T("显示主窗口"));
			ModifyMenu(hMenu, 8, MF_UNCHECKED | MF_BYPOSITION, WM_ONMIN, _T("隐藏主窗口"));

			ShowWindow(true);
		}
		else if (cmd == WM_ONMIN)
		{

			ModifyMenu(hMenu, 7, MF_UNCHECKED | MF_BYPOSITION, WM_ONSHOW, _T("显示主窗口"));
			ModifyMenu(hMenu, 8, MF_CHECKED | MF_BYPOSITION, WM_ONMIN, _T("隐藏主窗口"));

			ShowWindow(false);
		}
		else if (cmd == WM_ONSHOWNOTICE)
		{
			ModifyMenu(hMenu, 0, MF_CHECKED | MF_BYPOSITION, WM_ONSHOWNOTICE, _T("显示调试窗"));
			ModifyMenu(hMenu, 1, MF_UNCHECKED | MF_BYPOSITION, WM_ONHIDENOTICE, _T("隐藏调试窗"));

			if (m_hiddenWindow)
			{
				m_hiddenWindow->ShowWindow(true);
			}
		}
		else if (cmd == WM_ONHIDENOTICE)
		{

			UpdateHideStateNoticeMenu();

			if (m_hiddenWindow)
			{
				m_hiddenWindow->ShowWindow(false);
			}
		}
		else if (cmd == WM_WSRECONNECT)
		{
			if (m_hiddenWindow)
			{
				m_hiddenWindow->RefreshNavigateUrl();
			}
		}
		else if (cmd == WM_USERQUIT)
		{
			int ret = MessageBox(GetHWND(), _T("确定切换账号吗？"), _T("辅助诊断助手"), MB_SYSTEMMODAL | MB_YESNO | MB_ICONQUESTION);
			if (ret == IDYES)
			{
				DeleteFileA("login.ini");

				ExitApp();

				std::string exePath = nbase::UTF16ToUTF8(QPath::GetAppPath());
				std::string clearCachePath = exePath + "ClearCacheTool.exe";
				ShellExecute(NULL, _T("open"), nbase::UTF8ToUTF16(clearCachePath.c_str()).c_str(), NULL, NULL, SW_SHOWNORMAL);

			}
		}
		else if (cmd == WM_CLEARCACHE)
		{
			int ret = MessageBox(GetHWND(), _T("确定清除浏览器缓存？"), _T("辅助诊断助手"), MB_SYSTEMMODAL | MB_YESNO | MB_ICONQUESTION);
			if (ret == IDYES)
			{
				std::string exePath = nbase::UTF16ToUTF8(QPath::GetAppPath());
				std::string clearCachePath = exePath + "ClearCacheTool.exe";
				ShellExecute(NULL, _T("open"), nbase::UTF8ToUTF16(clearCachePath.c_str()).c_str(), NULL, NULL, SW_SHOWNORMAL);

				ExitApp();
			}
		}
	}

	if (uMsg == WM_SETTOOLBTNURL)
	{
		RequestToolConfigUrl();
	}
	else if (uMsg == WM_USERONLINESTATE)
	{
		if (wParam == 1)
		{
			btn_doctor->SetBkImage(L"doctor.png");
			btn_doctor1->SetBkImage(L"doctor-hi.png");

			m_trayIcon.hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(131));
			Shell_NotifyIcon(NIM_MODIFY, &m_trayIcon);

			ShowBalloonTip(L"上线成功！", L"辅助诊断助手", NIIF_INFO,3000);

			isUserOnline = true;
		}
		else if (wParam == 2)
		{
			if (isUserOnline)
			{
				isUserOnline = false;

				btn_doctor->SetBkImage(L"doctor-disable.png");
				btn_doctor1->SetBkImage(L"doctor-hi-disable.png");

				ShowBalloonTip(L"已离线！", L"辅助诊断助手", NIIF_INFO, 3000);

				m_trayIcon.hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(133));
				Shell_NotifyIcon(NIM_MODIFY, &m_trayIcon);

			}
		}
	}
	else if (uMsg == WM_CEFWINDOWOPEN)
	{
		string* url = (string*)wParam;
		HWND   hwnd = (HWND)lParam;

		if (url != NULL && !url->empty())
		{
			windowMap[*url] = hwnd;
		}

	}
	else if (uMsg == WM_CEFWINDOWCLOSE)
	{
		HWND  hwnd = (HWND)wParam;

		map<string, HWND>::iterator iter;//定义一个迭代指针iter
		for (iter = windowMap.begin(); iter != windowMap.end(); iter++)
		{
			if (iter->second == hwnd)
			{
				iter->second = 0;
			}
		}
	}
	else if (uMsg == WM_OPENEXISTCEFWINDOW)
	{
		HWND   hwnd = (HWND)wParam;
		if (hwnd > 0)
		{
			::SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
			RECT rect;
			GetWindowRect(hwnd, &rect);
			::SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);
		}

	}
	else if (uMsg == WM_OPENCEFWINDOWFROMRESULT)
	{
		string*	 url = (string*)wParam;
		POINT*   point = (POINT*)lParam;

		if (url != NULL && !url->empty() && point != NULL)
		{
			if (windowMap.count(*url) > 0 && windowMap[*url] > 0)
			{
				HWND   hwnd = windowMap[*url];

				::SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
				RECT rect;
				GetWindowRect(hwnd, &rect);
				::SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);

			}
			else
			{
				CefForm* window = new CefForm();
				window->SetNavigateUrl(*url);
				window->SetMaxFlag(false);
				window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
				window->ShowWindow();
				ui::UiRect rect = window->GetPos();
				MoveWindow(window->GetHWND(), rect.left, rect.top, point->x, point->y, true);
				window->CenterWindow();
			}
		}

	}
	else if (uMsg == WM_RESULTWINDOWOPEN)
	{

		if (toastHwnd > 0)
		{
			CloseWindow(toastHwnd);
			toastHwnd = 0;
		}

		string* url = (string*)wParam;
		if (windowMap.count(*url) > 0 && windowMap[*url] > 0)
		{
			HWND   hwnd = windowMap[*url];

			::SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
			RECT rect;
			GetWindowRect(hwnd, &rect);
			::SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);

		}
		else
		{
			if (result_form == NULL)
			{
				result_form = new ResultForm();
				result_form->SetNavigateUrl(*url);
				result_form->SetDebugWindowHWND(m_hiddenWindow->GetHWND());
				result_form->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
				result_form->ShowWindow(false);
			}
			else
			{
				result_form->SetNavigateUrl(*url);
				::SendMessage(result_form->GetHWND(), WM_SYSCOMMAND, SC_RESTORE, NULL);

				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				result_form->ReloadNavigateUrl();

				RECT rect;
				GetWindowRect(result_form->GetHWND(), &rect);
				::SetWindowPos(result_form->GetHWND(), HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);
			}

			ResultForm::g_main_hwnd = GetHWND();
			windowMap[*url] = result_form->GetHWND();

		}
	}
	else if (uMsg == WM_RESULTWINDOWCLOSE)
	{
		HWND  hwnd = (HWND)wParam;

		map<string, HWND>::iterator iter;//定义一个迭代指针iter
		for (iter = windowMap.begin(); iter != windowMap.end(); iter++)
		{
			if (iter->second == hwnd)
			{
				iter->second = 0;

				result_form = NULL;
			}
		}
	}
	else if (uMsg == WM_OPENRWCLIENT)
	{
		if (wParam == 1)
		{
			boost::thread rwThread(boost::bind(&CheckRWKnowledgeResultFun, (void*)this));
			rwThread.detach();
		}
		else
		{
			boost::thread rwThread(boost::bind(&CheckRWKnowledgeResultFun, (void*)this));
			rwThread.detach();
		}

	}
	else if (uMsg == WM_SETRWRECT)
	{

		boost::thread rwThread(&CheckRWKnowledgeSetWindowRect);
		rwThread.detach();

	}
	else if (uMsg == WM_MODIFYPASSWORD)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(5));

		GetLoginInfo();
	}
	else if (uMsg == WM_SHOWTOASTWINDOW)
	{
		toastHwnd = shared::Toast::ShowToast(_T("正在请求中，请稍候！"), 10000, NULL);
	}
	else if (uMsg == WM_ONCLOSENOTICE)
	{
		UpdateHideStateNoticeMenu();
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void BasicForm::CreateTrayMenu()
{
	//生成托盘菜单
	hMenu = CreatePopupMenu();
	//添加菜单,关键在于设置的一个标识符  WM_ONCLOSE 点击后会用到

	if (tool->GetDevToolConfig())
	{
		AppendMenu(hMenu, MF_UNCHECKED | MF_BYPOSITION, WM_ONSHOWNOTICE, _T("显示调试窗"));
		AppendMenu(hMenu, MF_CHECKED | MF_BYPOSITION, WM_ONHIDENOTICE, _T("隐藏调试窗"));

		AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	}

	AppendMenu(hMenu, MF_STRING | MF_BYPOSITION, WM_CLEARCACHE, _T("清除缓存"));
	if (tool->GetNeedLoginConfig())
	{
		AppendMenu(hMenu, MF_STRING | MF_BYPOSITION, WM_USERQUIT, _T("切换账号"));
	}

	AppendMenu(hMenu, MF_STRING, WM_WSRECONNECT, _T("离线重连"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(hMenu, MF_CHECKED, WM_ONSHOW, _T("显示主窗口"));
	AppendMenu(hMenu, MF_UNCHECKED, WM_ONMIN, _T("隐藏主窗口"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(hMenu, MF_STRING, WM_ONCLOSE, _T("退出"));
}

void BasicForm::AddTrayIcon()
{
	memset(&m_trayIcon, 0, sizeof(NOTIFYICONDATA));
	m_trayIcon.cbSize = sizeof(NOTIFYICONDATA);
	m_trayIcon.hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(133));
	m_trayIcon.hWnd = m_hWnd;
	lstrcpy(m_trayIcon.szTip, _T("辅助诊断助手"));
	m_trayIcon.uCallbackMessage = WM_SHOWTASK;
	m_trayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, &m_trayIcon);
	ShowWindow(SW_HIDE);
}


BOOL DownloadFiles(LPCWSTR url, LPCWSTR downloadPath)
{
	if (URLDownloadToFile(NULL, url, downloadPath, 0, 0) == S_OK)
		return true;
	else
		return false;
}

void  BasicForm::SetToolBtnUrl()
{
	for (int i = 0; i < g_value.size(); i++)
	{
		Json::Value item = g_value[i];
		if (item.isObject())
		{
			std::string toolUrl = "";
			std::string toolName = "";

			if (item.isMember("toolServerUrl"))
			{
				toolUrl = item["toolServerUrl"].asString();

				if (toolUrl.find("medical.c2cloud.cn:80") != string::npos)
				{
					toolUrl = shared::tools::replaceAllMark(toolUrl, "medical.c2cloud.cn:80", "medical.c2cloud.cn");
				}

				if (toolUrl.find("172.18.0.105:80") != string::npos)
				{
					toolUrl = shared::tools::replaceAllMark(toolUrl, "172.18.0.105:80", "172.18.0.105");
				}
			}

			if (item.isMember("toolName"))
			{
				toolName = item["toolName"].asString();
			}

			if (i == 0)
			{
				tool_btn1->SetToolTipText(nbase::UTF8ToUTF16(toolName));
				m_navUrl1 = toolUrl;
			}
			else if (i == 1)
			{
				tool_btn2->SetToolTipText(nbase::UTF8ToUTF16(toolName));
				m_navUrl2 = toolUrl;
			}
			else if (i == 2)
			{
				tool_btn3->SetToolTipText(nbase::UTF8ToUTF16(toolName));
				m_navUrl3 = toolUrl;
			}
			else if (i == 3)
			{
				tool_btn4->SetToolTipText(nbase::UTF8ToUTF16(toolName));
				m_navUrl4 = toolUrl;
			}
			else if (i == 4)
			{
				tool_btn5->SetToolTipText(nbase::UTF8ToUTF16(toolName));
				m_navUrl5 = toolUrl;
			}
			else if (i == 5)
			{
				tool_btn6->SetToolTipText(nbase::UTF8ToUTF16(toolName));
				m_navUrl6 = toolUrl;
			}
		}
	}
}
void  BasicForm::RequestToolConfigUrl()
{
	YLog log(YLog::INFO, "log.txt", YLog::ADD);

	std::string toolConfigUrl = tool->GetToolConfigUrl();
	if (!toolConfigUrl.empty())
	{
		Json::Value value = g_value;

		SetToolBtnUrl();

		std::string exePath = nbase::UTF16ToUTF8(QPath::GetAppPath());
		std::string imgPath = exePath + "resources\\themes\\default\\cef";
		if (!shared::tools::FilePathIsExist(imgPath, true))
		{
			log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("图片文件夹不存在"), imgPath);
			MessageBox(NULL, L"程序出现异常！", _T("辅助诊断助手"), MB_SYSTEMMODAL | MB_ICONERROR | MB_OK);

			//文件夹不存在退出程序
			//ExitApp();
			return;
		}

		try
		{

			string imageVector[6];
			string hotImageVector[6];
			string imagePathVector[6];
			string hotImagePathVector[6];

			for (unsigned int i = 0; i < value.size(); i++)
			{
				Json::Value item = value[i];
				if (item.isObject())
				{

					std::string toolImagePath = "";
					std::string toolImageHotPath = "";
					std::string toolUrl = "";
					std::string toolName = "";

					if (item.isMember("toolName"))
					{
						toolName = item["toolName"].asString();
					}

					if (item.isMember("toolServerImagePath"))
					{
						toolImagePath = item["toolServerImagePath"].asString();
					}

					if (item.isMember("toolServerHotImagePath"))
					{
						toolImageHotPath = item["toolServerHotImagePath"].asString();
					}

					if (item.isMember("toolServerUrl"))
					{
						toolUrl = item["toolServerUrl"].asString();
					}

					//log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("toolUrl"), toolUrl);
					//log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("toolImageHotPath"), toolImageHotPath);

					int pos = toolImagePath.find_last_of('/');
					if (pos < 0)
					{
						pos = 0;
					}
					string image(toolImagePath.substr(pos + 1));

					pos = toolImageHotPath.find_last_of('/');
					if (pos < 0)
					{
						pos = 0;
					}
					string hotImage(toolImageHotPath.substr(pos + 1));


					std::string btnImagePath = imgPath + "\\" + image;
					std::string btnHotImagePath = imgPath + "\\" + hotImage;


					imageVector[i] = image;
					hotImageVector[i] = hotImage;
					imagePathVector[i] = btnImagePath;
					hotImagePathVector[i] = btnHotImagePath;

					//不存在则下载
					if (shared::tools::FilePathIsExist(btnImagePath, false) == false)
					{
						//log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("toolImagePath"), toolImagePath);
						DownloadFiles(nbase::UTF8ToUTF16(toolImagePath).c_str(), nbase::UTF8ToUTF16(btnImagePath).c_str());
					}

					//不存在则下载
					if (shared::tools::FilePathIsExist(btnHotImagePath, false) == false)
					{
						//log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("toolImageHotPath"), toolImageHotPath);
						DownloadFiles(nbase::UTF8ToUTF16(toolImageHotPath).c_str(), nbase::UTF8ToUTF16(btnHotImagePath).c_str());
					}

				}

			}
			//SetCurrentDirectory(stringToLPCWSTR(exePath));


			for (int j = 0; j < 6; j++)
			{
				string image = imageVector[j];
				string hotImage = hotImageVector[j];
				string imagePath = imagePathVector[j];
				string hotImagePath = hotImagePathVector[j];

				if (shared::tools::FilePathIsExist(imagePath, false))
				{
					if (j == 0)
					{
						tool_btn1->SetStateImage(ui::ControlStateType::kControlStateNormal, nbase::UTF8ToUTF16(image));
					}
					else if (j == 1)
					{
						tool_btn2->SetStateImage(ui::ControlStateType::kControlStateNormal, nbase::UTF8ToUTF16(image));
					}
					else if (j == 2)
					{
						tool_btn3->SetStateImage(ui::ControlStateType::kControlStateNormal, nbase::UTF8ToUTF16(image));
					}
					else if (j == 3)
					{
						tool_btn4->SetStateImage(ui::ControlStateType::kControlStateNormal, nbase::UTF8ToUTF16(image));
					}
					else if (j == 4)
					{
						tool_btn5->SetStateImage(ui::ControlStateType::kControlStateNormal, nbase::UTF8ToUTF16(image));
					}
					else if (j == 5)
					{
						tool_btn6->SetStateImage(ui::ControlStateType::kControlStateNormal, nbase::UTF8ToUTF16(image));
					}
				}

				if (shared::tools::FilePathIsExist(hotImagePath, false))
				{
					if (j == 0)
					{
						tool_btn1->SetStateImage(ui::ControlStateType::kControlStateHot, nbase::UTF8ToUTF16(hotImage));
					}
					else if (j == 1)
					{
						tool_btn2->SetStateImage(ui::ControlStateType::kControlStateHot, nbase::UTF8ToUTF16(hotImage));
					}
					else if (j == 2)
					{
						tool_btn3->SetStateImage(ui::ControlStateType::kControlStateHot, nbase::UTF8ToUTF16(hotImage));
					}
					else if (j == 3)
					{
						tool_btn4->SetStateImage(ui::ControlStateType::kControlStateHot, nbase::UTF8ToUTF16(hotImage));
					}
					else if (j == 4)
					{
						tool_btn5->SetStateImage(ui::ControlStateType::kControlStateHot, nbase::UTF8ToUTF16(hotImage));
					}
					else if (j == 5)
					{
						tool_btn6->SetStateImage(ui::ControlStateType::kControlStateHot, nbase::UTF8ToUTF16(hotImage));
					}
				}
			}

		}
		catch (...)
		{

		}

	}
	else//不使用接口获取配置则读取配置文件的配置
	{
		if (!tool->GetNavigateUrlEx("navigateUrl1").empty())
			m_navUrl1 = tool->GetNavigateUrlEx("navigateUrl1");

		if (!tool->GetNavigateUrlEx("navigateUrl2").empty())
			m_navUrl2 = tool->GetNavigateUrlEx("navigateUrl2");

		if (!tool->GetNavigateUrlEx("navigateUrl3").empty())
			m_navUrl3 = tool->GetNavigateUrlEx("navigateUrl3");

		if (!tool->GetNavigateUrlEx("navigateUrl4").empty())
			m_navUrl4 = tool->GetNavigateUrlEx("navigateUrl4");

		if (!tool->GetNavigateUrlEx("navigateUrl5").empty())
			m_navUrl5 = tool->GetNavigateUrlEx("navigateUrl5");

		if (!tool->GetNavigateUrlEx("navigateUrl6").empty())
			m_navUrl6 = tool->GetNavigateUrlEx("navigateUrl6");
	}
}