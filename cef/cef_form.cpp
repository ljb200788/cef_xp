#include "stdafx.h"
#include "cef_form.h"
#include "login_form.h"
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
#include "RWData.h"
#include <json\value.h>
#include <json\reader.h> 
#include "iphlpapi.h"
#include "WininetHttp.h"

#pragma comment(lib, "iphlpapi.lib") 
#pragma comment(lib, "winmm.lib") 

using namespace rapidjson;
using namespace std;


HWND CefForm::g_main_hwnd = 0;
string CefForm::strUserName = "monkey";
string	macAddress = "";

//句柄页面映射MAP
map<string, HWND> g_windowMap;

CefForm* CefForm::g_ptr_rw_cef = NULL;

//标记页面是否加载完成
bool    isLoadEnd = false;

const wstring CefForm::kClassName = L"Cef";

CefForm::CefForm()
{
	m_bWndClosed = false;
	m_bNeedMax = true;
	m_bNeedHidden = false;

	m_hMainWnd = 0;
	m_strUrl = "";
	m_question = "";
	m_bRegisterFlag = false;
	m_bQaFlag = false;

	m_bIsFirst = true;
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
		cef_control_->LoadURL(nbase::win32::GetCurrentModuleDirectory() + L"resources\\themes\\default\\cef\\404.html");
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

/*
返回别名列表
*/
Json::Value  GetAliasArray(string term, string category)
{

	Json::Value  alias_value;
	alias_value["label"] = term;
	alias_value["type"] = category;

	YLog log(YLog::INFO, "log.txt", YLog::ADD);

	XMLConfigTool* tool = new XMLConfigTool();
	std::string aliasUrl = tool->GetAliasServerUrl();
	delete tool;

	if (!aliasUrl.empty())
	{
		CWininetHttp netHttp;
		std::string aliasRet = netHttp.RequestJsonInfo(aliasUrl, Hr_Post,
			"Content-Type:application/json;charset=utf-8", alias_value.toStyledString());
		log.W(__FILE__, __LINE__, YLog::DEBUG, "aliasRet", aliasRet);

		Json::Reader aliasReader;
		Json::Value aliasResult;
		if (aliasReader.parse(aliasRet, aliasResult))
		{
			if (aliasResult.isMember("alias"))
			{
				if (aliasResult["alias"].isArray())
				{
					return aliasResult["alias"];
				}
			}
		}
	}

	Json::Value test;
	return test;
}

LRESULT CefForm::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	YLog log(YLog::INFO, "log.txt", YLog::ADD);

	if (uMsg == WM_SENDRWMESSAGE)
	{

		XMLConfigTool* tool = new XMLConfigTool();
		string rwServerUrl = tool->GetRwServerUrl();
		delete tool;
		if (!rwServerUrl.empty())
		{
			string category = "";
			string term = shared::tools::GBKToUTF8("阿莫西林");
			string info_attribute = "";

			RWData* data = (RWData*)wParam;
			if (data != NULL)
			{
				category = data->rw_category;
				term = data->rw_rwname;
				info_attribute = data->rw_info_attribute;
			}

			Json::Value  value;
			value["action"] = "search";
			value["term"] = term;
			value["category"] = category;
			value["case_or_no"] = "";

			string para = value.toStyledString();
			shared::tools::deleteAllMark(para, " ");
			shared::tools::deleteAllMark(para, "\r");
			shared::tools::deleteAllMark(para, "\n");

			CWininetHttp netHttp;
			std::string ret = netHttp.RequestJsonInfo(rwServerUrl, Hr_Post,
				"Content-Type:application/json;charset=utf-8", value.toStyledString());

			log.W(__FILE__, __LINE__, YLog::DEBUG, shared::tools::UtfToString("人卫请求参数"), para);
			log.W(__FILE__, __LINE__, YLog::DEBUG, shared::tools::UtfToString("人卫响应结果"), ret);

			Json::Reader reader;
			Json::Value result;

			if (reader.parse(ret, result))
			{
				if (result.isMember("message"))
				{
					if (result["message"].isString())
					{
						string message = result["message"].asString();
						if (message.find("success") != string::npos)
						{
							if (result.isMember("sug_list"))
							{
								if (result["sug_list"].isArray())
								{
									if (result["sug_list"].size() > 0)
									{
										Json::Value alias_array;
										if (!category.empty())
										{
											alias_array = GetAliasArray(term, category);
										}

										for (int i = 0; i < result["sug_list"].size(); i++)
										{
											if (result["sug_list"][i].isObject())
											{

												string title = result["sug_list"][i]["title"].asString();
												string rwId = result["sug_list"][i]["rwid"].asString();
												string _category = result["sug_list"][i]["category"].asString();

												Json::Value  value1;
												value1["rwid"] = rwId;
												value1["info_attribute"] = info_attribute.c_str();

												bool isFind = false;

												//没传送类别使用默认类别
												if (category.empty())
												{
													if (title == term)
													{
														isFind = true;
													}
												}
												else
												{
													if (alias_array.isArray())
													{
														if (alias_array.size() > 0)
														{
															for (int j = 0; j < alias_array.size(); j++)
															{
																string name = alias_array[j].asString();
																if (title == name)
																{
																	isFind = true;
																	break;
																}
															}
														}
													}

													if (title == term && _category == category)
													{
														isFind = true;
													}
												}

												if (isFind)
												{
													//value1["case_or_no"] = "";
													value1["category"] = _category;

													string data = value1.toStyledString();
													shared::tools::deleteAllMark(data, " ");
													shared::tools::deleteAllMark(data, "\r");
													shared::tools::deleteAllMark(data, "\n");

													log.W(__FILE__, __LINE__, YLog::DEBUG, shared::tools::GBKToUTF8("发给人卫ws的数据"), data);

													std::string para = "window.CDSS_FE.rwWs.send(\'" + data + "\')\0";
													log.W(__FILE__, __LINE__, YLog::DEBUG, shared::tools::GBKToUTF8("执行JS代码"), para);
													cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);
													break;
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
		}
	}
	else if (uMsg == WM_SENDQAMESSAGE)
	{

		ShowWindow(true, true);

		string* message = (string*)wParam;

		std::string para = "window.CDSS_FE.getRecordQuestion(\'" + *message + "\')\0";

		log.W(__FILE__, __LINE__, YLog::DEBUG, shared::tools::UtfToString("执行命令参数"), para);

		cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);


	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CefForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_bRegisterFlag)
	{
		ShowWindow(false);
		return 0;
	}

	if (g_windowMap.count(m_strUrl) > 0)
	{
		g_windowMap[m_strUrl] = 0;
	}

	if (m_strUrl.find("rwBrowser") != string::npos)
	{
		if (g_ptr_rw_cef == this)
		{
			g_ptr_rw_cef = NULL;
		}
	}
	m_bWndClosed = true;

	if (g_main_hwnd != 0)
	{
		::SendMessage(g_main_hwnd, WM_CEFWINDOWCLOSE, (int)GetHWND(), 0);
	}

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
	else if (name == L"hiddenbtn")
	{
		if (m_hMainWnd > 0)
		{
			::SendMessage(m_hMainWnd, WM_ONCLOSENOTICE, 0, 0);
		}

		ShowWindow(SW_HIDE);
	}

	return true;
}

void  CefForm::SetMaxFlag(bool  maxFlag)
{
	m_bNeedMax = maxFlag;
}

void CefForm::SetRegisterFlag(bool registerFlag)
{
	m_bRegisterFlag = registerFlag;
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

void getNetworkInfo()
{
	// PIP_ADAPTER_INFO struct contains network information
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	unsigned long adapter_size = sizeof(IP_ADAPTER_INFO);
	int ret = GetAdaptersInfo(pIpAdapterInfo, &adapter_size);

	if (ret == ERROR_BUFFER_OVERFLOW)
	{
		// overflow, use the output size to recreate the handler
		delete pIpAdapterInfo;
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[adapter_size];
		ret = GetAdaptersInfo(pIpAdapterInfo, &adapter_size);
	}

	if (ret == ERROR_SUCCESS)
	{
		int card_index = 0;

		// may have many cards, it saved in linklist
		while (pIpAdapterInfo)
		{
			std::cout << "---- " << "NetworkCard " << ++card_index << " ----" << std::endl;

			std::cout << "Network Card Name: " << pIpAdapterInfo->AdapterName << std::endl;
			std::cout << "Network Card Description: " << pIpAdapterInfo->Description << std::endl;

			// get IP, one card may have many IPs
			PIP_ADDR_STRING pIpAddr = &(pIpAdapterInfo->IpAddressList);
			while (pIpAddr)
			{
				char local_ip[128] = { 0 };
				strcpy_s(local_ip, 128, pIpAddr->IpAddress.String);
				std::cout << "Local IP: " << local_ip << std::endl;

				pIpAddr = pIpAddr->Next;
			}

			char local_mac[128] = { 0 };
			int char_index = 0;
			for (int i = 0; i < pIpAdapterInfo->AddressLength; i++)
			{
				char temp_str[10] = { 0 };
				sprintf_s(temp_str, "%02X-", pIpAdapterInfo->Address[i]); // X for uppercase, x for lowercase
				strcpy_s(local_mac + char_index, 10, temp_str);
				char_index += 3;
			}
			local_mac[17] = '\0'; // remove tail '-'

			std::cout << "Local Mac: " << local_mac << std::endl;

			macAddress = local_mac;
			// here just need the first card info
			break;
			// iterate next
			//pIpAdapterInfo = pIpAdapterInfo->Next;
		}
	}

	if (pIpAdapterInfo)
		delete pIpAdapterInfo;
}

LRESULT CefForm::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT CefForm::OnNcLButtonDbClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::SetFocus(GetHWND());
	return __super::OnNcLButtonDbClick(uMsg, wParam, lParam, bHandled);
}
void CefForm::OnLoadEnd(int httpStatusCode)
{
	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	if (httpStatusCode != 200)
	{
		log.W(filename(__FILE__), __LINE__, YLog::ERR, "URL", cef_control_->GetUTF8URL());
	}

	XMLConfigTool* tool = new XMLConfigTool();
	string tempUserName = tool->GetWebsocketUser();
	if (!LoginForm::user_name.empty())
	{
		tempUserName = LoginForm::user_name;
		std::string para = "localStorage.setItem(\"isLogin\", \"true\")";
		cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);

		para = "localStorage.setItem(\"token\",\"";
		para += LoginForm::user_token;
		para += "\")";
		cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);
	}

	if (m_bRegisterFlag)
	{
		std::string para = "localStorage.setItem(\"isLogin\", \"false\")";
		cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);
	}

	if (m_bIsFirst && m_bQaFlag)
	{
		if (!m_question.empty())
		{
			std::string para = "window.CDSS_FE.getRecordQuestion(\'" + m_question + "\')\0";

			log.W(__FILE__, __LINE__, YLog::DEBUG, shared::tools::UtfToString("执行命令参数"), para);

			cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);

		}
	}

	getNetworkInfo();

	if (!macAddress.empty())
	{
		tempUserName = macAddress;
	}

	if (!tempUserName.empty())
	{
		strUserName = tempUserName;
		std::string para = "localStorage.setItem(\"wsUser\",\"";// \"lance\")";
		para += strUserName;
		para += "\")";
		cef_control_->GetBrowserHandler()->GetBrowser()->GetMainFrame()->ExecuteJavaScript(nbase::UTF8ToUTF16(para), L"", 0);
	}

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

	cef_control_->RegisterCppFunc(L"SetUserName", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		//shared::Toast::ShowToast(nbase::UTF8ToUTF16(params), 3000, GetHWND());
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
			if (doc.HasMember("name"))
			{
				Value& m = doc["name"];
				if (!m.IsNull() && m.IsString())
				{
					string username = m.GetString();
					if (!username.empty())
					{
						CefForm::strUserName = username;
						log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("设置连接用户名"), CefForm::strUserName);
					}
					else
					{
						log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("警告"), shared::tools::UtfToString("用户名为空！"));
					}

				}
			}
		}
	}));

	cef_control_->RegisterCppFunc(L"ShowOfflineMsg", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
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
			if (doc.HasMember("type"))
			{
				Value& m = doc["type"];
				if (!m.IsNull() && m.IsString())
				{
					string type = m.GetString();
					if (type.empty())
					{
						return;
					}
					else
					{
						if (type.find("offline") != string::npos)
						{
							//shared::Toast::ShowToast(L"已离线！", 3000, GetHWND());
							if (m_hMainWnd > 0)
							{
								::SendMessage(m_hMainWnd, WM_USERONLINESTATE, 2, 0);
							}
						}
						else if (type.find("online") != string::npos)
						{
							if (m_hMainWnd > 0)
							{
								::SendMessage(m_hMainWnd, WM_USERONLINESTATE, 1, 0);
								//shared::Toast::ShowToast(L"上线成功！", 3000, GetHWND());
							}
						}
						else if (type.find("kickout") != string::npos)
						{
							if (!m_bRegisterFlag)
							{
								shared::Toast::ShowToast(L"已被踢下线！", 3000, GetHWND());
							}

							if (m_hMainWnd > 0)
							{
								::SendMessage(m_hMainWnd, WM_USERONLINESTATE, 2, 0);
							}

						}
					}
				}
			}
		}

	}));

	// 注册一个方法提供前端调用，展开窗口
	cef_control_->RegisterCppFunc(L"ModifyPassword", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

		::SendMessage(CefForm::g_main_hwnd, WM_MODIFYPASSWORD, 0, 0);

	}));

	cef_control_->RegisterCppFunc(L"ShowDiseaseInfo", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

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

					shared::tools::SafeOpenUrlEx(msg, SW_SHOWMAXIMIZED);
				}
			}
		}

	}));

	cef_control_->RegisterCppFunc(L"callCsPlugin", ToWeakCallback([this](const std::string& params, nim_cef::ReportResultFunction callback) {
		callback(false, R"({ "message": "Success." })");

		if (params.empty())
		{
			return;
		}

		::SendMessage(CefForm::g_main_hwnd, WM_OPENRWCLIENT, 0, 0);

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

								SendMessage(WM_SENDRWMESSAGE, (WPARAM)data, 0);

								delete data;
								data = NULL;
							}
						}

					}
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
