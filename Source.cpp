#include <iostream>
#include <vector>
#include <string>
#include "list.h"
#include "commands.h"

int main() {
	std::ios::sync_with_stdio(false);
	while(commands::function_selector());
	return 0;
}