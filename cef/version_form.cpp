#include "stdafx.h"
#include <fstream>
#include <boost\thread\thread.hpp>
#include "version_form.h"
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <rapidjson\writer.h>
#include <rapidjson\stringbuffer.h>
#include "XMLConfigTool.h"
#include "ylog.h"
#include "WininetHttp.h"
#include <windows.h>
#include <shellapi.h>
#include "login_form.h"
#include "basic_form.h"

using namespace ui;
using namespace boost;
using namespace rapidjson;

string VersionForm::user_name = "";


VersionForm::VersionForm()
{
}

VersionForm::~VersionForm()
{
}

std::wstring VersionForm::GetSkinFolder()
{
	return L"cef";
}

std::wstring VersionForm::GetSkinFile()
{
	return L"version.xml";
}

std::wstring VersionForm::GetWindowClassName() const
{
	return kClassName;
}

const std::wstring VersionForm::kClassName = L"Version";

ui::Control* VersionForm::CreateControl(const std::wstring& pstrClass)
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

void  VersionForm::ShowMainWindow()
{
	XMLConfigTool* tool = new XMLConfigTool();
	bool needLogin = tool->GetNeedLoginConfig();
	if (needLogin)
	{
		LoginForm* window = new LoginForm();
		window->Create(NULL, LoginForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
		window->ShowWindow();
	}
	else
	{
		BasicForm* window = new BasicForm();
		window->Create(NULL, BasicForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
		window->ShowWindow();
	}

	delete tool;

	ShowWindow(SW_HIDE);
}
void VersionForm::InitWindow()
{
	// 监听鼠标单击事件
	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&VersionForm::OnClicked, this, std::placeholders::_1));

	SetIcon(107);
	SetTaskbarTitle(L"新版本信息");

	XMLConfigTool* tool = new XMLConfigTool();
	std::string g_update_url = tool->GetUpdateUrl();
	if (g_update_url.empty())
	{
		g_update_url = "http://medical.c2cloud.cn/kgms/ylkg/v1/versionRelease/selectOne";
	}
	std::string url = g_update_url + "?appCode=2&versionNum="+ versionNum;

	YLog log(YLog::INFO, "log.txt", YLog::ADD);

	CWininetHttp netHttp;
	std::string ret = netHttp.RequestJsonInfo(url, Hr_Post, "Content-Type:application/json;charset=utf-8", "{}");

	ui::CefControlBase* cef_control_ = dynamic_cast<ui::CefControlBase*>(FindControl(L"cef_control"));
	if (cef_control_)
	{
		cef_control_->LoadURL(QPath::GetAppPath() + L"resources\\themes\\default\\cef\\version.html");
	}

	if (!ret.empty())
	{
		log.W(__FILE__, __LINE__, YLog::INFO, "Result", ret);
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
						if (doc.HasMember("version"))
						{

							string updateRemark = "";
							int versionNum = 0;

							Value& version = doc["version"];
							if (version.HasMember("versionNum"))
							{
								if (version["versionNum"].IsInt())
								{
									versionNum = version["versionNum"].GetInt();
								}
							}

							if (version.HasMember("updateRemark"))
							{
								if (version["updateRemark"].IsString())
								{
									updateRemark = version["updateRemark"].GetString();
								}
							}

							ui::Label* versionNumLabel = dynamic_cast<ui::Label*>(FindControl(L"version_num_label"));
							if (versionNumLabel)
							{
								wstring str = L"发现当前最新版本V" + std::to_wstring(versionNum);
								versionNumLabel->SetText(str);
							}

							ui::Label* versionDesLabel = dynamic_cast<ui::Label*>(FindControl(L"version_des_label"));
							if (versionDesLabel)
							{
								versionDesLabel->SetText(nbase::UTF8ToUTF16(updateRemark));
							}
						}	
					}
				}
				else
				{
					ShowMainWindow();
				}
			}
		}
	}
	else
	{
		ShowMainWindow();
	}
}

bool VersionForm::Notify(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();
	return true;
}

LRESULT VersionForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ShowMainWindow();
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

bool VersionForm::OnClicked(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(__FILE__, __LINE__, YLog::INFO, "OnClicked", nbase::UTF16ToUTF8(name));

	if (name == L"btn_yes_update")
	{

		XMLConfigTool* tool = new XMLConfigTool();
		std::string g_update_url = tool->GetUpdateUrl();
		std::string url = g_update_url + "?appCode=2&versionNum=" + versionNum;
		delete tool;

		std::string exePath = nbase::UTF16ToUTF8(QPath::GetAppPath());
		std::string updateExePath = exePath + "DiagnosticAssistantUpdate.exe";

		log.W(__FILE__, __LINE__, YLog::INFO, "OnClicked", updateExePath);

		ShellExecute(NULL, _T("open"), nbase::UTF8ToUTF16(updateExePath.c_str()).c_str(), nbase::UTF8ToUTF16(url.c_str()).c_str(), NULL, SW_SHOWNORMAL);

		PostQuitMessage(0L);
	}
	else if (name == L"btn_no_update")
	{
		ShowMainWindow();
	}
	return true;
}