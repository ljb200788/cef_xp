#pragma once
class XMLConfigTool
{
public:
	XMLConfigTool();
	~XMLConfigTool();

	/*
		��ȡ���Դ�������
	*/
	bool   GetDevToolConfig();

	/*
		��ȡ��¼����
	*/
	bool   GetNeedLoginConfig();

	/*
		��ȡ����������
	*/
	bool   GetAutoStartConfig();

	/*
		��ȡ�����ͻ�������
	*/
	bool   GetRwStartConfig();

	/*
		��ȡ��־��������
	*/
	bool   GetLogDebugConfig();

	/*
		��ȡ�������������
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

