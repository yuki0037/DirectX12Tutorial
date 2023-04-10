#include "Window.h"
#include "DirectX12Tutorial.h"
#include "Misc.h"

int main(char* [], int)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	WindowDesc windowDesc{};
	windowDesc.mCaption = L"DirectX12で三角形を描画する簡単なサンプル";
	Window* window = _NEW Window(windowDesc);
	DirectX12Tutorial* tutorial = _NEW DirectX12Tutorial(window->GetHandle());

	while (window->Dispatch())
	{
		tutorial->Render();
	}

	SAFE_DELETE(tutorial);
	SAFE_DELETE(window);
	return 0;
}