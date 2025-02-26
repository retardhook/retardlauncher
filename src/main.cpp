#include <auth/offline.h>
#include <jar/launcher.h>
#include <iostream>

int main() {
	mojang::auth::Offline auth_system;
	mojang::game::Download download_system;
	mojang::game::Launcher launcher_system;

	mojang::auth::UserInfo user_info = auth_system.Login("nyri");
	mojang::game::JarInfo download_info = download_system.get("1.8", true);

	std::cout << "user: " << user_info.username << std::endl;
	std::cout << "uuid: " << user_info.uuid << std::endl;
	std::cout << "access token: " << user_info.access_token << std::endl;
	std::cout << "session token: " << user_info.session_token << std::endl;

	std::cout << "jar url: " << download_info.path << std::endl;
	std::cout << "jar version: " << download_info.version << std::endl;
	std::cout << "jar releaseTime: " << download_info.release_time << std::endl;
	std::cout << "jar type: " << download_info.type << std::endl;

	launcher_system.LaunchGame(user_info, download_info);

	return 0;
}
