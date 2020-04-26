#pragma once

using namespace std;
/*
	������֤�û�token
*/
int GetLoginInfo();

class LoginForm : public ui::WindowImplBase
{
public:
	LoginForm();
	~LoginForm();

	/**
	* һ�������ӿ��Ǳ���Ҫ��д�Ľӿڣ����������������ӿ�����������
	* GetSkinFolder		�ӿ�������Ҫ���ƵĴ���Ƥ����Դ·��
	* GetSkinFile			�ӿ�������Ҫ���ƵĴ��ڵ� xml �����ļ�
	* GetWindowClassName	�ӿ����ô���Ψһ��������
	*/
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

	/**
	* �յ� WM_CREATE ��Ϣʱ�ú����ᱻ���ã�ͨ����һЩ�ؼ���ʼ���Ĳ���
	*/
	virtual void InitWindow() override;

	/**
	* �յ� WM_CLOSE ��Ϣʱ�ú����ᱻ����
	*/
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
	* �����¼�¼�
	*/
	bool    UserLogin();


	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	static const std::wstring	kClassName;


	static string		user_name;
	static string		user_token;
	static string		user_id;

private:
	bool OnClicked(ui::EventArgs* msg);

	/**
	* �������пؼ���������Ϣ
	* @param[in] msg ��Ϣ�������Ϣ
	* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
	*/
	bool Notify(ui::EventArgs* msg);


	ui::RichEdit*	user_name_edit_;
	ui::RichEdit*	password_edit_;

};