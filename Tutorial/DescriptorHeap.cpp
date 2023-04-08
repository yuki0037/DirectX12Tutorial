#include "DescriptorHeap.h"
#include "Misc.h"
#include <algorithm>

DescriptorHeap::DescriptorHeap(
	ID3D12Device* device,
	UINT numDescriptors,
	D3D12_DESCRIPTOR_HEAP_TYPE type)
	:mDescriptorHeap(nullptr),
	mNumDescriptors(std::max(1u, numDescriptors)),
	mIncrementSize(device->GetDescriptorHandleIncrementSize(type))
{
	const D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc
	{
		type,
		mNumDescriptors,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		0
	};

	HRESULT hr = device->CreateDescriptorHeap(
		&descriptorHeapDesc,
		IID_PPV_ARGS(&mDescriptorHeap));
	HRESULT_ASSERT(hr);
}

DescriptorHandle DescriptorHeap::GetHandle(UINT index) const
{
	assert(mNumDescriptors > index);
	const UINT offset = mIncrementSize * index;
	auto cpuHandle = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += static_cast<SIZE_T>(offset);
	auto gupHandle = mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	gupHandle.ptr += static_cast<UINT64>(offset);
	return std::make_tuple(cpuHandle, gupHandle);
}

DescriptorHeap::~DescriptorHeap()
{
	SAFE_RELEASE(mDescriptorHeap);
}

DescriptorHandle DescriptorAllocator::Allocation(
	const DescriptorHeap* descriptorHeap,
	UINT* inoutStartIndex,
	UINT* inoutNumHandle,
	UINT* optoutIncrementSize)
{
	const UINT startIndex = (*inoutStartIndex);
	const UINT remainingNumHandle = descriptorHeap->GetNumDescriptors() - startIndex;
	if ((*inoutNumHandle) > remainingNumHandle)
	{
		(*inoutNumHandle) = remainingNumHandle;
	}
	if (optoutIncrementSize)
	{
		(*optoutIncrementSize) = descriptorHeap->GetIncrementSize();
	}
	(*inoutStartIndex) += (*inoutNumHandle);
	return descriptorHeap->GetHandle(startIndex);
}

DescriptorAllocator::DescriptorAllocator(
	ID3D12Device* device,
	UINT numRtv,
	UINT numDsv,
	UINT numCbvSrvUav,
	UINT numSampler)
	:mRtv(device, numRtv, D3D12_DESCRIPTOR_HEAP_TYPE_RTV),
	mRtvIndex(0),
	mDsv(device, numDsv, D3D12_DESCRIPTOR_HEAP_TYPE_DSV),
	mDsvIndex(0),
	mCbvSrvUav(device, numCbvSrvUav, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
	mCbvSrvUavIndex(0),
	mSampler(device, numSampler, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
	mSamplerIndex(0)
{

}

DescriptorHandle DescriptorAllocator::Allocation(
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	UINT* inoutNumHandle,
	UINT* optoutIncrementSize)
{
	assert(inoutNumHandle != nullptr && type != D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);

	DescriptorHandle handle;
	switch (type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		handle = Allocation(&mCbvSrvUav, &mCbvSrvUavIndex, inoutNumHandle, optoutIncrementSize);
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		handle = Allocation(&mSampler, &mSamplerIndex, inoutNumHandle, optoutIncrementSize);
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		handle = Allocation(&mRtv, &mRtvIndex, inoutNumHandle, optoutIncrementSize);
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		handle = Allocation(&mDsv, &mDsvIndex, inoutNumHandle, optoutIncrementSize);
		break;
	}
	return handle;
}

const DescriptorHeap* DescriptorAllocator::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) const
{
	assert(type != D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);

	const DescriptorHeap* descriptorHeap = nullptr;
	switch (type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		descriptorHeap = &mCbvSrvUav;
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		descriptorHeap = &mSampler;
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		descriptorHeap = &mRtv;
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		descriptorHeap = &mDsv;
		break;
	}
	return descriptorHeap;
}

DescriptorAllocator::~DescriptorAllocator()
{
}
