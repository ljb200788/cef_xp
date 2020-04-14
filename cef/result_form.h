#pragma once

const bool kEnableOffsetRenderVersion = false;

class ResultForm : public std::conditional<kEnableOffsetRenderVersion, ui::WindowImplBase, ui::ShadowWndBase>::type
{
public:
	ResultForm();
	~ResultForm();

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

	virtual LRESULT OnNcLButtonDbClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
	* �յ� WM_CREATE ��Ϣʱ�ú����ᱻ���ã�ͨ����һЩ�ؼ���ʼ���Ĳ���
	*/
	virtual void InitWindow() override;

	virtual LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
	* �յ� WM_CLOSE ��Ϣʱ�ú����ᱻ����
	*/
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void    SetNavigateUrl(std::string url);

	void	SetDebugWindowHWND(HWND wnd);

	void	ReloadNavigateUrl();

	static	HWND g_main_hwnd;

	static const std::wstring	kClassName;

	static string		user_name ;

private:
	bool OnClicked(ui::EventArgs* msg);
	void OnLoadEnd(int httpStatusCode);

	void OnUrlChange(const std::wstring& url);

	/**
	* �������пؼ���������Ϣ
	* @param[in] msg ��Ϣ�������Ϣ
	* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
	*/
	bool Notify(ui::EventArgs* msg);

	ui::CefControlBase* cef_control_;
	ui::CefControlBase* cef_control_dev_;

	std::string			m_strUrl;

	HWND				m_debug_wnd;
};