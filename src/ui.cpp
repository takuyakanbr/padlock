#include "stdafx.h"
#include "Resource.h"
#include <Commctrl.h>

#include "ui.hpp"
#include "state.hpp"

using state::InputState;

namespace {
	RECT workArea;
	RECT statusWndSize = { 0, 0, 76, 18 };
	RECT optionsWndSize = { 0, 0, 430, 175 };

	const WCHAR wndTitle[] = L"Padlock";
	const WCHAR statusWndClass[] = L"pl_status";
	const WCHAR optionsWndClass[] = L"pl_options";
	const WCHAR *statusTexts[] = { L"Standard", L"Restricted", L"Locked" };

	HINSTANCE hInst;
	HWND hStatusWnd;
	HWND hOptionsWnd;
	HFONT hFont;
	HFONT hStatusFont;

	inline void repaintStatusWnd(const HWND& hWnd) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		SetBkMode(hdc, TRANSPARENT);

		HFONT hOldFont = (HFONT)SelectObject(hdc, hStatusFont);

		DrawText(hdc, statusTexts[(int)state::getInputState()],
			-1, &statusWndSize, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

		SelectObject(hdc, hOldFont);
		EndPaint(hWnd, &ps);
	}

	inline void repaintOptionsWnd(const HWND& hWnd) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		SetBkMode(hdc, TRANSPARENT);

		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		TextOut(hdc, 22, 26, L"Unlock sequence:", 16);
		TextOut(hdc, 22, 59, L"Restrict sequence:", 18);
		TextOut(hdc, 22, 91, L"Lock sequence:", 14);

		SelectObject(hdc, hOldFont);
		EndPaint(hWnd, &ps);
	}


	LRESULT CALLBACK statusWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_PAINT:
			_Dc("uis: Repainting" << std::endl);
			repaintStatusWnd(hWnd);
			break;
		case WM_LBUTTONUP:
			_Dc("uis: Left button up" << std::endl);
			ShowWindow(hOptionsWnd, SW_SHOW);
			UpdateWindow(hOptionsWnd);
			break;
		case WM_RBUTTONUP:
			_Dc("uis: Right button up" << std::endl);
			if (state::isUnlocked())
				DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
			_Dc("uis: Quitting" << std::endl);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK tbUnlockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
		switch (message) {
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			std::string str = state::updateSequence(STATE_KEYSEQ_UNLOCKED, wParam);
			SetWindowTextA(hWnd, str.c_str());
			return 0;
		}
		case WM_CHAR:
		case WM_DEADCHAR:
			return 0;
		case WM_LBUTTONDOWN:
			state::notifyUpdate(STATE_KEYSEQ_UNLOCKED);
			break;
		}
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK tbLimitProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
		switch (message) {
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			std::string str = state::updateSequence(STATE_KEYSEQ_LIMITED, wParam);
			SetWindowTextA(hWnd, str.c_str());
			return 0;
		}
		case WM_CHAR:
		case WM_DEADCHAR:
			return 0;
		case WM_LBUTTONDOWN:
			state::notifyUpdate(STATE_KEYSEQ_LIMITED);
			break;
		}
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK tbLockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
		switch (message) {
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			std::string str = state::updateSequence(STATE_KEYSEQ_LOCKED, wParam);
			SetWindowTextA(hWnd, str.c_str());
			return 0;
		}
		case WM_CHAR:
		case WM_DEADCHAR:
			return 0;
		case WM_LBUTTONDOWN:
			state::notifyUpdate(STATE_KEYSEQ_LOCKED);
			break;
		}
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK optionsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_PAINT:
			repaintOptionsWnd(hWnd);
			break;
		case WM_CLOSE:
			state::notifyUpdate(STATE_KEYSEQ_NONE);
			ShowWindow(hWnd, SW_HIDE);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}


	// create the status window at the bottom right corner of the working area
	bool createStatusWindow(HINSTANCE hInstance) {
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = statusWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PADLOCK));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = CreateSolidBrush(RGB(210, 210, 210));
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = statusWndClass;
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		if (!RegisterClassExW(&wcex)) return false;

		hStatusWnd = CreateWindowEx(
			WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
			statusWndClass, wndTitle, WS_POPUP,
			workArea.right - statusWndSize.right, workArea.bottom - statusWndSize.bottom,
			statusWndSize.right, statusWndSize.bottom,
			nullptr, nullptr, hInstance, nullptr);

		if (!hStatusWnd) return false;

		ShowWindow(hStatusWnd, SW_SHOWNORMAL);
		UpdateWindow(hStatusWnd);

		return true;
	}

	// create the options window in the center of the working area
	bool createOptionsWindow(HINSTANCE hInstance) {
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = optionsWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PADLOCK));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = CreateSolidBrush(RGB(225, 225, 225));
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = optionsWndClass;
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		if (!RegisterClassExW(&wcex)) return false;

		LONG x = (workArea.right - workArea.left - optionsWndSize.right) / 2;
		LONG y = (workArea.bottom - workArea.top - optionsWndSize.bottom) / 2;
		hOptionsWnd = CreateWindowEx(
			WS_EX_TOPMOST, optionsWndClass, wndTitle, WS_CAPTION | WS_SYSMENU,
			x, y, optionsWndSize.right, optionsWndSize.bottom,
			nullptr, nullptr, hInstance, nullptr);

		// create textboxes
		HWND tbUnlock = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_UNLOCKED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 23, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		HWND tbRestrict = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_LIMITED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 56, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		HWND tbLock = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_LOCKED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 88, 270, 22, hOptionsWnd, NULL, NULL, NULL);

		SendMessage(tbUnlock, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(tbRestrict, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(tbLock, WM_SETFONT, (WPARAM)hFont, TRUE);
		SetWindowSubclass(tbUnlock, tbUnlockProc, 0, 0);
		SetWindowSubclass(tbRestrict, tbLimitProc, 0, 0);
		SetWindowSubclass(tbLock, tbLockProc, 0, 0);

		return hOptionsWnd;
	}
}

namespace ui {

	int mainLoop(HINSTANCE hInstance, int nCmdShow) {
		hInst = hInstance;

		hFont = CreateFont(15, 0, 0, 0, 0, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEVICE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("MS Shell Dlg"));
		hStatusFont = CreateFont(15, 0, 0, 0, 600, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEVICE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("Tahoma"));

		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

		if (!createStatusWindow(hInstance)) return FALSE;
		if (!createOptionsWindow(hInstance)) return FALSE;

		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DeleteObject(hStatusFont);
		return msg.wParam;
	}

	void redrawStatusWindow() {
		InvalidateRect(hStatusWnd, NULL, TRUE);
		SetWindowPos(hStatusWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}
