#pragma once

#include <windows.h>

namespace ui {

	int mainLoop(HINSTANCE hInstance, int nCmdShow);

	// request the status window to be redrawn
	void updateStatusWindow();
}