#include "GUI.h"
#include <string>
#include <vector>
#include "list.h"
#include <iostream>
#include "commands.h"

int main() {
	gui::market_gui mg;
	while(commands::backend_function_selector());
	mg.show();
	nana::exec();
	return 0;
}