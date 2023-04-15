#define _WIN32_WINNT 0x0400

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4311)

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include "resource.h"
#include <thread>

const char className[] = "AutoCLicker";

NOTIFYICONDATA structNID;
HINSTANCE gHInstance;
HICON hMainIcon;
POINT clickPos;
POINT mousePos;

HMENU menu;
HMENU speedMenu;
HMENU modeMenu;

#define	WM_USER_SHELLICON (WM_USER + 1)

#define ID_EXIT     (WM_USER + 2)
#define ID_ABOUT    (WM_USER + 3)

#define ID_SPEED    (WM_USER + 10)
#define ID_MODE     (WM_USER + 30)

#define ID_TIMER    (WM_USER + 50)

//#define MOUSE

int speed = 2;
std::string mode = "switch";

bool on = 0;

LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        PMSLLHOOKSTRUCT p = (PMSLLHOOKSTRUCT)lParam;

        if (p->flags != LLMHF_INJECTED) {
            if (LOWORD(wParam) == WM_LBUTTONDOWN) { // Only true when LMOUSE is first pressed down
                on = true;
            }
            else if (LOWORD(wParam) == WM_LBUTTONUP) {
                on = false;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK KeybHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    bool eat = false;
    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

        if (p->vkCode == VK_LCONTROL) {
            if (mode == "hold")
                on = (LOWORD(wParam) == WM_KEYDOWN);
            if (mode == "switch")
                on = ((on && !(LOWORD(wParam) == WM_KEYDOWN)) || (!on && (LOWORD(wParam) == WM_KEYDOWN))); // NEQV
            eat = true;
        }
    }
    return (eat ? true : CallNextHookEx(NULL, nCode, wParam, lParam));
}

void click() {
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep((speed == 3) * 15 + (speed == 2) * 40 + (speed == 1) * 90 + (rand() % 21));
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    Sleep((speed == 3) * 15 + (speed == 2) * 40 + (speed == 1) * 90 + (rand() % 21));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
    {
        if (on)
            click();
    }
    break;
    case WM_CREATE:
    {
        SetTimer(hwnd, ID_TIMER, 5, NULL);
    }
    break;
    case WM_CLOSE:
    {
        KillTimer(hwnd, ID_TIMER);
        DestroyWindow(hwnd);
    }
    break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    case WM_USER_SHELLICON:
    {
        switch (LOWORD(lParam)) {
        case WM_RBUTTONDOWN:
        {
            GetCursorPos(&clickPos);
            menu = CreatePopupMenu();
            speedMenu = CreatePopupMenu();
            modeMenu = CreatePopupMenu();

            char temp[3];

            for (int i = 3; i > 0; --i) {
                itoa(i, temp, 10);
                InsertMenu(speedMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_SPEED + i, temp);
            }

            InsertMenu(modeMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MODE + 2, "switch");
            InsertMenu(modeMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MODE + 1, "hold");

            CheckMenuItem(speedMenu, ID_SPEED + speed, MF_CHECKED);
            CheckMenuItem(modeMenu, ID_MODE + (mode == "switch" ? 2 : 1), MF_CHECKED);

            InsertMenu(menu, 0xFFFFFFFF, MF_POPUP, (UINT)speedMenu, "Speed");
            InsertMenu(menu, 0xFFFFFFFF, MF_POPUP, (UINT)modeMenu, "Mode");

            InsertMenu(menu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_ABOUT, "&About");
            InsertMenu(menu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_EXIT, "&Exit");

            SetForegroundWindow(hwnd);

            TrackPopupMenu(
                menu,
                TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
                clickPos.x,
                clickPos.y,
                0,
                hwnd,
                NULL);

            SendMessage(hwnd, WM_NULL, 0, 0);
        }
        break;
        }
    }
    break;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
        case ID_EXIT:
        {
            DestroyWindow(hwnd);
        }
        break;
        case ID_ABOUT:
        {
            MessageBox(hwnd, "Autoclicker: 3 Fast (~20 CPS), 2 Normal (~10 CPS), 1 Slow (~5 CPS)", "About", MB_OK);
        }
        break;
        }
        for (int i = ID_SPEED; i <= ID_SPEED + 3; ++i) {
            if (LOWORD(wParam) == i) {
                CheckMenuItem(speedMenu, ID_SPEED + speed, MF_UNCHECKED);
                CheckMenuItem(speedMenu, i, MF_CHECKED);
                speed = i - ID_SPEED;
            }
        }
        for (int i = ID_MODE + 1; i <= ID_MODE + 2; ++i) {
            if (LOWORD(wParam) == i) {
                CheckMenuItem(modeMenu, ID_MODE + (mode == "switch" ? 2 : 1), MF_UNCHECKED);
                CheckMenuItem(modeMenu, i, MF_CHECKED);
                mode = (i - ID_MODE) == 2 ? "switch" : "hold";
            }
        }
    }
    break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    HWND prev = FindWindow(className, NULL);
    if (prev)
        return 1;
#ifdef MOUSE
    HHOOK hhMouse = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, hInstance, 0);
#else
    HHOOK hhKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, KeybHookCallback, hInstance, 0);
#endif

    hMainIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAYICON));

    gHInstance = hInstance;

    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_TRAYICON));
    wc.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_TRAYICON));
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = className;
    wc.lpszMenuName = NULL;
    wc.style = 0;

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Unable to register window class.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        className,
        "AutoClicker",
        NULL,
        0, 0, 0, 0,
        NULL, NULL,
        hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Unable to create window.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);

    structNID.cbSize = sizeof(NOTIFYICONDATA);
    structNID.hWnd = hwnd;
    structNID.uID = IDI_TRAYICON;
    structNID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    strcpy(structNID.szTip, "AutoClicker");
    structNID.hIcon = hMainIcon;
    structNID.uCallbackMessage = WM_USER_SHELLICON;

    Shell_NotifyIcon(NIM_ADD, &structNID);

    while (GetMessage(&msg, hwnd, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#ifdef MOUSE
    UnhookWindowsHookEx(hhMouse);
#else
    UnhookWindowsHookEx(hhKeyboard);
#endif

    return 0;
}