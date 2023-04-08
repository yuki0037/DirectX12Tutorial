#pragma once
#include <map>
#include <tuple>
#include <d3d12.h>

using DescriptorHandle = std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>;

#define GET_CPU_HANDLE	std::get<0>
#define GET_GPU_HANDLE	std::get<1>

class DescriptorHeap
{
private:
	ID3D12DescriptorHeap* mDescriptorHeap;
	const UINT mNumDescriptors;
	const UINT mIncrementSize;
public:
	DescriptorHeap(ID3D12Device* device,
		UINT numDescriptors,
		D3D12_DESCRIPTOR_HEAP_TYPE type);
	[[nodiscard]] DescriptorHandle GetHandle(UINT index)const;
	[[nodiscard]] const ID3D12DescriptorHeap* GetDescriptorHeap()const { return mDescriptorHeap; }
	[[nodiscard]] const UINT GetNumDescriptors()const { return mNumDescriptors; }
	[[nodiscard]] const UINT GetIncrementSize()const { return mIncrementSize; }
	virtual ~DescriptorHeap();
};

class DescriptorAllocator
{
private:
	const DescriptorHeap mRtv;
	UINT mRtvIndex;
	const DescriptorHeap mDsv;
	UINT mDsvIndex;
	const DescriptorHeap mCbvSrvUav;
	UINT mCbvSrvUavIndex;
	const DescriptorHeap mSampler;
	UINT mSamplerIndex;
private:
	static DescriptorHandle Allocation(
		const DescriptorHeap* descriptorHeap,
		UINT* inoutStartIndex,
		UINT* inoutNumHandle,
		UINT* optoutIncrementSize);
public:
	DescriptorAllocator(ID3D12Device* device,
		UINT numRtv,
		UINT numDsv,
		UINT numCbvSrvUav,
		UINT numSampler);

	[[nodiscard]] DescriptorHandle Allocation(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		UINT* inoutNumHandle,
		UINT* optoutIncrementSize = nullptr);

	[[nodiscard]] const DescriptorHeap* GetDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE type)const;

	virtual ~DescriptorAllocator();
};