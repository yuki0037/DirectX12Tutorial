#include "DirectX12Tutorial.h"
#include "Misc.h"
#include <vector>
#include <fstream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using CSO = std::vector<BYTE>;
void LoadCSO(const char*, CSO*);
void SetBlendDesc(D3D12_BLEND_DESC*);
void SetRasterizerDesc(D3D12_RASTERIZER_DESC*);
void SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC*);

void DirectX12Tutorial::ApplyDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (SUCCEEDED(hr))
	{
		debugLayer->EnableDebugLayer();
	}
	SAFE_RELEASE(debugLayer);
}

void DirectX12Tutorial::CreateDevice()
{
	HRESULT hr = D3D12CreateDevice(
		//アダプターの設定はなし
		nullptr,
		//最小機能レベルはDirectX11相当
		D3D_FEATURE_LEVEL_11_0,
		//作成したデバイスの受け取り先
		IID_PPV_ARGS(&mDevice)
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateCmdObjects()
{
	HRESULT hr = mDevice->CreateCommandAllocator(
		//作成するコマンドタイプはDIRECT(汎用)を指定
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		//作成したコマンドアロケーターの受け取り先
		IID_PPV_ARGS(&mCmdAllocator)
	);
	HRESULT_ASSERT(hr);

	hr = mDevice->CreateCommandList(
		//ノードマスクは0を指定
		0,
		//作成するコマンドタイプはDIRECT(汎用)を指定
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		//コマンドアロケーターは先ほど作成したものを指定
		mCmdAllocator,
		//初期パイプラインステートは指定しない
		nullptr,
		//作成したコマンドリストの受け取り先
		IID_PPV_ARGS(&mCmdList)
	);
	HRESULT_ASSERT(hr);

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	//作成するコマンドタイプはDIRECT(汎用)を指定
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//このコマンドキューは通常の優先度を指定
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//フラグは設定なし
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//ノードマスクは0を指定
	cmdQueueDesc.NodeMask = 0;

	hr = mDevice->CreateCommandQueue(
		//コマンドキューの設定
		&cmdQueueDesc,
		//作成したコマンドキューの受け取り先
		IID_PPV_ARGS(&mCmdQueue)
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateSwapChainAndRenderTargets(const HWND hWnd)
{
	IDXGIFactory2* dxgiFactory = nullptr;
	UINT createFactoryFlags = 0;

#if defined(DEBUG) | defined(_DEBUG)
	//デバック時「DXGIDebug.dll」を読み取るように指定
	createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	HRESULT hr = CreateDXGIFactory2(
		//ファクトリー作成フラグ
		createFactoryFlags,
		//作成したファクトリーの受け取り先
		IID_PPV_ARGS(&dxgiFactory)
	);
	HRESULT_ASSERT(hr);

	//ALT + Enterを無視するように設定
	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	RECT windowRect{};
	if (!GetClientRect(hWnd, &windowRect))
	{
		HRESULT_ASSERT(HRESULT_FROM_WIN32(GetLastError()));
	}
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//書き込み先の大きさはウィンドウの大きさを同じになるように指定
	swapChainDesc.Width = static_cast<UINT>(windowRect.right - windowRect.left);
	swapChainDesc.Height = static_cast<UINT>(windowRect.bottom - windowRect.top);
	//書き込み先は32 ビットの符号なし正規化整数形式を指定
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//立体視は使用しない
	swapChainDesc.Stereo = FALSE;
	//MSAAの設定はなし
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	//バッファの使用用途は書き込み先を指定
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//バックバッファの数
	swapChainDesc.BufferCount = MBackBufferCount;
	//ウィンドウのサイズと書き込み先の大きさが違うとき引き伸ばすように指定
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	//フリップした後、バックバッファの内容を破棄するように指定
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//GPUに透明度の扱い方を任せる
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//フラグの指定なし
	swapChainDesc.Flags = 0;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc{};
	//リフレッシュレートの設定(1/60)
	swapChainFullscreenDesc.RefreshRate.Numerator = 1;
	swapChainFullscreenDesc.RefreshRate.Denominator = 60;
	//走査線の順序指定なし
	swapChainFullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//引き伸ばしあり
	swapChainFullscreenDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	//ウィンドウモードを指定
	swapChainFullscreenDesc.Windowed = TRUE;


	IDXGISwapChain1* tempSwapChain = nullptr;
	hr = dxgiFactory->CreateSwapChainForHwnd(
		//デバイスって書いてるけどコマンドキュー(DIRECTだけOK)
		mCmdQueue,
		//スワップチェインと関連付けするウィンドウハンドル
		hWnd,
		//スワップチェインの設定
		&swapChainDesc,
		//フルスクリーンの設定
		&swapChainFullscreenDesc,
		//出力先のモニターを指定しないのでnullptrを入れておく
		nullptr,
		//作成したスワップチェインの受け取り先
		&tempSwapChain
	);
	HRESULT_ASSERT(hr);

	/*
		先ほど作成したスワップチェインでは
		使用したいバージョンと違うので、
		作成したスワップチェインから使用したい、
		バージョンのスワップチェインを取り出す
	*/
	hr = tempSwapChain->QueryInterface(&mSwapChain);
	HRESULT_ASSERT(hr);

	//クリア値を設定
	mClearValue.Format = swapChainDesc.Format;
	mClearValue.Color[0] = 0.0f;
	mClearValue.Color[1] = 0.0f;
	mClearValue.Color[2] = 0.0f;
	mClearValue.Color[3] = 0.0f;

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	//ヒープ記述子はRTVを指定
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	//バックバッファの数用意する
	descriptorHeapDesc.NumDescriptors = swapChainDesc.BufferCount;
	//フラグ指定なし
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//ノードマスクは0を指定
	descriptorHeapDesc.NodeMask = 0;

	hr = mDevice->CreateDescriptorHeap(
		//ヒープ記述子の設定
		&descriptorHeapDesc,
		//作成したヒープ記述子の受け取り先
		IID_PPV_ARGS(&mRtvDescriptorHeap)
	);
	HRESULT_ASSERT(hr);

	//レンダーターゲットの先頭Cpuハンドルを取得
	auto rtvCpuHandle = mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//レンダーターゲットハンドルのインクリメントサイズを取得
	mRtvIncrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//バックバッファ全てでレンダーターゲットを作成
	for (UINT index = 0; index < swapChainDesc.BufferCount; ++index)
	{
		//スワップチェインからバックバッファを取得
		ID3D12Resource* backBuffer = nullptr;
		hr = mSwapChain->GetBuffer(index, IID_PPV_ARGS(&backBuffer));
		HRESULT_ASSERT(hr);

		//レンダーターゲットを作成
		mDevice->CreateRenderTargetView(
			//レンダーターゲットのもとになるバックバッファ
			backBuffer,
			//backBufferからレンダーターゲットの設定を吸い出してもらう
			nullptr,
			//レンダーターゲットの作成先のハンドル
			rtvCpuHandle
		);

		//バックバッファデータを保存しておく
		mBackBaffers[index].mResource = backBuffer;
		mBackBaffers[index].mCpuHandle = rtvCpuHandle;

		//ハンドルをインクリメントする
		rtvCpuHandle.ptr += static_cast<SIZE_T>(mRtvIncrementSize);

	}

	//ビューポートの設定
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<FLOAT>(swapChainDesc.Width);
	mViewport.Height = static_cast<FLOAT>(swapChainDesc.Height);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	//シザー矩形の設定(この矩形外のものをカリングするために使用)
	mScissorRect.left = 0;
	mScissorRect.top = 0;
	mScissorRect.right = static_cast<LONG>(swapChainDesc.Width);
	mScissorRect.bottom = static_cast<LONG>(swapChainDesc.Height);

	SAFE_RELEASE(tempSwapChain);
	SAFE_RELEASE(dxgiFactory);
}

void DirectX12Tutorial::CreateFence()
{
	HRESULT hr = mDevice->CreateFence(
		//フェンスの初期値
		mSignalValue,
		//フェンスのフラグは特になし
		D3D12_FENCE_FLAG_NONE,
		//作成したフェンスの受け取り先
		IID_PPV_ARGS(&mFence)
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureDataRootSignature{};
	featureDataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	//ルート署名のバージョンを確認
	HRESULT hr = mDevice->CheckFeatureSupport(
		D3D12_FEATURE_ROOT_SIGNATURE,
		&featureDataRootSignature,
		sizeof(featureDataRootSignature));
	//バージョン1_1に対応していない場合1_0に変更
	if (FAILED(hr))
	{
		featureDataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc{};
	//作成するルート署名のバージョンを設定
	versionedRootSignatureDesc.Version =
		featureDataRootSignature.HighestVersion;

	//バージョンに対応したパラメータを設定していく
	switch (versionedRootSignatureDesc.Version)
	{
	case D3D_ROOT_SIGNATURE_VERSION_1_0:
		versionedRootSignatureDesc.Desc_1_0.NumParameters = 0;
		versionedRootSignatureDesc.Desc_1_0.pParameters = nullptr;
		versionedRootSignatureDesc.Desc_1_0.NumStaticSamplers = 0;
		versionedRootSignatureDesc.Desc_1_0.pStaticSamplers = nullptr;
		versionedRootSignatureDesc.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		break;
	case D3D_ROOT_SIGNATURE_VERSION_1_1:
		versionedRootSignatureDesc.Desc_1_1.NumParameters = 0;
		versionedRootSignatureDesc.Desc_1_1.pParameters = nullptr;
		versionedRootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
		versionedRootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
		versionedRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		break;
	}

	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	//ルート署名をシリアライズする
	hr = D3D12SerializeVersionedRootSignature(
		&versionedRootSignatureDesc,
		&rootSignatureBlob,
		&errorBlob
	);
	_ASSERT_EXPR_A(SUCCEEDED(hr), static_cast<LPCSTR>(errorBlob->GetBufferPointer()));

	//シリアライズされたデータからルート署名を作成
	hr = mDevice->CreateRootSignature(
		0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)
	);
	HRESULT_ASSERT(hr);

	SAFE_RELEASE(rootSignatureBlob);
	SAFE_RELEASE(errorBlob);
}

void DirectX12Tutorial::CreatePipelineState()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = mRootSignature;
	CSO vertexShader;
	LoadCSO("VertexShader.cso", &vertexShader);
	graphicsPipelineStateDesc.VS.pShaderBytecode = vertexShader.data();
	graphicsPipelineStateDesc.VS.BytecodeLength = static_cast<SIZE_T>(vertexShader.size());
	CSO pixelShader;
	LoadCSO("PixelShader.cso", &pixelShader);
	graphicsPipelineStateDesc.PS.pShaderBytecode = pixelShader.data();
	graphicsPipelineStateDesc.PS.BytecodeLength = static_cast<SIZE_T>(pixelShader.size());

	graphicsPipelineStateDesc.DS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	graphicsPipelineStateDesc.HS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	graphicsPipelineStateDesc.GS = D3D12_SHADER_BYTECODE{ nullptr,0 };

	graphicsPipelineStateDesc.StreamOutput = D3D12_STREAM_OUTPUT_DESC{};

	SetBlendDesc(&graphicsPipelineStateDesc.BlendState);
	graphicsPipelineStateDesc.SampleMask = UINT_MAX;
	SetRasterizerDesc(&graphicsPipelineStateDesc.RasterizerState);
	SetDepthStencilDesc(&graphicsPipelineStateDesc.DepthStencilState);
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[]
	{
		{ "POSITION",	0,	 DXGI_FORMAT_R32G32B32_FLOAT,		0,	D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	0 },
		{ "COLOR",		0,	 DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	D3D12_APPEND_ALIGNED_ELEMENT,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	0 },
	};
	graphicsPipelineStateDesc.InputLayout.pInputElementDescs = inputElementDescs;
	graphicsPipelineStateDesc.InputLayout.NumElements = ARRAYSIZE(inputElementDescs);

	graphicsPipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleDesc.Quality = 0;
	graphicsPipelineStateDesc.NodeMask = 0;
	graphicsPipelineStateDesc.CachedPSO = D3D12_CACHED_PIPELINE_STATE{ nullptr,0 };
	graphicsPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hr = mDevice->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&mPipelineState)
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateVertexBuffer()
{
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = sizeof(Vertex) * 3;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = mDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mVertexBuffer)
	);
	HRESULT_ASSERT(hr);

	Vertex* vertices = nullptr;
	hr = mVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertices));
	HRESULT_ASSERT(hr);

	vertices[0].mPosition.x = mViewport.Width * 0.5f;
	vertices[0].mPosition.y = 0.0f;
	vertices[0].mPosition.z = 0.0f;
	vertices[0].mColor.x = 1.0f;
	vertices[0].mColor.y = 0.0f;
	vertices[0].mColor.z = 0.0f;
	vertices[0].mColor.w = 1.0f;

	vertices[1].mPosition.x = 0.0f;
	vertices[1].mPosition.y = mViewport.Height;
	vertices[1].mPosition.z = 0.0f;
	vertices[1].mColor.x = 0.0f;
	vertices[1].mColor.y = 1.0f;
	vertices[1].mColor.z = 0.0f;
	vertices[1].mColor.w = 1.0f;

	vertices[2].mPosition.x = mViewport.Width;
	vertices[2].mPosition.y = mViewport.Height;
	vertices[2].mPosition.z = 0.0f;
	vertices[2].mColor.x = 0.0f;
	vertices[2].mColor.y = 0.0f;
	vertices[2].mColor.z = 1.0f;
	vertices[2].mColor.w = 1.0f;

	//NDC空間へ座標変換
	for (UINT i = 0; i < 3; i++)
	{
		vertices[i].mPosition.x = vertices[i].mPosition.x / mViewport.Width * 2.0f - 1.0f;
		vertices[i].mPosition.y = 1.0f - 2.0f * vertices[i].mPosition.y / mViewport.Height;
	}

	mVertexBuffer->Unmap(0, nullptr);
}

void DirectX12Tutorial::BeginRendering()
{
	mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER resourceBarrier{};
	//リソースバリアのタイプを遷移に設定
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//フラグなし
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//操作するリソースのアドレス
	resourceBarrier.Transition.pResource = mBackBaffers[mBackBufferIndex].mResource;
	//操作するサブリソース(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCESですべて指定)
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//リソースバリアの遷移前の状態
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//リソースバリアの遷移後の状態
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	mCmdList->ResourceBarrier(
		//設定するリソースバリアの数
		1,
		//設定するリソースバリア
		&resourceBarrier
	);

	mCmdList->OMSetRenderTargets(
		1,
		&mBackBaffers[mBackBufferIndex].mCpuHandle,
		TRUE,
		nullptr
	);
	mCmdList->ClearRenderTargetView(
		mBackBaffers[mBackBufferIndex].mCpuHandle,
		mClearValue.Color,
		0,
		nullptr
	);
	mCmdList->RSSetViewports(1, &mViewport);
	mCmdList->RSSetScissorRects(1, &mScissorRect);
}

void DirectX12Tutorial::DrawPolygon()
{
	mCmdList->SetPipelineState(mPipelineState);
	mCmdList->SetGraphicsRootSignature(mRootSignature);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(Vertex) * 3;
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	mCmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCmdList->DrawInstanced(3, 1, 0, 0);
}

void DirectX12Tutorial::EndRendering()
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	//リソースバリアのタイプを遷移に設定
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//フラグなし
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//操作するリソースのアドレス
	resourceBarrier.Transition.pResource = mBackBaffers[mBackBufferIndex].mResource;
	//操作するサブリソース(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCESですべて指定)
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//リソースバリアの遷移前の状態
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//リソースバリアの遷移後の状態
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	mCmdList->ResourceBarrier(
		//設定するリソースバリアの数
		1,
		//設定するリソースバリア
		&resourceBarrier
	);

	//コマンドリストを閉じる
	HRESULT hr = mCmdList->Close();
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::ExecuteCmdLists()
{
	//実行するコマンドリストをひとまとめにする
	ID3D12CommandList* const cmdLists[]
	{
		mCmdList
	};

	//実行
	mCmdQueue->ExecuteCommandLists(
		//実行するコマンドリストの数
		ARRAYSIZE(cmdLists),
		//実行するコマンドリストのアドレス
		cmdLists
	);
}

void DirectX12Tutorial::WaitForPreviousFrame()
{
	HRESULT hr = mCmdQueue->Signal(mFence, ++mSignalValue);
	HANDLE fenceEvent = CreateEvent(0, 0, 0, 0);
	assert(fenceEvent != nullptr);
	hr = mFence->SetEventOnCompletion(mSignalValue, fenceEvent);
	HRESULT_ASSERT(hr);
	WaitForSingleObject(fenceEvent, INFINITE);
	WIN32_ASSERT(GetLastError());
	CloseHandle(fenceEvent);
}

void DirectX12Tutorial::Present()
{
	HRESULT hr = mSwapChain->Present(
		//垂直同期あり
		1,
		//フラグの指定なし
		0
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CmdReset()
{
	//コマンドアロケーターをリセット
	HRESULT hr = mCmdAllocator->Reset();
	HRESULT_ASSERT(hr);
	//コマンドリストにコマンドアロケーターをセット
	hr = mCmdList->Reset(mCmdAllocator, nullptr);
	HRESULT_ASSERT(hr);
}

DirectX12Tutorial::DirectX12Tutorial(const HWND hWnd)
	:mDevice(nullptr),
	mCmdList(nullptr),
	mCmdAllocator(nullptr),
	mCmdQueue(nullptr),
	mFence(nullptr),
	mPipelineState(nullptr),
	mRootSignature(nullptr),
	mRtvDescriptorHeap(nullptr),
	mClearValue(),
	mViewport(),
	mScissorRect(),
	mBackBaffers(),
	mVertexBuffer(nullptr),
	mSwapChain(nullptr),
	mSignalValue(0),
	mRtvIncrementSize(0),
	mBackBufferIndex(0)
{
	//ComObjectを触るうえでの儀式のようなもの
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
	HRESULT_ASSERT(hr);

#if defined(DEBUG) | defined(_DEBUG)
	ApplyDebugLayer();
#endif
	CreateDevice();
	CreateCmdObjects();
	CreateSwapChainAndRenderTargets(hWnd);
	CreateFence();
	CreateRootSignature();
	CreatePipelineState();
	CreateVertexBuffer();
}

void DirectX12Tutorial::Render()
{
	BeginRendering();
	DrawPolygon();
	EndRendering();
	ExecuteCmdLists();
	WaitForPreviousFrame();
	Present();
	CmdReset();
}

DirectX12Tutorial::~DirectX12Tutorial()
{
	SAFE_RELEASE(mDevice);
	SAFE_RELEASE(mCmdList);
	SAFE_RELEASE(mCmdAllocator);
	SAFE_RELEASE(mCmdQueue);
	SAFE_RELEASE(mFence);
	SAFE_RELEASE(mPipelineState);
	SAFE_RELEASE(mRootSignature);
	SAFE_RELEASE(mRtvDescriptorHeap);
	for (BackBuffer& backBaffer : mBackBaffers)
	{
		SAFE_RELEASE(backBaffer.mResource);
	}
	SAFE_RELEASE(mVertexBuffer);
	SAFE_RELEASE(mSwapChain);
	//ComObjectを触るうえでの儀式のようなもの
	CoUninitialize();
}

//

void LoadCSO(const char* filePass, CSO* cso)
{
	std::ifstream ifs{ filePass ,std::ios::binary };
	assert(ifs.is_open() && "file not found!!");

	//ファイルサイズを取得
	ifs.seekg(0, std::ios_base::end);
	auto fileSize = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);

	cso->resize(static_cast<size_t>(fileSize));

	//ファイルの読み込み
	ifs.read(
		reinterpret_cast<char*>(cso->data()),
		fileSize
	);
}

void SetBlendDesc(D3D12_BLEND_DESC* blendState)
{
	blendState->AlphaToCoverageEnable = TRUE;
	blendState->IndependentBlendEnable = FALSE;
	blendState->RenderTarget[0].BlendEnable = FALSE;
	blendState->RenderTarget[0].LogicOpEnable; FALSE;
	blendState->RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendState->RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendState->RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendState->RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendState->RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendState->RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendState->RenderTarget[0].LogicOp = D3D12_LOGIC_OP_CLEAR;
	blendState->RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void SetRasterizerDesc(D3D12_RASTERIZER_DESC* rasterizerState)
{
	rasterizerState->FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerState->CullMode = D3D12_CULL_MODE_NONE;
	rasterizerState->FrontCounterClockwise = TRUE;
	rasterizerState->DepthBias = 0;
	rasterizerState->DepthBiasClamp = 0;
	rasterizerState->SlopeScaledDepthBias = 0;
	rasterizerState->DepthClipEnable = FALSE;
	rasterizerState->MultisampleEnable = FALSE;
	rasterizerState->AntialiasedLineEnable = FALSE;
	rasterizerState->ForcedSampleCount = 0;
	rasterizerState->ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

void SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC* depthStencilState)
{
	depthStencilState->DepthEnable = FALSE;
	depthStencilState->DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilState->DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilState->StencilEnable = FALSE;
	depthStencilState->StencilReadMask = 0;
	depthStencilState->StencilWriteMask = 0;
	depthStencilState->FrontFace = {};
	depthStencilState->BackFace = {};
}