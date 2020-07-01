#include "stdafx.h"
#include "XMLConfigTool.h"
#include "tinyxml2.h"

using namespace tinyxml2;

XMLConfigTool::XMLConfigTool()
{

}


XMLConfigTool::~XMLConfigTool()
{
}

std::string  XMLConfigTool::GetDiagnoseUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "diagnoseUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string  XMLConfigTool::GetDiagnoseStdUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "diagnoseStdUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string  XMLConfigTool::GetKnowledgeLinkUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "knowledgeLinkUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string  XMLConfigTool::GetAliasServerUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "aliasUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string  XMLConfigTool::GetRwServerUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "rwServerUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string  XMLConfigTool::GetSpeechRecognitionUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "speechRecognitionUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string  XMLConfigTool::GetUploadUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "uploadUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

bool XMLConfigTool::GetNeedCheckInternetConfig()
{
	bool needCheckInternet = false;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		return needCheckInternet;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		return needCheckInternet;
	}
	XMLElement* elementNext = middleAfterInsertion->FirstChildElement();

	while (elementNext)
	{
		std::string eleName = elementNext->Name();
		if (strcmp(eleName.c_str(), "needCheckInternet") == 0)
		{
			std::string eleText = elementNext->GetText();
			if (strcmp(eleText.c_str(), "true") == 0)
			{
				needCheckInternet = true;
			}
		}
		elementNext = elementNext->NextSiblingElement();
	}
	return needCheckInternet;
}

std::string  XMLConfigTool::GetDevToolUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "noticeUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

unsigned int  XMLConfigTool::GetCommonTemplateIdValue()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "commonTemplateId") == 0)
		{
			std::string eleText = element11->GetText();
			return atoi(eleText.c_str());
		}
		element11 = element11->NextSiblingElement();
	}
	return 0;
}

unsigned int  XMLConfigTool::GetTemplateIdValue()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "templateId") == 0)
		{
			std::string eleText = element11->GetText();
			return atoi(eleText.c_str());
		}
		element11 = element11->NextSiblingElement();
	}
	return 1;
}

unsigned int  XMLConfigTool::GetLocalServerPort()
{
	unsigned int code = 8080;
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "localServerPort") == 0)
		{
			std::string eleText = element11->GetText();
			if (!eleText.empty())
			{
				int value = atoi(eleText.c_str());
				if (value >= 80)
				{
					code = value;
				}
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return code;
}

unsigned int  XMLConfigTool::GetHotkeyCodeStd()
{
	unsigned int code = (unsigned int)'B';
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "hotkeyCodeStd") == 0)
		{
			std::string eleText = element11->GetText();
			if (!eleText.empty())
			{
				int value = (unsigned int)(eleText.substr(0, 1).c_str()[0]);
				if (value >= 66 && value <= 90)//Ö»ÄÜBºÍZÖ®¼ä
				{
					code = value;
				}
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return code;
}
unsigned int  XMLConfigTool::GetHotkeyCodeUser()
{
	unsigned int code = (unsigned int)'C';
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "hotkeyCodeUser") == 0)
		{
			std::string eleText = element11->GetText();
			if (!eleText.empty())
			{
				int value = (unsigned int)(eleText.substr(0, 1).c_str()[0]);
				if (value >= 66 && value <= 90)//Ö»ÄÜBºÍZÖ®¼ä
				{
					code = value;
				}
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return code;
}

unsigned int  XMLConfigTool::GetScreenKeyValue()
{
	unsigned int code = 119;
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return code;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "screenKeyValue") == 0)
		{
			std::string eleText = element11->GetText();
			if (!eleText.empty())
			{
				return atoi(eleText.c_str());
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return code;
}

std::string   XMLConfigTool::GetNavigateUrlEx(std::string para)
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), para.c_str()) == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}
std::string   XMLConfigTool::GetNavigateUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "navigateUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string   XMLConfigTool::GetAssistantDiagnoseUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "assistantDiagnoseUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string XMLConfigTool::GetKnowledgeQueryUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "knowledgeQueryUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string XMLConfigTool::GetQaResultUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* elementNext = middleAfterInsertion->FirstChildElement();

	while (elementNext)
	{
		std::string eleName = elementNext->Name();
		if (strcmp(eleName.c_str(), "qaResultUrl") == 0)
		{
			std::string eleText = elementNext->GetText();
			return eleText;
		}
		elementNext = elementNext->NextSiblingElement();
	}
	return "";
}

std::string XMLConfigTool::GetQaPageUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* elementNext = middleAfterInsertion->FirstChildElement();

	while (elementNext)
	{
		std::string eleName = elementNext->Name();
		if (strcmp(eleName.c_str(), "qaPageUrl") == 0)
		{
			std::string eleText = elementNext->GetText();
			return eleText;
		}
		elementNext = elementNext->NextSiblingElement();
	}
	return "";
}

std::string XMLConfigTool::GetRegisterPageUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* elementNext = middleAfterInsertion->FirstChildElement();

	while (elementNext)
	{
		std::string eleName = elementNext->Name();
		if (strcmp(eleName.c_str(), "registerPageUrl") == 0)
		{
			std::string eleText = elementNext->GetText();
			return eleText;
		}
		elementNext = elementNext->NextSiblingElement();
	}
	return "";
}

std::string XMLConfigTool::GetHospitalName()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "hospitalName") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string    XMLConfigTool::GetLoginConfigUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "loginConfigUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string    XMLConfigTool::GetToolConfigUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "toolConfigUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}


std::string    XMLConfigTool::GetUpdateUrl()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "updateUrl") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

std::string   XMLConfigTool::GetWebsocketUser()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "websocketUser") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}

bool  XMLConfigTool::GetLogDebugConfig()
{
	bool logDebugState = false;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		return logDebugState;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		return logDebugState;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "logDebugState") == 0)
		{
			std::string eleText = element11->GetText();
			if (strcmp(eleText.c_str(), "true") == 0)
			{
				logDebugState = true;
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return logDebugState;
}

bool  XMLConfigTool::GetRwStartConfig()
{
	bool rwStartState = true;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		return rwStartState;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		return rwStartState;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "rwStartState") == 0)
		{
			std::string eleText = element11->GetText();
			if (strcmp(eleText.c_str(), "false") == 0)
			{
				rwStartState = false;
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return rwStartState;
}

bool  XMLConfigTool::GetAutoStartConfig()
{
	bool autoStartState = true;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		return autoStartState;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		return autoStartState;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{

		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "autoStartState") == 0)
		{
			std::string eleText = element11->GetText();
			if (strcmp(eleText.c_str(), "false") == 0)
			{
				autoStartState = false;
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return autoStartState;
}

bool  XMLConfigTool::GetNeedLoginConfig()
{
	bool m_bNeedLogin = false;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		//MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_OK);
		return m_bNeedLogin;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		//MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_OK);
		return m_bNeedLogin;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{

		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "needLogin") == 0)
		{
			std::string eleText = element11->GetText();
			if (strcmp(eleText.c_str(), "true") == 0)
			{
				m_bNeedLogin = true;
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return m_bNeedLogin;
}
bool XMLConfigTool::GetAutoLoginConfig()
{
	bool m_bAutoLogin = true;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		//MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_OK);
		return m_bAutoLogin;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		//MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_OK);
		return m_bAutoLogin;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{

		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "autoLogin") == 0)
		{
			std::string eleText = element11->GetText();
			if (strcmp(eleText.c_str(), "false") == 0)
			{
				m_bAutoLogin = false;
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return m_bAutoLogin;
}
bool  XMLConfigTool::GetDevToolConfig()
{
	bool m_bNeedDevTool = false;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		//MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_OK);
		return m_bNeedDevTool;
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		//MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_OK);
		return m_bNeedDevTool;
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{

		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "devTool") == 0)
		{
			std::string eleText = element11->GetText();
			if (strcmp(eleText.c_str(), "true") == 0)
			{
				m_bNeedDevTool = true;
			}
		}
		element11 = element11->NextSiblingElement();
	}
	return m_bNeedDevTool;
}

std::string  XMLConfigTool::GetCaptureCoordinate()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile("config.xml") != XML_SUCCESS)
	{
		MessageBox(NULL, _T("¶ÁÈ¡ÅäÖÃÎÄ¼þconfig.xmlÊ§°Ü£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	XMLNode* middleAfterInsertion = doc.FirstChildElement("configPara");
	if (middleAfterInsertion == NULL)
	{
		MessageBox(NULL, _T("ÅäÖÃÎÄ¼þ¸ñÊ½´íÎó£¡"), _T("¸¨ÖúÕï¶ÏÖúÊÖ"), MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
		return "";
	}
	XMLElement* element11 = middleAfterInsertion->FirstChildElement();

	while (element11)
	{
		std::string eleName = element11->Name();
		if (strcmp(eleName.c_str(), "captureCoordinate") == 0)
		{
			std::string eleText = element11->GetText();
			return eleText;
		}
		element11 = element11->NextSiblingElement();
	}
	return "";
}