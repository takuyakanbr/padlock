#include "stdafx.h"

#include "state.hpp"
#include "ui.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
	PWSTR pCmdLine, int nCmdShow) {

	_D(AllocConsole(); freopen("CONOUT$", "w", stdout); freopen("CONOUT$", "w", stderr););

	state::setup();
	return ui::mainLoop(hInstance, nCmdShow);

}
