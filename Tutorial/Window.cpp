#include "Window.h"
#pragma comment( lib, "winmm.lib" )

LRESULT CALLBACK Window::WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR user = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (msg == WM_DESTROY)
	{
		PostMessage(hWnd, WM_CLOSE, wParam, lParam);
	}
	else if ((msg == WM_CLOSE) && (user != 0))
	{
		reinterpret_cast<Window*>(user)->mClosed = TRUE;
	}
	else if (msg == WM_MOVE && (user != 0))
	{
		RECT rect{};
		GetWindowRect(hWnd, &rect);
		WindowDesc* winDesc = &reinterpret_cast<Window*>(user)->mWindowDesc;
		winDesc->mX = rect.left;
		winDesc->mY = rect.top;
	}

	if (user != 0)
	{
		return reinterpret_cast<Window*>(user)->RunProcedure(hWnd, msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT Window::RunProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return (mWindowDesc.mWndProc != NULL) ?
		mWindowDesc.mWndProc(hWnd, msg, wParam, lParam) :
		DefWindowProc(hWnd, msg, wParam, lParam);
}

Window::Window(WindowDesc windowDesc)
	:mHandle(NULL),
	mWindowDesc(windowDesc),
	mClosed(FALSE)
{
	if (mWindowDesc.mClassName.empty())
	{
		mWindowDesc.mClassName = std::to_wstring(reinterpret_cast<LONG_PTR>(this));
	}
	if (mWindowDesc.mInstance == NULL)
	{
		mWindowDesc.mInstance = GetModuleHandle(NULL);
	}

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = mWindowDesc.mClassStyle;
	wcex.lpfnWndProc = WindowProcedure;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = mWindowDesc.mInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = mWindowDesc.mClassName.c_str();
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(mWindowDesc.mIconHandle));
	RegisterClassEx(&wcex);

	RECT rc{ 0,0,mWindowDesc.mWidth,mWindowDesc.mHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	int width = static_cast<int>(rc.right - rc.left);
	int height = static_cast<int>(rc.bottom - rc.top);

	mHandle = CreateWindowW(
		wcex.lpszClassName,
		mWindowDesc.mCaption.c_str(),
		mWindowDesc.mWindowStyle,
		mWindowDesc.mX,
		mWindowDesc.mY,
		width,
		height,
		mWindowDesc.mWndParent,
		NULL,
		mWindowDesc.mInstance,
		NULL);
	ShowWindow(mHandle, mWindowDesc.mCmdShow);

	GetWindowRect(mHandle, &rc);
	mWindowDesc.mX = rc.left;
	mWindowDesc.mY = rc.top;

	SetWindowLongPtr(mHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

BOOL Window::Dispatch()
{
	if (mClosed) { return FALSE; }
	MSG msg = {};
	while (PeekMessageW(&msg, mHandle, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_CLOSE)
		{
			mClosed = TRUE;
		}
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return !mClosed;
}

BOOL Window::SetCaption(const std::wstring& newCaption)
{
	BOOL result = SetWindowTextW(mHandle, newCaption.c_str());
	if (result == TRUE)
	{
		mWindowDesc.mCaption = newCaption;
	}
	return result;
}

Window::~Window()
{
	PostMessage(mHandle, WM_CLOSE, NULL, NULL);
	UnregisterClass(mWindowDesc.mClassName.c_str(), mWindowDesc.mInstance);
}