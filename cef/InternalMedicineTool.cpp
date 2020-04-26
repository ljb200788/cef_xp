#include "stdafx.h"
#include "InternalMedicineTool.h"
#include "WininetHttp.h"
#include "ylog.h"
#include <json\value.h>
#include <json\reader.h>

InternalMedicineTool::InternalMedicineTool()
{
}


InternalMedicineTool::~InternalMedicineTool()
{
}

void InternalMedicineTool::GetAllDisease()
{
	string urlString = "http://172.16.13.2:8086/ylkg/v1/knowledges/concept/DISEASE";

	CWininetHttp netHttp;
	std::string ret = netHttp.RequestJsonInfo(urlString, Hr_Get,
		"Content-Type:application/x-www-form-urlencoded;charset=utf-8", "");

	YLog log(YLog::INFO, "log_disease.txt", YLog::ADD);
	log.W(__FILE__, __LINE__, YLog::INFO, "ret", ret);


	if (!ret.empty())
	{
		Json::Reader reader;
		Json::Value value;

		if (reader.parse(ret, value))
		{
			if (value.isArray())
			{
				for (int i = 0; i < value.size(); i++)
				{
					if (value[i].isObject())
					{
						if (value[i].isMember("label"))
						{
							string label = value[i]["label"].asString();
							log.W(__FILE__, __LINE__, YLog::INFO, "label", label);
						}

						if (value[i].isMember("about"))
						{
							string about = value[i]["about"].asString();
							log.W(__FILE__, __LINE__, YLog::INFO, "about", about);
						}
					}
				}
			}
		}
	}


}

