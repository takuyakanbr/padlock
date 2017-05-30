#include "stdafx.h"
#include "Resource.h"
#include <Commctrl.h>
#include <shellapi.h>

#include "ui.hpp"
#include "state.hpp"

#define UI_TRAYICON_UID 0x400
#define UI_TRAYICON_MSGID 0x410
#define UI_POPUPMENUITEM_SHOW_ID 0x05
#define UI_POPUPMENUITEM_EXIT_ID 0x06

using state::InputState;

namespace {
	RECT workArea;
	RECT statusWndSize = { 0, 0, 76, 18 };
	RECT optionsWndSize = { 0, 0, 430, 239 };

	WCHAR appTitle[51] = L"";
	const WCHAR statusWndClass[] = L"pl_status";
	const WCHAR optionsWndClass[] = L"pl_options";
	const WCHAR *statusTexts[] = { L"Default", L"Restricted", L"Locked" };
	const WCHAR *statusModeOptions[] = { L"Always show", L"Hide when unlocked", L"Always hide" };

	HINSTANCE hInst;
	HWND hStatusWnd;
	HWND hOptionsWnd;
	HWND tbUnlock;
	HWND tbLimit;
	HWND tbLock;
	HWND tbAutoLock;
	HWND cbStatusMode;
	HWND hTooltipWnd;

	HFONT hFont;
	HFONT hStatusFont;

	void createTrayIcon();
	void showStatusWindow();

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
		TextOut(hdc, 22, 123, L"Autolock (minutes):", 19);
		TextOut(hdc, 22, 155, L"Status box:", 11);

		SelectObject(hdc, hOldFont);
		EndPaint(hWnd, &ps);
	}


	LRESULT CALLBACK statusWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		static UINT taskbarCreatedMsgId;
		static HMENU hMenu;

		switch (message) {
		case UI_TRAYICON_MSGID:

			switch (LOWORD(lParam)) {
			case WM_LBUTTONDBLCLK:
				// show options window on double click
				if (state::isUnlocked())
					ShowWindow(hOptionsWnd, SW_SHOW);
				break;
			case WM_CONTEXTMENU:
				// display popup menu at cursor position
				{
					POINT p;
					GetCursorPos(&p);
					ShowWindow(hWnd, SW_SHOW);
					SetForegroundWindow(hWnd);
					TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, 
						p.x, p.y, 0, hWnd, NULL);
					break;
				}
			}
			break;

		case WM_EXITMENULOOP:
			// hide status window when popup menu is closed, if necessary
			if (state::isUnlocked() && state::getStatusMode() != STATE_STATUS_SHOWALWAYS)
				ShowWindow(hStatusWnd, SW_HIDE);
			break;
		case WM_COMMAND:
			// handles selection of popup menu items
			switch (wParam) {
			case UI_POPUPMENUITEM_SHOW_ID:
				if (state::isUnlocked())
					ShowWindow(hOptionsWnd, SW_SHOW);
				break;
			case UI_POPUPMENUITEM_EXIT_ID:
				if (state::isUnlocked())
					DestroyWindow(hWnd);
				break;
			}
			return 0;
		case WM_SETTINGCHANGE:
			// update status window position when work area changes
			SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
			SetWindowPos(hStatusWnd, NULL, workArea.right - statusWndSize.right, 
				workArea.bottom - statusWndSize.bottom, 0, 0, SWP_NOSIZE);
			return 0;
		case WM_CREATE:
			// create popup menu for tray icon
			hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, UI_POPUPMENUITEM_SHOW_ID, TEXT("Settings"));
			AppendMenu(hMenu, MF_STRING, UI_POPUPMENUITEM_EXIT_ID, TEXT("Exit"));

			// receive notification when taskbar is recreated
			taskbarCreatedMsgId = RegisterWindowMessageA("TaskbarCreated");

			break;
		case WM_PAINT:
			_Dc("uis: Repainting" << std::endl);
			repaintStatusWnd(hWnd);
			return 0;
		case WM_DESTROY:
			_Dc("uis: Quitting" << std::endl);
			PostQuitMessage(0);
			return 0;
		}

		// recreate the tray icon when taskbar is recreated
		if (message == taskbarCreatedMsgId) {
			createTrayIcon();
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
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
			state::notifyInputUpdate(seqId);
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
			// hide tooltip if mouse moves off the controls
			ShowWindow(hTooltipWnd, SW_HIDE);
			break;
		case WM_LBUTTONDOWN:
			// allow focus to be taken off the controls
			SetFocus(hOptionsWnd);
			break;
		case WM_PAINT:
			repaintOptionsWnd(hWnd);
			break;
		case WM_CLOSE:
		{
			// update state, save settings, and hide window
			CHAR arr[10];
			GetWindowTextA(tbAutoLock, arr, 10);
			SetWindowTextA(tbAutoLock, state::setAutoLock(arr).c_str());
			int index = (int)SendMessage(cbStatusMode, CB_GETCURSEL, NULL, NULL);
			SendMessage(cbStatusMode, CB_SETCURSEL, (WPARAM)state::setStatusMode(index), (LPARAM)0);
			showStatusWindow();
			state::notifyInputUpdate(STATE_KEYSEQ_NONE);
			ShowWindow(hWnd, SW_HIDE);
			break;
		}
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
			statusWndClass, appTitle, WS_POPUP,
			workArea.right - statusWndSize.right, workArea.bottom - statusWndSize.bottom,
			statusWndSize.right, statusWndSize.bottom,
			nullptr, nullptr, hInstance, nullptr);

		if (!hStatusWnd) return false;

		showStatusWindow();

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
			WS_EX_TOPMOST, optionsWndClass, appTitle, WS_CAPTION | WS_SYSMENU,
			x, y, optionsWndSize.right, optionsWndSize.bottom,
			nullptr, nullptr, hInstance, nullptr);

		// create controls
		tbUnlock = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_UNLOCKED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 23, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		tbLimit = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_LIMITED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 56, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		tbLock = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getSequence(STATE_KEYSEQ_LOCKED).c_str(),
			WS_CHILD | WS_VISIBLE, 120, 88, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		tbAutoLock = CreateWindowExA(WS_EX_CLIENTEDGE, "Edit",
			state::getAutoLock().c_str(), WS_CHILD | WS_VISIBLE,
			120, 120, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		cbStatusMode = CreateWindowA("ComboBox", "",
			CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			120, 152, 270, 22, hOptionsWnd, NULL, NULL, NULL);
		SendMessage(cbStatusMode, CB_ADDSTRING, NULL, (LPARAM)statusModeOptions[0]);
		SendMessage(cbStatusMode, CB_ADDSTRING, NULL, (LPARAM)statusModeOptions[1]);
		SendMessage(cbStatusMode, CB_ADDSTRING, NULL, (LPARAM)statusModeOptions[2]);
		SendMessage(cbStatusMode, CB_SETCURSEL, (WPARAM)state::getStatusMode(), (LPARAM)0);

		// use default system font for controls
		SendMessage(tbUnlock, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(tbLimit, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(tbLock, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(tbAutoLock, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessage(cbStatusMode, WM_SETFONT, (WPARAM)hFont, TRUE);

		// attach callback function to sequence textboxes
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

		// add tooltip to controls
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

		toolInfo.lpszText = L"Time (in minutes) of inactivity before automatically "
			"locking. Set to 0 to disable this feature.";
		toolInfo.uId = (UINT_PTR)tbAutoLock;
		SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

		toolInfo.lpszText = L"Select when the status box should be displayed.";
		toolInfo.uId = (UINT_PTR)cbStatusMode;
		SendMessage(hTooltipWnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

		return hOptionsWnd;
	}

	void showStatusWindow() {
		switch (state::getStatusMode()) {
		case STATE_STATUS_SHOWALWAYS:
			ShowWindow(hStatusWnd, SW_SHOW);
			break;
		case STATE_STATUS_HIDEWHENUNLOCKED:
			if (state::isUnlocked())
				ShowWindow(hStatusWnd, SW_HIDE);
			else
				ShowWindow(hStatusWnd, SW_SHOW);
		case STATE_STATUS_HIDEALWAYS:
			ShowWindow(hStatusWnd, SW_HIDE);
			break;
		}

	}

	void createTrayIcon() {
		NOTIFYICONDATA nid = { 0 };
		nid.cbSize = sizeof(nid);
		nid.hWnd = hStatusWnd;
		nid.uID = UI_TRAYICON_UID;
		nid.uCallbackMessage = UI_TRAYICON_MSGID;
		std::wcscpy(nid.szTip, L"Padlock");
#ifdef _WINXP
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
		nid.uVersion = NOTIFYICON_VERSION;
#else
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
		LoadIconMetric(hInst, MAKEINTRESOURCE(IDI_SMALL), LIM_SMALL, &(nid.hIcon));
		nid.uVersion = NOTIFYICON_VERSION_4;
#endif
		Shell_NotifyIcon(NIM_ADD, &nid);
		Shell_NotifyIcon(NIM_SETVERSION, &nid);
	}

	void deleteTrayIcon() {
		NOTIFYICONDATA nid = { 0 };
		nid.cbSize = sizeof(nid);
		nid.hWnd = hStatusWnd;
		nid.uID = UI_TRAYICON_UID;
		Shell_NotifyIcon(NIM_DELETE, &nid);
	}

}

namespace ui {

	int mainLoop(HINSTANCE hInstance, int nCmdShow) {
		std::wcscat(appTitle, APP_NAME);
		std::wcscat(appTitle, L" v");
		std::wcscat(appTitle, APP_VERSION);

		hInst = hInstance;

		hFont = CreateFont(15, 0, 0, 0, 0, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEVICE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("MS Shell Dlg"));
		hStatusFont = CreateFont(15, 0, 0, 0, 600, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEVICE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("Tahoma"));

		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

		if (!createStatusWindow(hInstance)) return FALSE;
		if (!createOptionsWindow(hInstance)) return FALSE;
		createTrayIcon();

		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		deleteTrayIcon();
		DeleteObject(hStatusFont);
		return msg.wParam;
	}

	void updateStatusWindow() {
		InvalidateRect(hStatusWnd, NULL, TRUE);
		SetWindowPos(hStatusWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		// update visibility of status window if necessary
		if (state::getStatusMode() == STATE_STATUS_HIDEWHENUNLOCKED) {
			if (state::isUnlocked())
				ShowWindow(hStatusWnd, SW_HIDE);
			else
				ShowWindow(hStatusWnd, SW_SHOW);
		}
	}

}
