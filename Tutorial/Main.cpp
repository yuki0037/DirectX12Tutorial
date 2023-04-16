#include "Window.h"
#include "DirectX12Tutorial.h"
#include "Misc.h"
#include <sstream>
#include <iostream>

int main(char* [], int)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	WindowDesc windowDesc{};
	windowDesc.mWidth = 640;
	windowDesc.mHeight = 640;
	windowDesc.mCaption = L"DirectX12で三角形を描画する簡単なサンプル";
	windowDesc.mWindowStyle = WS_OVERLAPPEDWINDOW;
	Window* window = _NEW Window(windowDesc);
	DirectX12Tutorial* tutorial = _NEW DirectX12Tutorial(window->GetHandle());

	std::cout << HRESULT_TRACE_STRING(-3) << std::endl;
	std::cout << HRESULT_TRACE_STRING(-2) << std::endl;
	std::cout << HRESULT_TRACE_STRING(-1) << std::endl;
	std::cout << HRESULT_TRACE_STRING(0) << std::endl;
	std::cout << HRESULT_TRACE_STRING(1) << std::endl;
	std::cout << HRESULT_TRACE_STRING(2) << std::endl;
	std::cout << HRESULT_TRACE_STRING(3) << std::endl;
	std::cout << HRESULT_TRACE_STRING(E_NOTIMPL) << std::endl;

	while (window->Dispatch())
	{
		std::wstringstream ss;
		windowDesc = window->GetWindowDesc();
		ss << std::boolalpha << window->IsActivate() << L" : ";
		ss << L"[" << windowDesc.mWidth << L"*" << windowDesc.mHeight << L"]";
		ss << L"x : " << windowDesc.mX << L" y : " << windowDesc.mY;
		window->SetCaption(ss.str());
		if (GetKeyState(VK_ESCAPE) & 0x80)
		{
			window->PostMsg(WM_CLOSE, 0, 0);
		}

		tutorial->Render();
	}

	SAFE_DELETE(tutorial);
	SAFE_DELETE(window);
	return 0;
}