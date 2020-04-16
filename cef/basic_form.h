#pragma once

#include "cef_form.h"
#include <ShellAPI.h>


DWORD GetProcessIDByName(WCHAR* pName);

// 控制是否使用离屏渲染，当为 true  时将使用默认窗口阴影方案，离屏渲染模式下窗口有 WS_EX_LAYERED 属性
// 当为 false 时因使用了真窗口模式不支持带有 WS_EX_LAYERED 属性窗口，所以使用外置窗口阴影方案，需要在 xml 中将窗口 shadowattached 属性设置为 false
const bool kEnableOffsetRender = true;

class BasicForm : public std::conditional<kEnableOffsetRender, ui::WindowImplBase, ui::ShadowWndBase>::type
{
public:
	BasicForm();
	~BasicForm();


	void  SetToolBtnUrl();

	/*
	请求工具配置接口设置工具条按钮
	*/
	void  RequestToolConfigUrl();

	void AddTrayIcon();

	void CreateTrayMenu();

	void  ShowDevTool();

	void ExitApp();

	void UpdateHideStateNoticeMenu();

	void AutoHiddenWindow(int iNum);

	void AutoShowWindow();

	bool JudgeCursorOut();

	void  GetBmhipInfo();

	/**
	 * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual ui::Control* CreateControl(const std::wstring& pstrClass) override;

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	/**
	 * 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	 */
	virtual void InitWindow() override;

	/**
	 * 收到 WM_CLOSE 消息时该函数会被调用
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	static const std::wstring	kClassName;

	BOOL ShowBalloonTip(std::wstring szMsg, std::wstring szTitle, DWORD dwInfoFlags = NIIF_INFO, UINT uTimeout = 3000);


private:
	bool OnClicked(ui::EventArgs* msg);

private:

	RECT		m_rtWindow;
	int			m_iWindowWidth;
	int			m_iWindowHeight;

	ui::Control*			btn_doctor;
	ui::Control*			btn_doctor1;

	ui::Box*				box_tool;

	ui::Button* tool_btn1;
	ui::Button* tool_btn2;
	ui::Button* tool_btn3;
	ui::Button* tool_btn4;
	ui::Button* tool_btn5;
	ui::Button* tool_btn6;

	std::string  m_navUrl1;
	std::string  m_navUrl2;
	std::string  m_navUrl3;
	std::string  m_navUrl4;
	std::string  m_navUrl5;
	std::string  m_navUrl6;

	CefForm* m_hiddenWindow;
};

