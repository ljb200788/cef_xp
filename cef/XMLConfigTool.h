#pragma once
class XMLConfigTool
{
public:
	XMLConfigTool();
	~XMLConfigTool();

	/*
		获取调试窗口配置
	*/
	bool   GetDevToolConfig();

	/*
		获取登录配置
	*/
	bool   GetNeedLoginConfig();

	/*
		获取自启动配置
	*/
	bool   GetAutoStartConfig();

	/*
		获取人卫客户端配置
	*/
	bool   GetRwStartConfig();

	/*
		获取日志级别配置
	*/
	bool   GetLogDebugConfig();

	/*
		获取互联网检测配置
	*/
	bool   GetNeedCheckInternetConfig();

	std::string   GetDevToolUrl();

	std::string   GetUploadUrl();

	std::string   GetSpeechRecognitionUrl();

	std::string   GetRwServerUrl();

	std::string   GetAliasServerUrl();
	
	std::string   GetKnowledgeLinkUrl();

	std::string   GetDiagnoseUrl();

	std::string   GetDiagnoseStdUrl();

	std::string   GetNavigateUrl();

	std::string   GetNavigateUrlEx(std::string para);

	std::string   GetWebsocketUser();

	std::string   GetUpdateUrl();

	std::string   GetCaptureCoordinate();

	std::string   GetToolConfigUrl();

	std::string   GetLoginConfigUrl();

	std::string   GetAssistantDiagnoseUrl();

	std::string   GetQaResultUrl();

	std::string   GetQaPageUrl();

	std::string   GetRegisterPageUrl();
	
	unsigned int  GetTemplateIdValue();

	unsigned int  GetCommonTemplateIdValue();

	unsigned int  GetScreenKeyValue();

	unsigned int  GetHotkeyCodeUser();

	unsigned int  GetHotkeyCodeStd();

	unsigned int  GetLocalServerPort();

};

