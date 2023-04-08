#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class DirectX12Tutorial
{
private:
	ID3D12Device* mDevice;
	ID3D12GraphicsCommandList* mCmdList;
	ID3D12CommandAllocator* mCmdAllocator;
	ID3D12CommandQueue* mCmdQueue;
	ID3D12Fence* mFence;
	ID3D12PipelineState* mPipelineState;
	ID3D12RootSignature* mRootSignature;
	ID3D12DescriptorHeap* mDescriptorHeap;
public:

};