#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <array>
#include <DirectXMath.h>

using namespace DirectX;

class DirectX12Tutorial
{
public:
	static constexpr UINT MBackBufferCount = 3;
	static constexpr UINT MSRVRootParameterIndex = 0;
	static constexpr UINT MCBVRootParameterIndex = 1;
	static constexpr UINT MSamplerRootParameterIndex = 2;
public:
	struct Vertex
	{
		XMFLOAT3 mPosition{};
		XMFLOAT3 mNormal{};
		XMFLOAT2 mTexcoords[4]{};
		XMFLOAT4 mColor{};
	};

	struct Scene
	{
		XMFLOAT4X4 mWorldViewProj{};
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
	ID3D12GraphicsCommandList* mCmdListForDrawing;
	ID3D12CommandAllocator* mCmdAllocatorForDrawing;
	ID3D12GraphicsCommandList* mCmdListForCopy;
	ID3D12CommandAllocator* mCmdAllocatorForCopy;
	ID3D12CommandQueue* mCmdQueue;
	ID3D12Fence* mFence;
	ID3D12PipelineState* mPipelineState2D;
	ID3D12PipelineState* mPipelineState3D;
	ID3D12RootSignature* mRootSignature;
	ID3D12DescriptorHeap* mRtvDescriptorHeap;
	ID3D12DescriptorHeap* mDsvDescriptorHeap;
	ID3D12DescriptorHeap* mCbvAndSrvDescriptorHeap;
	ID3D12DescriptorHeap* mSamplerDescriptorHeap;
	D3D12_CLEAR_VALUE mClearValue;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	BackBaffers mBackBaffers;
	ID3D12Resource* mDepthBuffer;
	ID3D12Resource* mConstantBuffer;
	ID3D12Resource* mTexture;
	ID3D12Resource* mTextureUploadBuffer;
	ID3D12Resource* mVertexBuffer2D;
	ID3D12Resource* mVertexBuffer3D;
	ID3D12Resource* mIndexBuffer3D;

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
	void CreateDepthStencilView();
	void LoadTexture();
	void CreateFence();
	void CreateRootSignature();
	void CreatePipelineState2D();
	void CreatePipelineState3D();
	void CreateVertexBuffer2D();
	void CreateVertexBuffer3D();

	//更新関連の処理
	void BeginRendering();
	void DrawPolygon2D();
	void DrawPolygon3D();
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