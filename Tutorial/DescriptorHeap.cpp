#include "DescriptorHeap.h"
#include "Misc.h"

DescriptorHeap::DescriptorHeap(
	ID3D12Device* device,
	UINT numDescriptors,
	D3D12_DESCRIPTOR_HEAP_TYPE type)
	:mDescriptorHeap(nullptr),
	mIncrementSize(0)
{
	const D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc
	{
		type,
		numDescriptors,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0
	};

	HRESULT hr = device->CreateDescriptorHeap(
		&descriptorHeapDesc,
		IID_PPV_ARGS(&mDescriptorHeap));
	HRESULT_ASSERT(hr);

	mIncrementSize = device->GetDescriptorHandleIncrementSize(type);
}

DescriptorHandle DescriptorHeap::GetHandle(UINT index) const
{
	const UINT offset = mIncrementSize * index;
	DescriptorHandle handle;
	GET_CPU_HANDLE(handle) = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	GET_CPU_HANDLE(handle).ptr += static_cast<SIZE_T>(offset);
	GET_GPU_HANDLE(handle) = mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	GET_GPU_HANDLE(handle).ptr += static_cast<UINT64>(offset);
	return handle;
}

DescriptorHeap::~DescriptorHeap()
{
	SAFE_RELEASE(mDescriptorHeap);
}
