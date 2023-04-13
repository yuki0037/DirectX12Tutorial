#define CMD_CLOSE_IMMEDIATELY true

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
		IID_PPV_ARGS(&mCmdAllocatorForDrawing)
	);
	HRESULT_ASSERT(hr);

	hr = mDevice->CreateCommandList(
		//ノードマスクは0を指定
		0,
		//作成するコマンドタイプはDIRECT(汎用)を指定
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		//コマンドアロケーターは先ほど作成したものを指定
		mCmdAllocatorForDrawing,
		//初期パイプラインステートは指定しない
		nullptr,
		//作成したコマンドリストの受け取り先
		IID_PPV_ARGS(&mCmdListForDrawing)
	);
	HRESULT_ASSERT(hr);

	hr = mDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&mCmdAllocatorForCopy)
	);
	HRESULT_ASSERT(hr);

	hr = mDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCmdAllocatorForCopy,
		nullptr,
		IID_PPV_ARGS(&mCmdListForCopy)
	);
	HRESULT_ASSERT(hr);

#if CMD_CLOSE_IMMEDIATELY
	hr = mCmdListForDrawing->Close();
	HRESULT_ASSERT(hr);
	hr = mCmdListForCopy->Close();
	HRESULT_ASSERT(hr);
#endif

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

void DirectX12Tutorial::CreatePipelineState2D()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	//ルート署名 シェーダーと互換性があるか判定されます。
	graphicsPipelineStateDesc.pRootSignature = mRootSignature;
	CSO vertexShader;
	//頂点シェーダーの読み込み
	LoadCSO("VertexShader.cso", &vertexShader);
	//頂点シェーダーの設定
	graphicsPipelineStateDesc.VS.pShaderBytecode = vertexShader.data();
	graphicsPipelineStateDesc.VS.BytecodeLength = static_cast<SIZE_T>(vertexShader.size());
	CSO pixelShader;
	//ピクセルシェーダーの読み込み
	LoadCSO("PixelShader.cso", &pixelShader);
	//ピクセルシェーダーの設定
	graphicsPipelineStateDesc.PS.pShaderBytecode = pixelShader.data();
	graphicsPipelineStateDesc.PS.BytecodeLength = static_cast<SIZE_T>(pixelShader.size());
	//その他のシェーダーは使用しない
	graphicsPipelineStateDesc.DS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	graphicsPipelineStateDesc.HS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	graphicsPipelineStateDesc.GS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	//ストリーム出力は使用しないので無視してOK
	graphicsPipelineStateDesc.StreamOutput = D3D12_STREAM_OUTPUT_DESC{};
	//ブレンドの設定
	SetBlendDesc(&graphicsPipelineStateDesc.BlendState);
	//ブレンドのサンプルマスクの設定
	graphicsPipelineStateDesc.SampleMask = UINT_MAX;
	//ラスタライザーの設定
	SetRasterizerDesc(&graphicsPipelineStateDesc.RasterizerState);
	//深度ステンシルの設定
	SetDepthStencilDesc(&graphicsPipelineStateDesc.DepthStencilState);
	//頂点バッファのデータの情報を取得
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[]
	{
		{
			/*
				セマンティック名
				シェーダーに送る際のデータの送り先の名前
				OpenGLのuniform変数送るときの
				glGetUniformLocation(GLuint program,const GLchar *name);
				nameのような役割
			*/
			"POSITION",
			/*
				セマンティックインデックス
				送り先が配列(例:COLOR[n])の場合
				のｎを指定する
			*/
			0,
			/*
				送るデータのフォーマット
				DXGI_FORMAT_R32_FLOAT -> 32bit実数型が1つ(x)
				DXGI_FORMAT_R32G32_FLOAT -> 32bit実数型が2つ(x,y)
				DXGI_FORMAT_R32G32B32_FLOAT -> 32bit実数型が3つ(x,y,z)
				DXGI_FORMAT_R32G32B32A32_FLOAT -> 32bit実数型が4つ(x,y,z,w)
			*/
			DXGI_FORMAT_R32G32_FLOAT,
			/*
				頂点バッファのインデックス
				以下の3つのバッファを送信す際、
				どの頂点バッファからこのデータを取得するかの
				インデックス
				[0]頂点座標のみを送る頂点バッファ
				[1]法線のみを送る頂点バッファ
				[2]頂点色のみを送る頂点バッファ
			*/
			0,
			/*
				頂点データの先頭からこのデータまでの
				オフセット値(byte単位)
			*/
			offsetof(Vertex,mPosition),
			/*
				頂点データの場合 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
				インデックスデータの場合 D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
			*/
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			//頂点バッファの場合0
			0
		},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex,mColor), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	//入力データの設定
	graphicsPipelineStateDesc.InputLayout.pInputElementDescs = inputElementDescs;
	graphicsPipelineStateDesc.InputLayout.NumElements = ARRAYSIZE(inputElementDescs);
	//ストリップの切り取り値
	graphicsPipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//プリミティブトポロジーのタイプ(頂点情報の解釈方法)
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//書き込み先の数
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	//書き込み先のフォーマット
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//深度バッファのフォーマット(使用しない場合DXGI_FORMAT_UNKNOWN)
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	//書き込み先のMSAAの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleDesc.Quality = 0;
	//ノードマスクは0
	graphicsPipelineStateDesc.NodeMask = 0;
	/*
		キャッシュされたパイプラインステートオブジェクト(PSO)データ
		同じ設定のPSOを複数作成するときに、最初に作成したPSOから
		キャッシュすることで作成が高速になる(らしい)
	*/
	graphicsPipelineStateDesc.CachedPSO = D3D12_CACHED_PIPELINE_STATE{ nullptr,0 };
	//フラグなし
	graphicsPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	//パイプラインステートオブジェクトの作成
	HRESULT hr = mDevice->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&mPipelineState2D));
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateVertexBuffer2D()
{
	//ヒープのプロパティをアップロードの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc{};
	//リソースのデータのタイプ(バッファを設定)
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	//アライメントは0
	resourceDesc.Alignment = 0;
	//データサイズ
	resourceDesc.Width = sizeof(Vertex) * 3;
	//「Height」「DepthOrArraySize」「MipLevels」はバッファの場合1固定
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	//バッファの場合はDXGI_FORMAT_UNKNOWN固定
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	//バッファの場合はMSAAは(Count = 1,Quality = 0)固定
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	//バッファの場合以下略
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//フラグなし
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = mDevice->CreateCommittedResource(
		//ヒープのプロパティ
		&heapProperties,
		//ヒープのフラグなし
		D3D12_HEAP_FLAG_NONE,
		//リソースの設定
		&resourceDesc,
		//アップロードヒープの場合 D3D12_RESOURCE_STATE_GENERIC_READ
		D3D12_RESOURCE_STATE_GENERIC_READ,
		//バッファの場合以下略
		nullptr,
		//作成したバッファの保存先
		IID_PPV_ARGS(&mVertexBuffer2D)
	);
	HRESULT_ASSERT(hr);

	//リソースからデータのアドレスを取得
	Vertex* vertices = nullptr;
	hr = mVertexBuffer2D->Map(0, nullptr, reinterpret_cast<void**>(&vertices));
	HRESULT_ASSERT(hr);

	//頂点データの設定

	float offsetX = mViewport.Width / 6.0f;
	float offsetY = mViewport.Height / 6.0f;

	vertices[0].mPosition.x = mViewport.Width * 0.5f;
	vertices[0].mPosition.y = offsetY;
	vertices[0].mColor.x = 1.0f;
	vertices[0].mColor.y = 0.0f;
	vertices[0].mColor.z = 0.0f;
	vertices[0].mColor.w = 1.0f;

	vertices[1].mPosition.x = offsetX;
	vertices[1].mPosition.y = mViewport.Height - offsetY;
	vertices[1].mColor.x = 0.0f;
	vertices[1].mColor.y = 1.0f;
	vertices[1].mColor.z = 0.0f;
	vertices[1].mColor.w = 1.0f;

	vertices[2].mPosition.x = mViewport.Width - offsetX;
	vertices[2].mPosition.y = mViewport.Height - offsetY;
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

	mVertexBuffer2D->Unmap(0, nullptr);
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

	mCmdListForDrawing->ResourceBarrier(
		//設定するリソースバリアの数
		1,
		//設定するリソースバリア
		&resourceBarrier
	);

	//書き込み先の設定
	mCmdListForDrawing->OMSetRenderTargets(
		1,
		&mBackBaffers[mBackBufferIndex].mCpuHandle,
		TRUE,
		nullptr
	);
	//書き込み先のクリア
	mCmdListForDrawing->ClearRenderTargetView(
		mBackBaffers[mBackBufferIndex].mCpuHandle,
		mClearValue.Color,
		0,
		nullptr
	);
	//ビューポートの設定
	mCmdListForDrawing->RSSetViewports(1, &mViewport);
	//シザー矩形の設定
	mCmdListForDrawing->RSSetScissorRects(1, &mScissorRect);
}

void DirectX12Tutorial::DrawPolygon2D()
{
	//パイプラインステートの設定
	mCmdListForDrawing->SetPipelineState(mPipelineState2D);
	//ルート署名の設定
	mCmdListForDrawing->SetGraphicsRootSignature(mRootSignature);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//頂点バッファのアドレス
	vertexBufferView.BufferLocation = mVertexBuffer2D->GetGPUVirtualAddress();
	//頂点バッファの大きさ
	vertexBufferView.SizeInBytes = sizeof(Vertex) * 3;
	//1頂点あたりの大きさ
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	//頂点バッファの設定
	mCmdListForDrawing->IASetVertexBuffers(0, 1, &vertexBufferView);
	//プリミティブの解釈方法
	mCmdListForDrawing->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//ドローコール
	mCmdListForDrawing->DrawInstanced(3, 1, 0, 0);
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

	mCmdListForDrawing->ResourceBarrier(
		//設定するリソースバリアの数
		1,
		//設定するリソースバリア
		&resourceBarrier
	);
}

void DirectX12Tutorial::ExecuteCmdLists()
{
	//コマンドリストを閉じる
	HRESULT hr = mCmdListForCopy->Close();
	HRESULT_ASSERT(hr);
	hr = mCmdListForDrawing->Close();
	HRESULT_ASSERT(hr);

	//実行するコマンドリストをひとまとめにする
	ID3D12CommandList* const cmdLists[]
	{
		mCmdListForCopy,
		mCmdListForDrawing
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
	/*
		(現在積まれている)コマンドキューの最後に
		フェンス内の数値を入力した数値にする
		という命令を送信
	*/
	HRESULT hr = mCmdQueue->Signal(mFence, ++mSignalValue);
	HRESULT_ASSERT(hr);

	//待機する2通りの方法の違いと問題点
#if 0
	//ループする方法
	/*
		フェンスの使用方法次第では
		while (mSignalValue != mFence->GetCompletedValue());
		では機能しない場合がある。
		Signal後必ず待機する場合は問題ないが
		mSignalValue = 0;
		mCmdQueue->Signal(mFence, ++mSignalValue);
		//mSignalValueが1に処理済みの値が0
		~~~何かしらの処理~~~
		mCmdQueue->Signal(mFence, ++mSignalValue);
		//mSignalValueが2に一つ前のシグナルが完了し処理済みの値が1に

		//待機
		while (mSignalValue != mFence->GetCompletedValue());
		//最後のシグナル時の値になっていないがmSignalValueと
		//処理済みの値とは違うので素通りする
	*/
	while (mSignalValue > mFence->GetCompletedValue());
#else
	//イベントを使用する方法(確実性がある)
	HANDLE fenceEvent = CreateEvent(0, 0, 0, 0);
	assert(fenceEvent != nullptr);
	hr = mFence->SetEventOnCompletion(mSignalValue, fenceEvent);
	HRESULT_ASSERT(hr);
	WaitForSingleObject(fenceEvent, INFINITE);
	WIN32_ASSERT(GetLastError());
	CloseHandle(fenceEvent);
#endif
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
	HRESULT hr = mCmdAllocatorForDrawing->Reset();
	HRESULT_ASSERT(hr);
	//コマンドリストにコマンドアロケーターをセット
	hr = mCmdListForDrawing->Reset(mCmdAllocatorForDrawing, nullptr);
	HRESULT_ASSERT(hr);
	hr = mCmdAllocatorForCopy->Reset();
	HRESULT_ASSERT(hr);
	hr = mCmdListForCopy->Reset(mCmdAllocatorForCopy, nullptr);
	HRESULT_ASSERT(hr);
}

DirectX12Tutorial::DirectX12Tutorial(const HWND hWnd)
	:mDevice(nullptr),
	mCmdListForDrawing(nullptr),
	mCmdAllocatorForDrawing(nullptr),
	mCmdListForCopy(nullptr),
	mCmdAllocatorForCopy(nullptr),
	mCmdQueue(nullptr),
	mFence(nullptr),
	mPipelineState2D(nullptr),
	mPipelineState3D(nullptr),
	mRootSignature(nullptr),
	mRtvDescriptorHeap(nullptr),
	mDsvDescriptorHeap(nullptr),
	mCbvAndSrvDescriptorHeap(nullptr),
	mSamplerDescriptorHeap(nullptr),
	mClearValue(),
	mViewport(),
	mScissorRect(),
	mBackBaffers(),
	mDepthBuffer(nullptr),
	mConstantBuffer(nullptr),
	mTexture(nullptr),
	mTextureUploadBuffer(nullptr),
	mVertexBuffer2D(nullptr),
	mVertexBuffer3D(nullptr),
	mIndexBuffer3D(nullptr),
	mSwapChain(nullptr),
	mSignalValue(0),
	mRtvIncrementSize(0),
	mBackBufferIndex(0)
{
	//ComObjectを触るうえでの儀式のようなもの
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
	HRESULT_ASSERT(hr);

#if defined(DEBUG) | defined(_DEBUG)
	//デバックレイヤーの適用(必ずデバイス作成前に行う)
	ApplyDebugLayer();
#endif
	//デバイス作成
	CreateDevice();
	//コマンド関係の作成
	CreateCmdObjects();
	//スワップチェインと書き込み先の作成
	CreateSwapChainAndRenderTargets(hWnd);
	//フェンスの作成
	CreateFence();
	//ルート署名の作成
	CreateRootSignature();
	//パイプラインステートの作成
	CreatePipelineState2D();
	//頂点バッファの作成
	CreateVertexBuffer2D();
}

void DirectX12Tutorial::Render()
{
	/*
		コマンドのリセット
		コマンドリストを作成したあとすぐにクローズすれば
		フレームの最初に実行する
		クローズとリセットが交互に行われるようにする
		作成した直後はリセットした状態である
	*/
#if CMD_CLOSE_IMMEDIATELY
	CmdReset();
#endif
	//描画開始処理
	BeginRendering();
	//ポリゴンの描画
	DrawPolygon2D();
	//描画終了処理
	EndRendering();
	//コマンドリストの実行
	ExecuteCmdLists();
	//描画が完了するまで待機
	WaitForPreviousFrame();
	//ウィンドウに送信
	Present();
#if !CMD_CLOSE_IMMEDIATELY
	CmdReset();
#endif
}

DirectX12Tutorial::~DirectX12Tutorial()
{
	/*
		コマンドキューで処理を行っている最中に
		データが消えるのを防ぐため
		コマンドキューの中身がなくなるまで待機
	*/
	WaitForPreviousFrame();
	//随時開放
	SAFE_RELEASE(mDevice);
	SAFE_RELEASE(mCmdListForDrawing);
	SAFE_RELEASE(mCmdAllocatorForDrawing);
	SAFE_RELEASE(mCmdListForCopy);
	SAFE_RELEASE(mCmdAllocatorForCopy);
	SAFE_RELEASE(mCmdQueue);
	SAFE_RELEASE(mFence);
	SAFE_RELEASE(mPipelineState2D);
	SAFE_RELEASE(mPipelineState3D);
	SAFE_RELEASE(mRootSignature);
	SAFE_RELEASE(mRtvDescriptorHeap);
	SAFE_RELEASE(mDsvDescriptorHeap);
	SAFE_RELEASE(mCbvAndSrvDescriptorHeap);
	SAFE_RELEASE(mSamplerDescriptorHeap);
	for (BackBuffer& backBaffer : mBackBaffers)
	{
		SAFE_RELEASE(backBaffer.mResource);
	}
	SAFE_RELEASE(mDepthBuffer);
	SAFE_RELEASE(mConstantBuffer);
	SAFE_RELEASE(mTexture);
	SAFE_RELEASE(mTextureUploadBuffer);
	SAFE_RELEASE(mVertexBuffer2D);
	SAFE_RELEASE(mVertexBuffer3D);
	SAFE_RELEASE(mIndexBuffer3D);
	SAFE_RELEASE(mSwapChain);
	//ComObjectを触るうえでの儀式のようなもの
	CoUninitialize();
}

void LoadCSO(const char* filePath, CSO* cso)
{
	std::ifstream ifs{ filePath ,std::ios::binary };
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
	blendState->RenderTarget[0].LogicOpEnable = FALSE;
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