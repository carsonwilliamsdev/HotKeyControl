/* HotKeyControl.cpp */

#include "HotKeyControl.h"


// hotkey vars
bool editingHotKey = false;
bool isAltDown, isCtrlDown, isShiftDown, isWinDown = false;
int keyDown = 0;
int hotKey = 0;
int hotKeyId = 0;
bool hotKeySet = false;

HWND g_hwnd, g_parentHwnd, hHotKeyBtn, hHotKeyStatic;

void EndHotKeyEditing()
{
    editingHotKey = false;
    RegisterNewHotKey();
    SetWindowText(hHotKeyBtn, _T("Edit Keybinding"));
}

LRESULT CALLBACK HotKeyStaticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_KILLFOCUS:
        {
            // end editing if focus is lost
            // ignore for hotkey button since it already ends editing
            if ((HWND)wParam != hHotKeyBtn)
            {
                EndHotKeyEditing();
            }
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

bool IsKeyMsg(UINT msg)
{
    return (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN || msg == WM_KEYUP || msg == WM_SYSKEYUP);
}

void ProcessSetHotKey(MSG msg)
{
    bool isKeyDown = (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN);
    if (isKeyDown && hotKeySet)
    {
        // setting a new hotkey
        if (hotKeySet)
        {
            isAltDown = (GetKeyState(VK_MENU) < 0);
            isCtrlDown = (GetKeyState(VK_CONTROL) < 0);
            isShiftDown = (GetKeyState(VK_SHIFT) < 0);
            isWinDown = (GetKeyState(VK_LWIN) < 0) || (GetKeyState(VK_RWIN) < 0);
            hotKeySet = false;
            keyDown = 0;
            hotKey = 0;
        }
    }

    bool isKeyMod = false;

    // ignore keyups after hotkey is set
    if (!isKeyDown && hotKeySet)
    {
        return;
    }

    // check modifiers
    switch (msg.wParam)
    {
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU:
        {
            isAltDown = isKeyDown;
            isKeyMod = true;
            break;
        }
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
        {
            isCtrlDown = isKeyDown;
            isKeyMod = true;
            break;
        }
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
        {
            isShiftDown = isKeyDown;
            isKeyMod = true;
            break;
        }
        case VK_LWIN:
        case VK_RWIN:
        {
            isWinDown = isKeyDown;
            isKeyMod = true;
            break;
        }
    }

    // check for hotkey press
    if (!isKeyMod && isKeyDown)
    {
        keyDown = msg.lParam;
        hotKey = msg.wParam;
        hotKeySet = true;
    }

    String output(_T(""));
    if (isCtrlDown)
        output.append(_T("ctrl + "));
    if (isAltDown)
        output.append(_T("alt + "));
    if (isShiftDown)
        output.append(_T("shift + "));
    if (isWinDown)
        output.append(_T("win + "));
    if (keyDown != 0)
    {
        TCHAR keynamebuffer[200];
        if (GetKeyNameText(keyDown, keynamebuffer, (sizeof(keynamebuffer)-1)) != 0)
        {
            output.append(keynamebuffer);
        }
    }
    if (output.empty()) output.append(_T("none"));

    SetWindowText(hHotKeyStatic, output.c_str());
}

void RegisterNewHotKey()
{
    if (hotKey == 0) return; // no hotkey to set

    if (hotKeyId != 0)
    {
        if (UnregisterHotKey(g_hwnd, hotKeyId) == 0)
        {
            int err = GetLastError();
            MessageBox(g_hwnd, _T("Error unregistering previous HotKey!"), _T("Error"), MB_OK);
        }
    }
    else
    {
        hotKeyId = ID_HOTKEY;
    }

    hotKeyId++;
    UINT mods = MOD_NOREPEAT;
    if (isAltDown) mods = (mods | MOD_ALT);
    if (isCtrlDown) mods = (mods | MOD_CONTROL);
    if (isShiftDown) mods = (mods | MOD_SHIFT);
    if (isWinDown) mods = (mods | MOD_WIN);
    if (RegisterHotKey(
        g_hwnd,
        hotKeyId,
        mods,
        hotKey) == 0)
    {
        int err = GetLastError();
        if (err == ERROR_HOTKEY_ALREADY_REGISTERED)
        {
            MessageBox(g_hwnd, _T("Error setting HotKey! The HotKey is already registered."), _T("Error"), MB_OK);
        }
        else
        {
            MessageBox(g_hwnd, _T("Error setting HotKey!"), _T("Error"), MB_OK);
        }
    }
}

static LRESULT CALLBACK HotKeyControlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_HOTKEY_BUTTON:
                {
                    if (!editingHotKey)
                    {
                        editingHotKey = true;
                        SetWindowText(hHotKeyBtn, _T("Stop Recording"));
                        SetFocus(hHotKeyStatic);
                        
                        while (editingHotKey)
                        {
                            MSG msg;
                            while (PeekMessage(&msg, g_parentHwnd, 0, 0, PM_REMOVE))
                            {
                                if (msg.message != WM_QUIT && IsKeyMsg(msg.message))
                                {
                                    ProcessSetHotKey(msg);
                                }
                                else
                                {
                                    TranslateMessage(&msg);
                                    DispatchMessage(&msg);
                                }
                            }
                        }

                    }
                    else
                    {
                        EndHotKeyEditing();
                    }
                    break;
                }
            }
            break;
        }
        case WM_CREATE:
        {
            g_hwnd = hwnd;
            g_parentHwnd = GetParent(g_hwnd);
            if (g_parentHwnd == NULL)
            {
                MessageBox(NULL, _T("HotKeyControl: No parent window detected!"), _T("Warning"), MB_OK);
            }

            hHotKeyStatic = CreateWindow(
                _T("STATIC"),
                _T("none"),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | ES_READONLY | ES_CENTER,
                10, 10, 200, 20,
                hwnd,
                (HMENU)IDC_HOTKEY_STATIC,
                GetModuleHandle(NULL),
                NULL);

            hHotKeyBtn = CreateWindow(
                _T("BUTTON"),
                _T("Edit Keybinding"),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                215, 10, 120, 20,
                hwnd,
                (HMENU)IDC_HOTKEY_BUTTON,
                GetModuleHandle(NULL),
                NULL);

            SetWindowSubclass(hHotKeyStatic, HotKeyStaticProc, 0, 0);

            break;
        }
        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
            if (pDIS->hwndItem == hHotKeyBtn)
            {
                COLORREF backgroundColor = editingHotKey ? RGB(255, 0, 0) : GetSysColor(COLOR_BTNFACE);
                COLORREF textColor = editingHotKey ? RGB(255, 255, 255) : GetSysColor(COLOR_BTNTEXT);
                HBRUSH bgBrush = CreateSolidBrush(backgroundColor);
                RECT crect;
                GetClientRect(hHotKeyBtn, &crect);
                FillRect(pDIS->hDC, &crect, bgBrush);
                SetBkColor(pDIS->hDC, backgroundColor);
                int textLength = GetWindowTextLength(hHotKeyBtn);
                SetTextColor(pDIS->hDC, textColor);
                TCHAR staticText[99] = {};
                int len = SendMessage(hHotKeyBtn, WM_GETTEXT, ARRAYSIZE(staticText), (LPARAM)staticText);
                TextOut(pDIS->hDC, pDIS->rcItem.left + (23 - len), pDIS->rcItem.top + 2, staticText, len);
            }
            return TRUE;
        }
        case WM_HOTKEY:
        {
            if (wParam == hotKeyId)
            {
                // TODO: hotkey stuff
                MessageBox(g_hwnd, _T("HotKey was pressed!"), _T("HotKey Test Message"), MB_OK);
            }
            break;
        }
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            break;
        }
        case WM_DESTROY:
        {
            editingHotKey = false;
            if (hotKeyId != 0)
            {
                if (UnregisterHotKey(g_hwnd, hotKeyId) == 0)
                {
                    MessageBox(NULL, _T("Error unregistering HotKey!"), _T("Error"), MB_OK);
                }
            }
            PostQuitMessage(0);
            break;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void HotKeyWindowRegister(void)
{
    WNDCLASS wc = { 0 };
    wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = HotKeyControlProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = HOTKEYCONTROL_WC;
    RegisterClass(&wc);
}

void HotKeyWindowUnregister(void)
{
    UnregisterClass(HOTKEYCONTROL_WC, NULL);
}
