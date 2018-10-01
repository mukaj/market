#include "GUI.h"
#include <string>
#include <vector>
#include "list.h"

#ifdef _WIN32

#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	gui::market_gui mg;
	mg.show();
	nana::exec();
	return 0;
}

#else

int main{
	gui::market_gui mg;
	mg.show();
	nana::exec();
	return 0;
}

#endif