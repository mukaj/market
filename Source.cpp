#ifdef _WIN32
#include "GUI.hpp"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	gui::market_gui mg;
	mg.show();
	nana::exec();
	return 0;
}

#else
#include "GUI.hpp"

int main() {
	gui::market_gui mg;
	mg.show();
	nana::exec();
	return 0;
}

#endif