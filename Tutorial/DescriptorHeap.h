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
	UINT mIncrementSize;
public:
	DescriptorHeap(ID3D12Device* device, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
	DescriptorHandle GetHandle(UINT index)const;
	~DescriptorHeap();
};

class DescriptorAllocator
{
private:
	using DescriptorHeaps = std::map<D3D12_DESCRIPTOR_HEAP_TYPE, DescriptorHeap*>;
private:
	DescriptorHeaps mDescriptorHeaps;
public:
	DescriptorAllocator();
};