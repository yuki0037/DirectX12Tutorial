#pragma once
#include <string>
#include <Windows.h>
#include "HighResolutionTimer.h"

struct WindowDesc
{
	HWND mWndParent{ NULL };
	WNDPROC mWndProc{ NULL };
	std::wstring mClassName{};
	std::wstring mCaption{};
	UINT mClassStyle{ CS_HREDRAW | CS_VREDRAW };
	DWORD mWindowStyle{ WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE };
	HINSTANCE mInstance{ GetModuleHandle(NULL) };
	UINT mIconHandle{ 0 };
	INT mX{ CW_USEDEFAULT };
	INT mY{ CW_USEDEFAULT };
	LONG mWidth{ 1280 };
	LONG mHeight{ 720 };
	INT mCmdShow{ SW_SHOWDEFAULT };
};

class Window
{
private:
	using Timer = HighResolutionTimer;
private:
	HWND mHandle;
	WindowDesc mWindowDesc;
	BOOL mClosed;
	Timer mTimer;
private:
	static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
	LRESULT RunProcedure(HWND, UINT, WPARAM, LPARAM);
public:
	Window(WindowDesc windowDesc);
	bool IsActivate()const;
	double GetElapsedTime() const;
	BOOL Dispatch();
	const WindowDesc& GetWindowDesc()const { return mWindowDesc; }
	const HWND GetHandle()const { return mHandle; }
	BOOL PostMsg(UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL SetCaption(const std::wstring& newCaption);
	~Window();
};