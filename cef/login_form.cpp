#include "stdafx.h"
#include "login_form.h"
#include "cef_form.h"
#include <fstream>
#include <boost\thread\thread.hpp>
#include "XMLConfigTool.h"
#include "basic_form.h"
#include "WininetHttp.h"
#include "json\value.h"
#include "json\reader.h"
#include "ylog.h"
#include "INI.h"
#include "md5.h"
#include "iphlpapi.h"

#pragma comment(lib, "iphlpapi.lib") 

using namespace ui;
using namespace boost;

string LoginForm::user_name = "";
string LoginForm::user_token = "";
string LoginForm::user_id = "";


string getMacInfo()
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

			string macAddress = local_mac;
			return macAddress;
		}
	}

	if (pIpAdapterInfo)
		delete pIpAdapterInfo;

	return "";
}
int GetLoginInfo()
{
	INI::PARSE_FLAGS = INI::PARSE_COMMENTS_SLASH | INI::PARSE_COMMENTS_HASH;

	INI ini("login.ini", true);

	ini.select("UserInfo");
	string token = ini.get("UserInfo", "token", "");
	LoginForm::user_token = token;
	LoginForm::user_name = ini.get("UserInfo", "userName", "");
	LoginForm::user_id = ini.get("UserInfo", "userId", "");

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, "token", token);

	if (!token.empty())
	{
		XMLConfigTool* tool = new XMLConfigTool();
		std::string loginConfigUrl = tool->GetLoginConfigUrl();
		delete tool;

		if (loginConfigUrl.empty())
		{
			shared::Toast::ShowToast(L"登录接口地址未配置！", 3000, NULL);
		}
		else
		{

			CWininetHttp netHttp;
			std::string ret = netHttp.RequestJsonInfo(loginConfigUrl + "token/validation", Hr_Post, "Content-Type:text/plain;charset=utf-8", token);

			log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("token有效验证"), ret);

			Json::Reader reader_validate;
			Json::Value value_validate;
			if (reader_validate.parse(ret, value_validate))
			{
				if (value_validate.isMember("code"))
				{
					if (value_validate["code"].isInt())
					{
						int code = value_validate["code"].asInt();
						if (code == 10001)//更新token
						{
							ret = netHttp.RequestJsonInfo(loginConfigUrl + "token/refresh", Hr_Post, "Content-Type:text/plain;charset=utf-8", token);
							log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("token更新"), ret);

							Json::Reader reader;
							Json::Value value;

							if (reader.parse(ret, value))
							{
								if (value.isMember("code"))
								{
									if (value["code"].isInt())
									{
										int code = value["code"].asInt();
										if (code == 10001)//更新token
										{

											//更新配置中的token
											if (value.isMember("newToken"))
											{
												if (value["newToken"].isString())
												{
													string token = value["newToken"].asString();

													LoginForm::user_token = token;

													ini.set("token", token);
													ini.save("login.ini");
												}
											}
											return 1;
										}
										else if (code == 10002)//token不存在
										{

										}
									}
								}
							}
							if (value_validate.isMember("isInvalid"))
							{
								if (value_validate["isInvalid"].isBool())
								{
									if (value_validate["isInvalid"].asBool())
									{
										
									}
									else
									{
										return 1;
									}
								}
							}
						}

					}
				}
			}
		}
	}

	return 0;
}

int AutoRegisterInfo()
{
	INI::PARSE_FLAGS = INI::PARSE_COMMENTS_SLASH | INI::PARSE_COMMENTS_HASH;

	INI ini_read("login.ini", true);

	ini_read.select("UserInfo");

	string token = ini_read.get("UserInfo", "token", "");
	LoginForm::user_token = token;
	LoginForm::user_name = ini_read.get("UserInfo", "userName", "");
	LoginForm::user_id = ini_read.get("UserInfo", "userId", "");

	YLog log(YLog::INFO, "log.txt", YLog::ADD);
	log.W(filename(__FILE__), __LINE__, YLog::DEBUG, "token", token);

	if (token.empty())
	{
		XMLConfigTool* tool = new XMLConfigTool();
		std::string registerConfigUrl = tool->GetRegisterConfigUrl();
		delete tool;

		if (!registerConfigUrl.empty())
		{
			Json::Value resultPara;
			resultPara["userName"] = getMacInfo();
			resultPara["userRealname"] = getMacInfo();

			std::string pwd_md5 = md5("creator@123");
			resultPara["userPassword"] = pwd_md5;

			string data = getMacInfo() + "creator";
			string registerMark  = "registerMark:" + md5(data);

			CWininetHttp netHttp;
			std::string ret = netHttp.RequestJsonInfo(registerConfigUrl, Hr_Post, registerMark + "\r\nContent-Type:application/json;charset=utf-8", resultPara.toStyledString());

			log.W(filename(__FILE__), __LINE__, YLog::DEBUG, "registerConfigUrl", ret);


			string userId = "";
			string user_name = "";

			Json::Reader reader;
			Json::Value value;
			if (reader.parse(ret, value))
			{
				if (value.isMember("token"))
				{
					if (value["token"].isString())
					{
						token = value["token"].asString();
					}

					if (value.isMember("userName"))
					{
						if (value["userName"].isString())
						{
							user_name = value["userName"].asString();
						}

					}

					if (value.isMember("userId"))
					{
						if (value["userId"].isString())
						{
							userId = value["userId"].asString();
						}
					}

					INI::SAVE_FLAGS = INI::SAVE_PRUNE | INI::SAVE_PADDING_SECTIONS | INI::SAVE_SPACE_SECTIONS | INI::SAVE_SPACE_KEYS | INI::SAVE_TAB_KEYS | INI::SAVE_SEMICOLON_KEYS;

					LoginForm::user_token = token;
					LoginForm::user_name = user_name;
					LoginForm::user_id = userId;

					INI ini_write("login.ini", false);

					ini_write.create("UserInfo");
					ini_write.set("token", token);
					ini_write.set("userName", user_name);
					ini_write.set("userId", userId);

					ini_write.save("login.ini");

					return 1;
				}

			}
		}
	}
	else
	{
		return 1;
	}

	return 0;
}


LoginForm::LoginForm()
{
}

LoginForm::~LoginForm()
{
}

std::wstring LoginForm::GetSkinFolder()
{
	return L"login";
}

std::wstring LoginForm::GetSkinFile()
{
	return L"login.xml";
}

std::wstring LoginForm::GetWindowClassName() const
{
	return kClassName;
}

const std::wstring LoginForm::kClassName = L"Login";


bool checkString(const char * iName)
{
	bool result = false;
	while (*iName)
	{
		if ((*iName) & 0x80)
		{
			result = false;
			++iName;//汉字跳过一个字节检测
			break;
		}
		else if (((*iName) >= 'a' && (*iName) <= 'z' || (*iName) >= 'A' && (*iName) <= 'Z') || ((*iName) >= '0' && (*iName) <= '9'))
		{
			result = true;
		}
		else if ((*iName) == '_')
		{
			result = true;
		}
		else if ((*iName) == '.')
		{
			result = true;
		}
		else if ((*iName) == '@')
		{
			result = true;
		}
		else
		{
			result = false;
			break;
		}
		++iName;
	}
	return result;
}
void LoginForm::InitWindow()
{
	// 监听鼠标单击事件
	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&LoginForm::OnClicked, this, std::placeholders::_1));

	user_name_edit_ = (RichEdit*)FindControl(L"username");
	password_edit_  = (RichEdit*)FindControl(L"password");

	user_name_edit_->SetSelAllOnFocus(true);
	password_edit_->SetSelAllOnFocus(true);

	//user_name_edit_->SetText(L"wind");
	//password_edit_->SetText(L"wind");

	user_name_edit_->SetLimitText(32);
	password_edit_->SetLimitText(32);

	//SetWindowLong(GetHWND(), GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	CenterWindow();
	ShowWindow(true);
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	SetForegroundWindow(m_hWnd);

	SetIcon(107);
	SetTaskbarTitle(L"登录");

	INI ini("login.ini", true);

	ini.select("UserInfo");
	string username = ini.get("UserInfo", "userName", "");
	if (!username.empty())
	{
		user_name_edit_->SetText(nbase::UTF8ToUTF16(username));
	}

}

bool LoginForm::Notify(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();

	return true;
}

LRESULT LoginForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	nim_cef::CefManager::GetInstance()->PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

bool   LoginForm::UserLogin()
{
	std::string username = user_name_edit_->GetUTF8Text();
	StringHelper::Trim(username);

	std::string password = password_edit_->GetUTF8Text();
	StringHelper::Trim(password);

	if (username.length() <= 0)
	{
		shared::Toast::ShowToast(L"请输入账号！", 3000, GetHWND());
	}
	//else if (username.length() < 6)
	//{
	//	shared::Toast::ShowToast(L"账号长度不能少于6位！", 3000, GetHWND());
	//}
	else if (checkString(username.c_str()) == false)
	{
		shared::Toast::ShowToast(L"账号只能包含字母,下划线和数字！", 3000, GetHWND());
	}
	else if (password.length() <= 0)
	{
		shared::Toast::ShowToast(L"请输入密码！", 3000, GetHWND());
	}
	else
	{

		user_name = user_name_edit_->GetUTF8Text();

		XMLConfigTool* tool = new XMLConfigTool();
		std::string loginConfigUrl = tool->GetLoginConfigUrl();
		delete tool;

		if (loginConfigUrl.empty())
		{
			shared::Toast::ShowToast(L"登录接口地址未配置！", 3000, GetHWND());
		}
		else
		{
			Json::Value resultPara;
			resultPara["userName"] = user_name;

			std::string pwd_md5 = md5(password);
			resultPara["userPassword"] = pwd_md5;// password;

			CWininetHttp netHttp;
			std::string ret = netHttp.RequestJsonInfo(loginConfigUrl + "login", Hr_Post, "Content-Type:application/json;charset=utf-8", resultPara.toStyledString());

			YLog log(YLog::INFO, "log.txt", YLog::ADD);
			log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("MD5密码"), pwd_md5);
			log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("登录结果"), ret);


			if (ret.empty())
			{
				shared::Toast::ShowToast(L"登录失败！", 3000, GetHWND());
				return false;
			}

			Json::Reader reader;
			Json::Value value;

			if (reader.parse(ret, value))
			{
				if (value.isObject())
				{
					if (value.isMember("code"))
					{
						if (value["code"].isInt())
						{
							int code = value["code"].asInt();
							if (code != 10001)
							{
								if (value.isMember("message"))
								{
									if (value["message"].isString())
									{
										string message = value["message"].asString();

										shared::Toast::ShowToast(nbase::UTF8ToUTF16(message).c_str(), 3000, GetHWND());
										return false;
									}
								}
							}
							else
							{

								string token = "";
								if (value.isMember("token"))
								{
									if (value["token"].isString())
									{
										token = value["token"].asString();
										LoginForm::user_token = token;
									}
								}

								string userId = "";
								if (value.isMember("userId"))
								{
									if (value["userId"].isString())
									{
										userId = value["userId"].asString();
										LoginForm::user_id = userId;
									}
								}
								
								INI::SAVE_FLAGS = INI::SAVE_PRUNE | INI::SAVE_PADDING_SECTIONS | INI::SAVE_SPACE_SECTIONS | INI::SAVE_SPACE_KEYS | INI::SAVE_TAB_KEYS | INI::SAVE_SEMICOLON_KEYS;

								INI ini("login.ini", false);

								ini.create("UserInfo");
								ini.set("token", token);
								ini.set("userName", user_name);
								ini.set("userId", userId);

								ini.save("login.ini");
							}
						}
					}
				}
			}
			else
			{
				shared::Toast::ShowToast(L"登录失败！", 3000, GetHWND());
				return false;
			}

			ShowWindow(false);

			BasicForm* window = new BasicForm();
			window->Create(NULL, BasicForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
			window->ShowWindow();
		}
		
	}

	return  true;
}
LRESULT LoginForm::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	bHandled = true;

	if (wParam == VK_RETURN)
	{
		UserLogin();
	}
	else if (wParam == VK_TAB)
	{
		if (user_name_edit_->IsFocused())
		{
			password_edit_->SetFocus();
		}
		else
		{
			user_name_edit_->SetFocus();
		}
	}
	return __super::OnKeyDown(uMsg, wParam, lParam, bHandled);
}
bool LoginForm::OnClicked(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();

	if (name == L"btn_login")
	{
		UserLogin();
	}
	else if (name == L"btn_register")
	{

		XMLConfigTool* tool = new XMLConfigTool();
		std::string registerPageUrl = tool->GetRegisterPageUrl();
		delete tool;

		if (registerPageUrl.empty())
		{
			registerPageUrl = "http://medical.c2cloud.cn/#/user/register";
		}

		CefForm* registerForm = new CefForm();
		registerForm->SetMaxFlag(false);
		registerForm->SetRegisterFlag(true);
		registerForm->SetNavigateUrl(registerPageUrl);
		registerForm->Create(m_hWnd, CefForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0, nim_cef::CefManager::GetInstance()->IsEnableOffsetRender());
		ui::UiRect rect = registerForm->GetPos();
		MoveWindow(registerForm->GetHWND(), rect.left, rect.top, 900, 650, true);
		registerForm->CenterWindow();
		registerForm->ShowWindow();
	}
	else if(name == L"minbtn")
	{
		ShowWindow(SW_HIDE);
	}
	else if (name == L"closebtn")
	{
		PostQuitMessage(0L);
	}
	return true;
}