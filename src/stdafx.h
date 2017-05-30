// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>


#include <iostream>

#define APP_NAME L"Padlock"
#define APP_VERSION L"1.11"

//#define _PADLOCK_DEBUG

#ifdef _PADLOCK_DEBUG
#define _D(x) x
#define _Dc(x) std::cout << x
#else
#define _D(x)
#define _Dc(x)
#endif
