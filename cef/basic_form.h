#pragma once

#include "cef_form.h"
#include <ShellAPI.h>


DWORD GetProcessIDByName(WCHAR* pName);

// �����Ƿ�ʹ��������Ⱦ����Ϊ true  ʱ��ʹ��Ĭ�ϴ�����Ӱ������������Ⱦģʽ�´����� WS_EX_LAYERED ����
// ��Ϊ false ʱ��ʹ�����洰��ģʽ��֧�ִ��� WS_EX_LAYERED ���Դ��ڣ�����ʹ�����ô�����Ӱ��������Ҫ�� xml �н����� shadowattached ��������Ϊ false
const bool kEnableOffsetRender = true;

class BasicForm : public std::conditional<kEnableOffsetRender, ui::WindowImplBase, ui::ShadowWndBase>::type
{
public:
	BasicForm();
	~BasicForm();


	void  SetToolBtnUrl();

	/*
	���󹤾����ýӿ����ù�������ť
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
	 * һ�������ӿ��Ǳ���Ҫ��д�Ľӿڣ����������������ӿ�����������
	 * GetSkinFolder		�ӿ�������Ҫ���ƵĴ���Ƥ����Դ·��
	 * GetSkinFile			�ӿ�������Ҫ���ƵĴ��ڵ� xml �����ļ�
	 * GetWindowClassName	�ӿ����ô���Ψһ��������
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual ui::Control* CreateControl(const std::wstring& pstrClass) override;

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	/**
	 * �յ� WM_CREATE ��Ϣʱ�ú����ᱻ���ã�ͨ����һЩ�ؼ���ʼ���Ĳ���
	 */
	virtual void InitWindow() override;

	/**
	 * �յ� WM_CLOSE ��Ϣʱ�ú����ᱻ����
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

