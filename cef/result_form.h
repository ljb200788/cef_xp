#pragma once

const bool kEnableOffsetRenderVersion = false;

class ResultForm : public std::conditional<kEnableOffsetRenderVersion, ui::WindowImplBase, ui::ShadowWndBase>::type
{
public:
	ResultForm();
	~ResultForm();

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

	virtual LRESULT OnNcLButtonDbClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
	* 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	*/
	virtual void InitWindow() override;

	virtual LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
	* 收到 WM_CLOSE 消息时该函数会被调用
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
	* 处理所有控件的所有消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool Notify(ui::EventArgs* msg);

	ui::CefControlBase* cef_control_;
	ui::CefControlBase* cef_control_dev_;

	std::string			m_strUrl;

	HWND				m_debug_wnd;
};