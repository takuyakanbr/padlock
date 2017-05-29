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
	HWND tbUnlock;
	HWND tbLimit;
	HWND tbLock;
	HWND hTooltipWnd;

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
		case WM_SETTINGCHANGE:
			// update status window position when work area changes
			SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
			SetWindowPos(hStatusWnd, NULL, workArea.right - statusWndSize.right, 
				workArea.bottom - statusWndSize.bottom, 0, 0, SWP_NOSIZE);
			break;
		case WM_LBUTTONUP:
			// show options window on left click
			if (state::isUnlocked()) {
				ShowWindow(hOptionsWnd, SW_SHOW);
				UpdateWindow(hOptionsWnd);
			}
			break;
		case WM_RBUTTONUP:
			// destroy window (and quit application) on right click
			if (state::isUnlocked())
				DestroyWindow(hWnd);
			break;
		case WM_PAINT:
			_Dc("uis: Repainting" << std::endl);
			repaintStatusWnd(hWnd);
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

	inline LRESULT CALLBACK seqTextboxProc(const int seqId, HWND hWnd, UINT message, 
		WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			// update the sequence, and the displayed text
			std::string str = state::updateSequence(seqId, wParam);
			SetWindowTextA(hWnd, str.c_str());
			return 0;
		}
		case WM_CHAR:
		case WM_DEADCHAR:
			// catch these so that the text does not get modified
			return 0;
		case WM_KILLFOCUS:
			// reset the displayed text when focus is lost
			SetWindowTextA(hWnd, state::getSequence(seqId).c_str());
			break;
		case WM_LBUTTONDOWN:
			// when clicked: set this sequence to be updated, and clear the displayed text
			state::notifyUpdate(seqId);
			SetWindowTextA(hWnd, "");
			break;
		}
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK tbUnlockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
		return seqTextboxProc(STATE_KEYSEQ_UNLOCKED, hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK tbLimitProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
		return seqTextboxProc(STATE_KEYSEQ_LIMITED, hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK tbLockProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
		return seqTextboxProc(STATE_KEYSEQ_LOCKED, hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK optionsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_MOUSEMOVE:
			// hide tooltip if mouse moves off the textboxes
			ShowWindow(hTooltipWnd, SW_HIDE);
			break;
		case WM_LBUTTONDOWN:
			// allow focus to be taken off the textboxes
			SetFocus(hOptionsWnd);
			break;
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
		tbUnlock = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_UNLOCKED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 23, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		tbLimit = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_LIMITED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 56, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		tbLock = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_LOCKED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 88, 270, 22, hOptionsWnd, NULL, NULL, NULL);

		SendMessage(tbUnlock, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(tbLimit, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(tbLock, WM_SETFONT, (WPARAM)hFont, TRUE);
		SetWindowSubclass(tbUnlock, tbUnlockProc, 0, 0);
		SetWindowSubclass(tbLimit, tbLimitProc, 0, 0);
		SetWindowSubclass(tbLock, tbLockProc, 0, 0);

		// create tooltip window
		hTooltipWnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			hOptionsWnd, NULL,
			hInst, NULL);

		// add tooltip to textboxes
		TOOLINFO toolInfo;
		toolInfo.cbSize = sizeof(toolInfo);
		toolInfo.hwnd = hOptionsWnd;
		toolInfo.uId = (UINT_PTR)tbUnlock;
		toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		toolInfo.lpszText = L"Left click to begin specifying a sequence "
			"of up to 10 inputs. Click again to reset.";
		SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
		toolInfo.uId = (UINT_PTR)tbLimit;
		SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
		toolInfo.uId = (UINT_PTR)tbLock;
		SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

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
