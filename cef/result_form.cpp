#include "stdafx.h"
#include <fstream>
#include <boost\thread\thread.hpp>
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <rapidjson\writer.h>
#include <rapidjson\stringbuffer.h>
#include "XMLConfigTool.h"
#include "ylog.h"
#include "WininetHttp.h"
#include <windows.h>
#include <shellapi.h>
#include "basic_form.h"
#include "result_form.h"
#include <stdio.h> 
#include <stdlib.h>
#include <json/value.h>
#include <json/reader.h>
#include "tool.h"
#include "RWData.h"


using namespace ui;
using namespace rapidjson;
using namespace std;

string ResultForm::user_name = "";

HWND ResultForm::g_main_hwnd = 0;

const std::wstring ResultForm::kClassName = L"result";


// 获取屏幕大小
int m_iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
int m_iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

ResultForm::ResultForm()
{
	m_debug_wnd = 0;
}

ResultForm::~ResultForm()
{
}

std::wstring ResultForm::GetSkinFolder()
{
	return L"cef";
}

std::wstring ResultForm::GetSkinFile()
{
	return L"result.xml";
}

std::wstring ResultForm::GetWindowClassName() const
{
	return kClassName;
}

void	ResultForm::SetDebugWindowHWND(HWND wnd)
{
	m_debug_wnd = wnd;
}

ui::Control* ResultForm::CreateControl(const std::wstring& pstrClass)
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


void MoveWindowThreadNarrowFun(void*& data)
{
	int i = 0;

	ResultForm* ptrForm = (ResultForm*)data;
	if (ptrForm != NULL)
	{
		RECT rect;
		GetWindowRect(ptrForm->GetHWND(), &rect);

		// 获取屏幕大小
		int m_iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if ((rect.left + 40 * i + 460) > m_iScreenWidth)
			{
				return;
			}
			SetWindowPos(ptrForm->GetHWND(), HWND_TOP, rect.left + 40* i, rect.top, 320, rect.bottom - rect.top, SWP_SHOWWINDOW);
			i++;
		}
	}

}

void MoveWindowFromRightThreadFun(void*& data)
{
	ResultForm* ptrForm = (ResultForm*)data;
	if (ptrForm != NULL)
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		ShowWindow(ptrForm->GetHWND(),SWP_SHOWWINDOW);

		int top = (m_iScreenHeight - 630) / 2;

		SetWindowPos(ptrForm->GetHWND(), HWND_TOP, m_iScreenWidth+50, top, 320, 630, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
		int i = 0;

		RECT rect;
		GetWindowRect(ptrForm->GetHWND(), &rect);

		while (true)
		{
			i++;
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			if ((rect.left - 20 * i + 440) < m_iScreenWidth)
			{
				return;
			}

			int top = (m_iScreenHeight - 630) / 2;
			SetWindowPos(ptrForm->GetHWND(), HWND_TOP, rect.left-20*i, top, 320, 630, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
		}
	}
}

void MoveWindowThreadZoomFun(void*& data)
{
	int i = 0;

	ResultForm* ptrForm = (ResultForm*)data;
	if (ptrForm != NULL )
	{
		RECT rect;
		GetWindowRect(ptrForm->GetHWND(), &rect);

		if ((rect.right - rect.left) > 1100)
		{
			return;
		}

		SetWindowPos(ptrForm->GetHWND(), HWND_TOP, rect.left, rect.top, 1100, rect.bottom - rect.top, SWP_SHOWWINDOW);

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			RECT rect1;
			GetWindowRect(ptrForm->GetHWND(), &rect1);
			if ((rect1.left + 550) <= m_iScreenWidth/2)
			{
				return;
			}

			SetWindowPos(ptrForm->GetHWND(), HWND_TOP, rect.left - 40 * i, rect.top, 1100, rect.bottom - rect.top, SWP_SHOWWINDOW);
			i++;
		}
	}

}

void ResultForm::InitWindow()
{
	// 监听鼠标单击事件
	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&ResultForm::OnClicked, this, std::placeholders::_1));

	SetIcon(107);
	SetTaskbarTitle(L"辅助诊断分析页面");
	cef_control_ = dynamic_cast<ui::CefControlBase*>(FindControl(L"cef_control"));
	cef_control_dev_ = dynamic_cast<ui::CefControlBase*>(FindControl(L"cef_control_dev"));
	if (cef_control_)
	{
		cef_control_->AttachDevTools(cef_control_dev_);
		cef_control_->LoadURL(m_strUrl);
		// 监听页面加载完毕通知
		cef_control_->AttachLoadEnd(nbase::Bind(&ResultForm::OnLoadEnd, this, std::placeholders::_1));
		cef_control_->AttachUrlChange(nbase::Bind(&ResultForm::OnUrlChange, this, std::placeholders::_1));
	}

	if (!nim_cef::CefManager::GetInstance()->IsEnableOffsetRender())
		cef_control_dev_->SetVisible(false);

	cef_control_dev_->SetVisible(false);

	SetForegroundWindow(GetHWND());

	boost::thread right_thread(boost::bind(&MoveWindowFromRightThreadFun, (void*)this));
	right_thread.detach();

	SetIcon(107);
}


void ResultForm::SetNavigateUrl(std::string url)
{
	if (!url.empty())
	{
		m_strUrl = url;
	}
}

void	ResultForm::ReloadNavigateUrl()
{
	if (!m_strUrl.empty())
	{
		if (cef_control_)
		{
			cef_control_->LoadURL(m_strUrl);
		}
	}
}

LRESULT ResultForm::OnNcLButtonDbClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::SetFocus(GetHWND());
	return __super::OnNcLButtonDbClick(uMsg, wParam, lParam, bHandled);
}

void	ResultForm::OnLoadEnd(int httpStatusCode)
{
	cef_control_->SetFocus();
	// 注册一个方法提供前端调用
	cef_control_->RegisterCppFunc(L"ShowMessageBox", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

		YLog log(YLog::DEBUG, "log.txt", YLog::ADD);
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

						POINT point;
						point.x = width;
						point.y = height;

						if (g_main_hwnd != 0)
						{
							::SendMessage(g_main_hwnd, WM_OPENCEFWINDOWFROMRESULT, (WPARAM)&msg, (LPARAM)&point);
						}
					}
				}
			}
		}
	}));

	// 注册一个方法提供前端调用，展开窗口
	cef_control_->RegisterCppFunc(L"UnFoldMessageBox", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

		//YLog log(YLog::INFO, "log.txt", YLog::ADD);
		//log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("UnFoldMessageBox接收到的参数"), params);

		boost::thread zoom_thread(boost::bind(&MoveWindowThreadZoomFun, (void*)this));
		zoom_thread.detach();	

	}));

	cef_control_->RegisterCppFunc(L"callCsPlugin", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

		DWORD pid = GetProcessIDByName(L"RWKnowledge.exe");
		if (pid == 0)
		{
			HWND hwnd = shared::Toast::ShowToast(L"请先启动人卫客户端！", 3000, GetHWND());
			SetForegroundWindow(hwnd);
			return;
		}
		//::SendMessage(CefForm::g_main_hwnd, WM_OPENRWCLIENT, 0, 0);

		YLog log(YLog::INFO, "log.txt", YLog::ADD);
		log.W(filename(__FILE__), __LINE__, YLog::DEBUG, "callCsPlugin", params);

		Json::Reader reader;
		Json::Value result;

		if (reader.parse(params, result))
		{
			if (result.isMember("type"))
			{
				if (result["type"].isString())
				{
					string type = result["type"].asString();
					if (type.find("SearchRWKnowledge") != string::npos)
					{
						if (result.isMember("params"))
						{
							if (result["params"].isObject())
							{
								string rwname = result["params"]["rwname"].asString();
								string category = result["params"]["category"].asString();
								string info_attribute = result["params"]["info_attribute"].asString();

								RWData* data = new RWData();
								data->rw_rwname = rwname;
								data->rw_category = category;
								data->rw_info_attribute = info_attribute;

								::SendMessage(m_debug_wnd, WM_SENDRWMESSAGE, (WPARAM)data, 0);

								delete data;
								data = NULL;
							}
						}

					}
					else if (type.find("dianosisBasis") != string::npos)
					{
						if (result.isMember("params"))
						{
							if (result["params"].isObject())
							{
								string rwname = result["params"]["name"].asString();
								string category = "disease";
								string info_attribute = shared::tools::GBKToUTF8("诊断");

								RWData* data = new RWData();
								data->rw_rwname = rwname;
								data->rw_category = category;
								data->rw_info_attribute = info_attribute;
								data->rw_mode = 1;

								::SendMessage(m_debug_wnd, WM_SENDRWMESSAGE, (WPARAM)data, 0);

								delete data;
								data = NULL;

							}
						}
					}
					else if (type.find("identifyDiagnosis") != string::npos)
					{
						if (result.isMember("params"))
						{
							if (result["params"].isObject())
							{
								string rwname = result["params"]["name"].asString();
								string category = "disease";
								string info_attribute = shared::tools::GBKToUTF8("鉴别诊断");

								RWData* data = new RWData();
								data->rw_rwname = rwname;
								data->rw_category = category;
								data->rw_info_attribute = info_attribute;
								data->rw_mode = 1;

								::SendMessage(m_debug_wnd, WM_SENDRWMESSAGE, (WPARAM)data, 0);

								delete data;
								data = NULL;
							}
						}
					}
					else if (type.find("treatmentOptions") != string::npos)
					{
						if (result.isMember("params"))
						{
							if (result["params"].isObject())
							{
								string rwname = result["params"]["name"].asString();
								string category = "disease";
								string info_attribute = shared::tools::GBKToUTF8("治疗");

								RWData* data = new RWData();
								data->rw_rwname = rwname;
								data->rw_category = category;
								data->rw_info_attribute = info_attribute;
								data->rw_mode = 1;

								::SendMessage(m_debug_wnd, WM_SENDRWMESSAGE, (WPARAM)data, 0);

								delete data;
								data = NULL;
							}
						}
					}
				}
				
			}
		}


	}));

	ui::Label* label_Title_ = dynamic_cast<ui::Label*>(FindControl(L"title"));
	if (label_Title_ && cef_control_)
	{
		label_Title_->SetText(cef_control_->GetTitle());
	}
}

void	ResultForm::OnUrlChange(const std::wstring& url)
{
	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, "OnUrlChange", nbase::UTF16ToUTF8(url));

	string pageUrl = nbase::UTF16ToUTF8(url);
	if (pageUrl.find("status=unfold") != string::npos)
	{
		boost::thread zoom_thread(boost::bind(&MoveWindowThreadZoomFun, (void*)this));
		zoom_thread.detach();
	}
	else
	{
		RECT rect;
		GetWindowRect(GetHWND(), &rect);
		if ((rect.right - rect.left) > 1000)
		{
			boost::thread narrow_thread(boost::bind(&MoveWindowThreadNarrowFun, (void*)this));
			narrow_thread.detach();
		}

	}
	
}

LRESULT	ResultForm::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	if (wParam == VK_F5)
	{
		bHandled = TRUE;
		if (cef_control_)
		{
			cef_control_->Refresh();
		}
	}

	if (wParam == VK_F12)
	{
		bHandled = TRUE;

		if (cef_control_dev_->IsVisible())
		{
			cef_control_dev_->SetVisible(false);
		}
		else
		{
			cef_control_dev_->SetVisible(true);
		}
	}

	return __super::OnKeyDown(uMsg, wParam, lParam, bHandled);
}

bool ResultForm::Notify(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();
	return true;
}

LRESULT ResultForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (g_main_hwnd != 0)
	{
		::SendMessage(g_main_hwnd, WM_RESULTWINDOWCLOSE, (int)GetHWND(), 0);
	}

	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

bool ResultForm::OnClicked(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();
	return true;
}