#include "stdafx.h"
#include "rest_form.h"
#include <fstream>
#include <boost\thread\thread.hpp>
#include "WininetHttp.h"
#include "ylog.h"

using namespace ui;
using namespace boost;

RestForm::RestForm()
{
}

RestForm::~RestForm()
{
}

std::wstring RestForm::GetSkinFolder()
{
	return L"cef";
}

std::wstring RestForm::GetSkinFile()
{
	return L"test.xml";
}

std::wstring RestForm::GetWindowClassName() const
{
	return kClassName;
}

const std::wstring RestForm::kClassName = L"DiagnosticAssistantTest";


void RestForm::InitWindow()
{
	// 监听鼠标单击事件
	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&RestForm::OnClicked, this, std::placeholders::_1));

	user_name_edit_ = (RichEdit*)FindControl(L"url");
	password_edit_  = (RichEdit*)FindControl(L"para");

	user_name_edit_->SetSelAllOnFocus(true);
	password_edit_->SetSelAllOnFocus(true);

	user_name_edit_->SetText(L"http://localhost:8080/request");

	std::wstring para(L"{\"type\":\"ui\",\"data\":{\"item_id\":\"test123456\",\"item_type\":\"DISEASE\"},\"api\":\"knowledges\"}");
	password_edit_->SetText(para);

	SetIcon(132);

	SetForegroundWindow(GetHWND());

	SetWindowText(GetHWND(),L"接口测试中心");
}

bool RestForm::Notify(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();

	return true;
}

LRESULT RestForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostQuitMessage(0);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

bool RestForm::OnClicked(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();

	if (name == L"btn_login")
	{

		std::string username = user_name_edit_->GetUTF8Text();
		StringHelper::Trim(username);

		std::string password = password_edit_->GetUTF8Text();
		StringHelper::Trim(password);

		if (username.empty() || password.empty())
		{
			MessageBox(NULL,_T("接口地址或者接口参数为空！"), _T("辅助诊断助手"), MB_OK);
			return true;
		}

		ui::RichEdit* result_edit_ = (RichEdit*)FindControl(L"result");
		result_edit_->SetText(L"");

		HttpRequest requestMethod = Hr_Get;

		ui::Option* method_option = (Option*)FindControl(L"method");
		if (method_option->IsSelected())
		{
			requestMethod = Hr_Post;
		}
		else
		{
			requestMethod = Hr_Get;
		}

		CWininetHttp netHttp;
		std::string ret = netHttp.RequestJsonInfo(username, requestMethod, "Content-Type:application/json;charset=utf-8", password.c_str());
		
		result_edit_->SetText(nbase::UTF8ToUTF16(ret));

		
		YLog log(YLog::INFO, "log.txt", YLog::ADD);
		log.W(__FILE__, __LINE__, YLog::INFO, "Result", ret);
	}
	else 
	{
		//shared::Toast::ShowToast(name, 3000, GetHWND());
	}
	

	return true;
}