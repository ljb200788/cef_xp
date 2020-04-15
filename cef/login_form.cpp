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


using namespace ui;
using namespace boost;

string LoginForm::user_name = "";
string LoginForm::user_token = "";

int GetLoginInfo()
{
	INI::PARSE_FLAGS = INI::PARSE_COMMENTS_SLASH | INI::PARSE_COMMENTS_HASH;

	INI ini("login.ini", true);

	ini.select("UserInfo");
	string token = ini.get("UserInfo", "token", "");
	LoginForm::user_name = ini.get("UserInfo", "username", "");

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
			std::string ret = netHttp.RequestJsonInfo(loginConfigUrl + "token/validation/"+token, Hr_Post, "Content-Type:application/json;charset=utf-8", token);
			log.W(filename(__FILE__), __LINE__, YLog::DEBUG, shared::tools::UtfToString("验证结果"), ret);

			Json::Reader reader;
			Json::Value value;

			if (reader.parse(ret, value))
			{
				if (value.isMember("code"))
				{
					if (value["code"].isInt())
					{
						int code = value["code"].asInt();
						if (code == 10043)//token有效
						{
							LoginForm::user_token = token;
							return 1;
						}
						else if (code == 10042)//token过期更新token
						{
							//更新配置中的token
							if (value.isMember("token"))
							{
								if (value["token"].isString())
								{
									string token = value["token"].asString();

									LoginForm::user_token = token;

									ini.set("token", token);
									ini.save("login.ini");
								}
							}
							return 1;
						}
						else if (code == 10041)//token不存在
						{

						}
					}
				}
			}
		}
	}

	// Loop through sections, keys and values
	/*for (auto i : ini.sections) {
	cout << "[" << i.first << "]" << endl;

	//for(auto j = i.second->begin(); j != i.second->end(); j++)
	for (auto j : *i.second) {
	cout << "  " << j.first << "=" << j.second << endl;
	}
	}*/


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

	SetIcon(132);
	SetTaskbarTitle(L"登录");

	INI ini("login.ini", true);

	ini.select("UserInfo");
	string username = ini.get("UserInfo", "username", "");
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
	else if (username.length() < 6)
	{
		shared::Toast::ShowToast(L"账号长度不能少于6位！", 3000, GetHWND());
	}
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
			resultPara["userPassword"] = password;

			CWininetHttp netHttp;
			std::string ret = netHttp.RequestJsonInfo(loginConfigUrl + "login", Hr_Post, "Content-Type:application/json;charset=utf-8", resultPara.toStyledString());

			YLog log(YLog::INFO, "log.txt", YLog::ADD);
			//log.W(filename(__FILE__), __LINE__, YLog::INFO, shared::tools::UtfToString("登录参数"), resultPara.toStyledString());
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
								INI::SAVE_FLAGS = INI::SAVE_PRUNE | INI::SAVE_PADDING_SECTIONS | INI::SAVE_SPACE_SECTIONS | INI::SAVE_SPACE_KEYS | INI::SAVE_TAB_KEYS | INI::SAVE_SEMICOLON_KEYS;

								INI ini("login.ini", false);

								ini.create("UserInfo");
								ini.set("token", token);
								ini.set("username", user_name);

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