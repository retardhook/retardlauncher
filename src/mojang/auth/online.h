#pragma once
#include <string>

// @todo: add auth using mojang / microsoft ( no clue if its even mojang anymore )
namespace mojang::auth
{
	struct UserInfo {
		std::string access_token;
		std::string client_token;
		std::string uuid;
		std::string username;
	};

	class Online {

		UserInfo Login( std::string& email, std::string& password )
		{
			// try to login into passed in account
			// const auto response =  cpr::Post( cpr::Url{ " "})
		}

	};
}