#pragma once

#include <iostream>

inline void debugPrintln(std::string data) {
#ifdef _DEBUG
	std::cout << data << std::endl;
#endif // _DEBUG
}