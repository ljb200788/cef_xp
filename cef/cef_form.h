#pragma once

// �����Ƿ�ʹ��������Ⱦ����Ϊ true  ʱ��ʹ��Ĭ�ϴ�����Ӱ������������Ⱦģʽ�´����� WS_EX_LAYERED ����
// ��Ϊ false ʱ��ʹ�����洰��ģʽ��֧�ִ��� WS_EX_LAYERED ���Դ��ڣ�����ʹ�����ô�����Ӱ��������Ҫ�� xml �н����� shadowattached ��������Ϊ false
const bool kEnableOffsetRenderCef = false;

class CefForm : public std::conditional<kEnableOffsetRenderCef, ui::WindowImplBase, ui::ShadowWndBase>::type
{
public:
	CefForm();
	~CefForm();

	void    SetNavigateUrl(std::string url);

	void	RefreshNavigateUrl();

	void    SetMaxFlag(bool  maxFlag);
	void    SetRegisterFlag(bool  registerFlag);
	void    SetHiddenFlag(bool  hiddenFlag);
	bool	GetHiddenFlag();

	void	SetMinBtnHidden(bool isVisible);

	void	SetMaxBtnHidden(bool isVisible);

	void    RefreshCefControl();

	void    SetMainWndHWND(HWND hwnd);

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

	/**
	 * �յ� WM_CREATE ��Ϣʱ�ú����ᱻ���ã�ͨ����һЩ�ؼ���ʼ���Ĳ���
	 */
	virtual void InitWindow() override;

	/**
	 * �յ� WM_CLOSE ��Ϣʱ�ú����ᱻ����
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT OnNcLButtonDbClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	static const std::wstring	kClassName;

	static	HWND g_main_hwnd;

	static	std::string strUserName;

	//���������ҳ�洰��
	static	CefForm* g_ptr_rw_cef;

private:
	bool OnClicked(ui::EventArgs* msg);
	bool OnNavigate(ui::EventArgs* msg);
	void OnLoadEnd(int httpStatusCode);
	void OnLoadError(int errorCode);

private:
	ui::CefControlBase* cef_control_;
	ui::CefControlBase* cef_control_dev_;
	ui::Button*			btn_dev_tool_;
	ui::Button*			btn_hidden_tool_;
	ui::Button*			btn_close_tool_;
	ui::RichEdit*		edit_url_;

	ui::Label*			label_Title_;

	std::string			m_strUrl;

	bool				m_bNeedMax;
	bool				m_bWndClosed;
	bool				m_bNeedHidden;

	HWND				m_hMainWnd;

	bool				m_bRegisterFlag;
	bool				m_bQaFlag;
	std::string			m_question;

	bool				m_bIsFirst;

};

