#pragma once

#include "httplib.h"

char* GetLocalPogramVersion();

class CHttpServerUtil
{
public:
	CHttpServerUtil();
	~CHttpServerUtil();

	bool  StartServer();

	HWND  m_MainHwnd;
};

