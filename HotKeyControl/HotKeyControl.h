/* HotKeyControl.h */

#pragma once

#ifndef HOTKEYCONTROL_H
#define HOTKEYCONTROL_H

#include <windows.h>
#include <tchar.h>
#include <string>
#include <commctrl.h>

#include "resource.h"

#ifndef UNICODE  
	typedef std::string String;
#else
	typedef std::wstring String;
#endif

/* window class */
#define HOTKEYCONTROL_WC   _T("HotKeyControl")

/* register/unregister the window class */
void HotKeyWindowRegister(void);
void HotKeyWindowUnregister(void);

/* hotkey functions */
bool IsKeyMsg(UINT);
void ProcessSetHotKey(MSG);
LRESULT CALLBACK HotKeyEditProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
LRESULT CALLBACK HotKeyStaticProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
void RegisterNewHotKey();
void EndHotKeyEditing();

#endif  /* HOTKEYCONTROL_H */
