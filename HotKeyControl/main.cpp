/* main.c
 * basic window for testing HotKeyControl
 */

#include <tchar.h>
#include <windows.h>

#include "HotKeyControl.h"


static HINSTANCE hInst;
static HWND hHotKeyEditor, hTestEdit, hWindowsHotKeyControl;

#define IDC_HOTKEYEDITOR    1337
#define IDC_TESTEDIT        9001

HWND hwndHot = NULL;

static LRESULT CALLBACK MainProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            hHotKeyEditor = CreateWindow(HOTKEYCONTROL_WC, NULL, WS_CHILD | WS_VISIBLE,
                10, 10, 800, 50, hwnd, (HMENU)IDC_HOTKEYEDITOR, hInst, NULL);
            
            hTestEdit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"),
                _T(" [TestEdit]\r\n click in here to lose focus\r\n longest key combo = ctrl + alt + shift + Caps Lock?\r\n my control on top, CommonControl on bottom"), 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                10, 70, 375, 100, hwnd, (HMENU)IDC_TESTEDIT, hInst, NULL);
            
            // Ensure that the common control DLL is loaded. 
            INITCOMMONCONTROLSEX icex;  //declare an INITCOMMONCONTROLSEX Structure
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_HOTKEY_CLASS;   //set dwICC member to ICC_HOTKEY_CLASS    
                                             // this loads the Hot Key control class.
            InitCommonControlsEx(&icex);

            hWindowsHotKeyControl = CreateWindowEx(0,                        // no extended styles 
                HOTKEY_CLASS,             // class name 
                TEXT(""),                 // no title (caption) 
                WS_CHILD | WS_VISIBLE,    // style 
                10, 180,                   // position 
                200, 20,                  // size 
                hwnd,                  // parent window 
                NULL,                     // uses class menu 
                hInst,                  // instance 
                NULL);                    // no WM_CREATE parameter 

            SetFocus(hWindowsHotKeyControl);

            // Set rules for invalid key combinations. If the user does not supply a
            // modifier key, use ALT as a modifier. If the user supplies SHIFT as a 
            // modifier key, use SHIFT + ALT instead.
            SendMessage(hWindowsHotKeyControl,
                HKM_SETRULES,
                (WPARAM)HKCOMB_NONE | HKCOMB_S,   // invalid key combinations 
                MAKELPARAM(HOTKEYF_ALT, 0));       // add ALT to invalid entries 

            // Set CTRL + ALT + A as the default hot key for this window. 
            // 0x41 is the virtual key code for 'A'. 
            SendMessage(hwndHot,
                HKM_SETHOTKEY,
                MAKEWORD(0x41, HOTKEYF_CONTROL | HOTKEYF_ALT),
                0);
            
            break;
        }
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    WNDCLASS wc = { 0 };
    HWND hwnd;
    MSG msg;

    hInst = hInstance;

    HotKeyWindowRegister();

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = _T("MainWindow");
    RegisterClass(&wc);

    hwnd = CreateWindow(_T("MainWindow"), _T("HotKeyControl Test Window"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (IsDialogMessage(hwnd, &msg))
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    HotKeyWindowUnregister();

    return (int)msg.wParam;
}
