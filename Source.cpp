#include "GUI.h"
#include <string>
#include <vector>
#include "list.h"
#include <iostream>

int main() {
	gui::market_gui mg;
	mg.show();
	nana::exec();
	return 0;
}