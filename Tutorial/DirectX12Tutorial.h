#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <array>
#include <DirectXMath.h>

class DirectX12Tutorial
{
public:
	static constexpr UINT MBackBufferCount = 3;
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 mPosition{};
		DirectX::XMFLOAT4 mColor{};
	};

	struct BackBuffer
	{
		ID3D12Resource* mResource{};
		D3D12_CPU_DESCRIPTOR_HANDLE mCpuHandle{};
	};

	using BackBaffers = std::array<BackBuffer, MBackBufferCount>;
private:
	//DirectX12関連
	ID3D12Device* mDevice;
	ID3D12GraphicsCommandList* mCmdList;
	ID3D12CommandAllocator* mCmdAllocator;
	ID3D12CommandQueue* mCmdQueue;
	ID3D12Fence* mFence;
	ID3D12PipelineState* mPipelineState;
	ID3D12RootSignature* mRootSignature;
	ID3D12DescriptorHeap* mRtvDescriptorHeap;
	D3D12_CLEAR_VALUE mClearValue;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	BackBaffers mBackBaffers;
	ID3D12Resource* mVertexBuffer;

	//DXGI関連
	IDXGISwapChain3* mSwapChain;

	//その他
	UINT64 mSignalValue;
	UINT mRtvIncrementSize;
	UINT mBackBufferIndex;
private:
	//初期化関連の処理
	void ApplyDebugLayer();
	void CreateDevice();
	void CreateCmdObjects();
	void CreateSwapChainAndRenderTargets(const HWND);
	void CreateFence();
	void CreateRootSignature();
	void CreatePipelineState();
	void CreateVertexBuffer();

	//更新関連の処理
	void BeginRendering();
	void DrawPolygon();
	void EndRendering();
	void ExecuteCmdLists();
	void WaitForPreviousFrame();
	void Present();
	void CmdReset();
public:
	DirectX12Tutorial(const HWND);
	void Render();
	~DirectX12Tutorial();
};