#pragma once

// 控制是否使用离屏渲染，当为 true  时将使用默认窗口阴影方案，离屏渲染模式下窗口有 WS_EX_LAYERED 属性
// 当为 false 时因使用了真窗口模式不支持带有 WS_EX_LAYERED 属性窗口，所以使用外置窗口阴影方案，需要在 xml 中将窗口 shadowattached 属性设置为 false
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
	 * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual ui::Control* CreateControl(const std::wstring& pstrClass) override;

	/**
	 * 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	 */
	virtual void InitWindow() override;

	/**
	 * 收到 WM_CLOSE 消息时该函数会被调用
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT OnNcLButtonDbClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	static const std::wstring	kClassName;

	static	HWND g_main_hwnd;

	static	std::string strUserName;

	//人卫浏览器页面窗口
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

