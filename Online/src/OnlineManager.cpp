#include "stdafx.h"
#include "Online/OnlineManager.hpp"
#include "rapidjson/document.h"

using namespace rapidjson;

const string BASE_URL = "http://ongaku.local:5000/api/v0";

OnlineManager::OnlineManager()
{
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
}

OnlineManager::~OnlineManager()
{
	curl_global_cleanup();
}

size_t writeFunction(void *ptr, size_t size, size_t nmemb, string* data)
{
    data->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

bool OnlineManager::login(const string& username, const string& password)
{
	const string form_data = "username=" + username + "&password=" + password;
	const string endpoint_url = BASE_URL + "/users/login";

	string response;
	string headers;

	curl_easy_setopt(curl, CURLOPT_URL, endpoint_url.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, form_data.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);

	const CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
		Logf("login failed: %s\n", Logger::Warning, curl_easy_strerror(res));
	else
	{
		Logf("login success!\n", Logger::Info);
		Document document;
		document.Parse(response.c_str());

		assert(document.IsObject() && !document.HasParseError());
		token = document["payload"]["token"].GetString();
		logged_in = true;
	}

	curl_easy_cleanup(curl);
	return res == CURLE_OK;
}

void OnlineManager::logout()
{
	token = nullptr;
	logged_in = false;
}

bool OnlineManager::is_logged_in() const
{
	return logged_in;
}

void OnlineManager::submit_score(int score, int crit, int almost, int miss, float gauge, const int32 id)
{
	
}
