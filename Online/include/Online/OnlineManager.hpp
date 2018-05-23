#pragma once
#include <curl/curl.h>

class OnlineManager : Unique
{
public:
	OnlineManager();
	~OnlineManager();

	bool login(const string& username, const string& password);
	void logout();

	bool is_logged_in() const;
	void submit_score(int score, int crit, int almost, int miss, float gauge, const int32 id);

private:
	CURL* curl;
	bool logged_in = false;
	string token;
};
