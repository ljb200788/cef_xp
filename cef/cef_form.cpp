#include "stdafx.h"
#include "cef_form.h"
#include "tool.h"
#include "ylog.h"
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <rapidjson\writer.h>
#include <rapidjson\stringbuffer.h>
#include <boost/thread.hpp>
#include <iostream>
#include <istream>
#include <ostream>

using namespace rapidjson;
using namespace std;


HWND CefForm::g_main_hwnd = 0;

//句柄页面映射MAP
std::map<string, HWND> g_windowMap;


//标记页面是否加载完成
bool    isLoadEnd = false;

const std::wstring CefForm::kClassName = L"Cef";
CefForm::CefForm()
{
	m_bWndClosed = false;
	m_bNeedMax = true;
	m_bNeedHidden = false;
}

CefForm::~CefForm()
{
}

void CefForm::SetNavigateUrl(std::string url)
{
	if (!url.empty())
	{
		m_strUrl.assign(url);
	}

}
std::wstring CefForm::GetSkinFolder()
{
	return L"cef";
}

std::wstring CefForm::GetSkinFile()
{
	return L"cef.xml";
}

std::wstring CefForm::GetWindowClassName() const
{
	return kClassName;
}

ui::Control* CefForm::CreateControl(const std::wstring& pstrClass)
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

void RefreshMessage(void*& data)
{
	int i = 0;
	while (true)
	{
		boost::this_thread::sleep(boost::posix_time::microseconds(1000));
		if (i >= 5 && isLoadEnd)
		{
			isLoadEnd = false;
			CefForm* ptrForm = (CefForm*)data;
			if (ptrForm != NULL)
			{
				if (ptrForm->GetHiddenFlag())
				{
					ptrForm->RefreshCefControl();
				}
			}
			return;
		}
		i++;
	}
}

void CefForm::InitWindow()
{
	// 监听鼠标单击事件
	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&CefForm::OnClicked, this, std::placeholders::_1));

	// 从 XML 中查找指定控件
	cef_control_		= dynamic_cast<ui::CefControlBase*>(FindControl(L"cef_control"));
	cef_control_dev_	= dynamic_cast<ui::CefControlBase*>(FindControl(L"cef_control_dev"));
	btn_dev_tool_		= dynamic_cast<ui::Button*>(FindControl(L"btn_dev_tool"));
	edit_url_			= dynamic_cast<ui::RichEdit*>(FindControl(L"edit_url"));

	// 设置输入框样式
	edit_url_->SetSelAllOnFocus(true);
	edit_url_->AttachReturn(nbase::Bind(&CefForm::OnNavigate, this, std::placeholders::_1));

	// 监听页面加载完毕通知
	cef_control_->AttachLoadEnd(nbase::Bind(&CefForm::OnLoadEnd, this, std::placeholders::_1));

	// 打开开发者工具
	cef_control_->AttachDevTools(cef_control_dev_);

	if (!nim_cef::CefManager::GetInstance()->IsEnableOffsetRender())
		cef_control_dev_->SetVisible(false);

	cef_control_dev_->SetVisible(false);

	label_Title_ = dynamic_cast<ui::Label*>(FindControl(L"title"));

	if (!m_strUrl.empty())
	{
		YLog log(YLog::INFO, "log.txt", YLog::ADD);
		log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("网址"), m_strUrl);
		cef_control_->LoadURL(m_strUrl);
	}
	else
	{
		// 加载皮肤目录下的 html 文件
		cef_control_->LoadURL(nbase::win32::GetCurrentModuleDirectory() + L"resources\\themes\\default\\cef\\cef.html");
	}

	btn_hidden_tool_ = dynamic_cast<ui::Button*>(FindControl(L"hiddenbtn"));
	btn_close_tool_ = dynamic_cast<ui::Button*>(FindControl(L"closebtn"));

	if (btn_hidden_tool_ != nullptr)
	{
		if (!m_bNeedHidden)
		{
			btn_hidden_tool_->SetVisible(false);
			btn_close_tool_->SetVisible(true);
		}
		else
		{
			btn_hidden_tool_->SetVisible(true);
			btn_close_tool_->SetVisible(false);
		}
	}

	if (m_bNeedHidden)
	{
		boost::thread refresh_thread(boost::bind(&RefreshMessage, (void*)this));
		refresh_thread.detach();
	}

	if (m_bNeedMax)
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
	}

	SetIcon(107);
}

LRESULT CefForm::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CefForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	nim_cef::CefManager::GetInstance()->PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

bool CefForm::OnClicked(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();

	if (name == L"btn_dev_tool")
	{
		if (cef_control_->IsAttachedDevTools())
		{
			cef_control_->DettachDevTools();
		}
		else
		{
			cef_control_->AttachDevTools(cef_control_dev_);
		}

		if (nim_cef::CefManager::GetInstance()->IsEnableOffsetRender())
		{
			cef_control_dev_->SetVisible(cef_control_->IsAttachedDevTools());
		}
	}
	else if (name == L"btn_back")
	{
		cef_control_->GoBack();
	}
	else if (name == L"btn_forward")
	{
		cef_control_->GoForward();
	}
	else if (name == L"btn_navigate")
	{
		OnNavigate(nullptr);
	}
	else if (name == L"btn_refresh")
	{
		cef_control_->Refresh();
	}

	return true;
}

void  CefForm::SetMaxFlag(bool  maxFlag)
{
	m_bNeedMax = maxFlag;
}

void   CefForm::SetMainWndHWND(HWND hwnd)
{
	m_hMainWnd = hwnd;
}

void  CefForm::RefreshCefControl()
{
	if (cef_control_)
	{
		cef_control_->Refresh();
	}
}

bool CefForm::OnNavigate(ui::EventArgs* msg)
{
	if (!edit_url_->GetText().empty())
	{
		cef_control_->LoadURL(edit_url_->GetText());
		cef_control_->SetFocus();
	}

	return true;
}

void CefForm::OnLoadEnd(int httpStatusCode)
{

	std::string para = "localStorage.setItem(\"isLogin\", \"true\")";
	cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);

	para = "localStorage.setItem(\"wsUser\", \"lance\")";
	//para += strUserName;
	//para += "\")";
	cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);
	FindControl(L"btn_back")->SetEnabled(cef_control_->CanGoBack());
	FindControl(L"btn_forward")->SetEnabled(cef_control_->CanGoForward());

	// 注册一个方法提供前端调用
	cef_control_->RegisterCppFunc(L"ShowMessageBox", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

		YLog log(YLog::INFO, "log.txt", YLog::ADD);
		log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("接收到的参数"), params);

		rapidjson::Document doc;
		doc.Parse(params.c_str());

		if (!doc.IsNull())
		{
			if (doc.HasMember("url"))
			{
				Value& m = doc["url"];
				if (!m.IsNull() && m.IsString())
				{
					std::string msg = m.GetString();
					if (msg.empty())
					{
						return;
					}
					//shared::Toast::ShowToast(nbase::UTF8ToUTF16(msg), 3000, GetHWND());
					//log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("容器页面"), cef_control_->GetUTF8URL());
					log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("通知中心打开服务页面"), msg);
					if (strcmp(msg.c_str(), "rw") == 0)
					{
						if (cef_control_->GetUTF8URL().find("rwBrowser") != string::npos)
						{
							::SendMessage(CefForm::g_main_hwnd, WM_OPENRWCLIENT, 0, 0);
						}
						else
						{
							::SendMessage(CefForm::g_main_hwnd, WM_OPENRWCLIENT, 1, 0);
						}
						return;
					}


					int pos = msg.find_last_of('/');
					if (pos < 0)
					{
						pos = 0;
					}
					string s(msg.substr(pos + 1));

					int _pos = msg.find_last_of('?');
					if (_pos < 0)
					{
						_pos = 0;
					}
					string _s(msg.substr(0, _pos));

					//log.W(filename(__FILE__), __LINE__, YLog::INFO,shared::tools::UtfToString("文件路径"), s);
					bool needDownLoad = false;

					if (_pos > 0)
					{
						int pos1 = _s.find_last_of('/');
						if (pos1 < 0)
						{
							pos1 = 0;
						}
						string s1(_s.substr(pos1 + 1));
						if (s1.find(".") != string::npos)
						{
							needDownLoad = true;
						}
					}
					else
					{
						if (s.find(".") != string::npos && s.find(".html") == string::npos)
						{
							needDownLoad = true;
						}
					}

					if (needDownLoad)
					{
						shared::tools::SafeOpenUrlEx(msg, SW_SHOWNORMAL);
					}
					else
					{
						if (g_windowMap.count(msg) > 0 && g_windowMap[msg] > 0)
						{
							log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("窗口句柄"), g_windowMap[msg]);

							::SendMessage(g_windowMap[msg], WM_SYSCOMMAND, SC_RESTORE, NULL);
							RECT rect;
							GetWindowRect(g_windowMap[msg], &rect);
							::SetWindowPos(g_windowMap[msg], HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);

							::SendMessage(CefForm::g_main_hwnd, WM_OPENEXISTCEFWINDOW, (int)g_windowMap[msg], 0);
						}
						else
						{

							CefForm* window = new CefForm();
							window->SetNavigateUrl(msg);
							window->SetMaxFlag(false);
							window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
							window->CenterWindow();
							window->ShowWindow();

							g_windowMap[msg] = window->GetHWND();

							int width = 0;
							int height = 0;
							if (doc.HasMember("width"))
							{
								Value& m = doc["width"];
								if (!m.IsNull() && m.IsInt())
								{
									width = m.GetInt() + 50;
								}
							}

							if (doc.HasMember("height"))
							{
								Value& m = doc["height"];
								if (!m.IsNull() && m.IsInt())
								{
									height = m.GetInt() + 35;
								}
							}

							//根据参数定制化页面的默认大小
							if (width > 0 && height > 0)
							{
								ui::UiRect rect = window->GetPos();
								MoveWindow(window->GetHWND(), rect.left, rect.top, width, height, true);
								window->CenterWindow();
							}
						}

					}
				}
			}

			if (doc.HasMember("message"))
			{
				Value& m = doc["message"];
				if (!m.IsNull() && m.IsString())
				{
					std::string msg = m.GetString();
				}
			}
		}
	}));


	// 注册一个方法提供前端调用
	cef_control_->RegisterCppFunc(L"ShowWebsocketMessageBox", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		//shared::Toast::ShowToast(nbase::UTF8ToUTF16(params), 3000, GetHWND());
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

		std::string paraMsg = params;
		if (paraMsg.find("index.html") != string::npos)
		{
			shared::tools::deleteAllMark(paraMsg, "index.html");
		}

		YLog log(YLog::DEBUG, "log.txt", YLog::ADD);
		log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("接收到的参数"), paraMsg);

		rapidjson::Document doc;
		doc.Parse(paraMsg.c_str());

		if (!doc.IsNull())
		{
			if (doc.HasMember("url"))
			{
				Value& m = doc["url"];
				if (!m.IsNull() && m.IsString())
				{
					std::string msg = m.GetString();
					if (msg.empty())
					{
						return;
					}

					log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("通知中心打开服务页面"), msg);

					/*
					if (g_unique_window == NULL)
					{
					g_unique_window = new CefForm();
					g_unique_window->SetHiddenFlag(true);
					g_unique_window->Create(NULL, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
					}

					g_unique_window->SetNavigateUrl(msg);
					g_unique_window->SetMaxFlag(false);
					g_unique_window->RefreshNavigateUrl();
					g_unique_window->ShowWindow();
					//g_unique_window->ToTopMost(true);
					*/
					int width = 0;
					int height = 0;
					if (doc.HasMember("width"))
					{
						Value& m = doc["width"];
						if (!m.IsNull() && m.IsInt())
						{
							width = m.GetInt() + 50;
						}
					}

					if (doc.HasMember("height"))
					{
						Value& m = doc["height"];
						if (!m.IsNull() && m.IsInt())
						{
							height = m.GetInt() + 35;
						}
					}

					//获取屏幕大小
					int m_iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
					int m_iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

					if (width >= (m_iScreenWidth - 100))
					{
						width = m_iScreenWidth - 100;
					}

					//根据参数定制化页面的默认大小
					//if (width > 0 && height > 0)
					//{
					//	ui::UiRect rect = g_unique_window->GetPos();
					//	MoveWindow(g_unique_window->GetHWND(), m_iScreenWidth-width-100, (m_iScreenHeight-height)/2, width, height, true);
					//	g_unique_window->ShowWindow();
					//}

					::SendMessage(CefForm::g_main_hwnd, WM_RESULTWINDOWOPEN, (WPARAM)&msg, 0);
				}
			}

			if (doc.HasMember("message"))
			{
				Value& m = doc["message"];
				if (!m.IsNull() && m.IsString())
				{
					std::string msg = m.GetString();
				}
			}
		}
	}));


	SetTaskbarTitle(cef_control_->GetTitle());
	if (label_Title_)
	{
		label_Title_->SetText(cef_control_->GetTitle());
	}

	isLoadEnd = true;
}

void	CefForm::RefreshNavigateUrl()
{
	if (!m_strUrl.empty())
	{
		if (cef_control_)
		{
			cef_control_->LoadURL(m_strUrl);
		}
	}
}

void	CefForm::SetMinBtnHidden(bool isVisible)
{
	ui::Button* btn_min_ = dynamic_cast<ui::Button*>(FindControl(L"minbtn"));
	if (btn_min_)
	{
		btn_min_->SetVisible(isVisible);
	}
}

bool CefForm::GetHiddenFlag()
{
	return m_bNeedHidden;
}
void CefForm::SetHiddenFlag(bool  hiddenFlag)
{
	m_bNeedHidden = hiddenFlag;
}

void	CefForm::SetMaxBtnHidden(bool isVisible)
{
	ui::Button* btn_max_ = dynamic_cast<ui::Button*>(FindControl(L"maxbtn"));
	if (btn_max_)
	{
		btn_max_->SetVisible(isVisible);
	}
}
