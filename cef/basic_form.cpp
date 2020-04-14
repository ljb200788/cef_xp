#include "stdafx.h"
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

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

const std::wstring BasicForm::kClassName = L"Basic";

XMLConfigTool* tool = new XMLConfigTool();

ResultForm* result_form = NULL;
HWND toastHwnd = 0;
bool  isUserOnline = true;
//托盘菜单
//win32程序使用的是HMENU，如果是MFC程序可以使用CMenu
HMENU hMenu;

Json::Value g_value;

volatile bool isTreadState = true;
volatile bool isTreadMessageState = true;
volatile bool isTreadRWKnowledgeState = true;
volatile bool isTreadRecordState = true;

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

using namespace std;


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
	}

	tool_btn1 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting1"));
	tool_btn2 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting2"));
	tool_btn3 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting3"));
	tool_btn4 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting4"));
	tool_btn5 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting5"));
	tool_btn6 = dynamic_cast<ui::Button*>(FindControl(L"proxy_setting6"));

	AddTrayIcon();

	SetIcon(107);

	CreateTrayMenu();

	SetTaskbarTitle(L"辅助诊断助手");

	boost::thread toolConfigThread(boost::bind(&GetToolConfigThreadFun, (void*)this));
	toolConfigThread.detach();

	ShowDevTool();

	CefForm::g_main_hwnd = GetHWND();
}

void BasicForm::ExitApp()
{

	UnregisterHotKey(GetHWND(), hotkeyId8);
	UnregisterHotKey(GetHWND(), hotkeyId7);
	UnregisterHotKey(GetHWND(), hotkeyId6);
	UnregisterHotKey(GetHWND(), hotkeyId5);
	UnregisterHotKey(GetHWND(), hotkeyId4);
	UnregisterHotKey(GetHWND(), hotkeyId3);
	UnregisterHotKey(GetHWND(), hotkeyId1);

	isTreadState = false;
	isTreadMessageState = false;
	isTreadRWKnowledgeState = false;

	m_trayIcon.hIcon = NULL;
	Shell_NotifyIcon(NIM_DELETE, &m_trayIcon);

	if (serverUtil)
	{
		delete serverUtil;
	}

	if (tool)
	{
		delete tool;
	}

	PostQuitMessage(0L);
}

LRESULT BasicForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ExitApp();
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
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
			//MessageBox(0, _T("网络未连接！"), _T("智能诊断精灵"), MB_OK);
		}
	}
	else if (name == L"proxy_setting2")
	{
		//MsgboxCallback cb = nbase::Bind(&BasicForm::OnMsgBoxCallback, this, std::placeholders::_1,"cef");
		//ShowMsgBox(GetHWND(), cb, L"确认打开窗口！", false, L"提示", true, L"确定", true, L"取消", true);
		//ShowMsgBox(GetHWND(), MsgboxCallback());

		//MessageBox(NULL, _T("持续开发中，敬请期待！"), _T("辅助诊断助手"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		//return true;
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
			//MessageBox(0, _T("网络未连接！"), _T("智能诊断精灵"), MB_OK);
		}
	}
	else if (name == L"proxy_setting3")
	{
		//MessageBox(NULL, _T("持续开发中，敬请期待！"), _T("辅助诊断助手"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		//return true;

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

LRESULT BasicForm::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	//如果在图标中单击左键则还原
	if (lParam == WM_LBUTTONDOWN)
	{
		ShowWindow(true);
		::SetFocus(m_hWnd);
		SetForegroundWindow(m_hWnd);
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

			m_trayIcon.hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(130));
			Shell_NotifyIcon(NIM_MODIFY, &m_trayIcon);

			//wstring name = nbase::UTF8ToUTF16(CefForm::strUserName.c_str());
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

				m_trayIcon.hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(130));
				Shell_NotifyIcon(NIM_MODIFY, &m_trayIcon);

				ShowBalloonTip(L"已离线！", L"辅助诊断助手", NIIF_INFO, 3000);
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
				result_form->ReloadNavigateUrl();
				::SendMessage(result_form->GetHWND(), WM_SYSCOMMAND, SC_RESTORE, NULL);
				RECT rect;
				GetWindowRect(result_form->GetHWND(), &rect);
				::SetWindowPos(result_form->GetHWND(), HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);
			}

			ResultForm::g_main_hwnd = GetHWND();
			windowMap[*url] = result_form->GetHWND();

		}
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
	m_trayIcon.hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(131));
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