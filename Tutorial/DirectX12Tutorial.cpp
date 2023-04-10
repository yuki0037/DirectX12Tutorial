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
		//�A�_�v�^�[�̐ݒ�͂Ȃ�
		nullptr,
		//�ŏ��@�\���x����DirectX11����
		D3D_FEATURE_LEVEL_11_0,
		//�쐬�����f�o�C�X�̎󂯎���
		IID_PPV_ARGS(&mDevice)
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateCmdObjects()
{
	HRESULT hr = mDevice->CreateCommandAllocator(
		//�쐬����R�}���h�^�C�v��DIRECT(�ėp)���w��
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		//�쐬�����R�}���h�A���P�[�^�[�̎󂯎���
		IID_PPV_ARGS(&mCmdAllocator)
	);
	HRESULT_ASSERT(hr);

	hr = mDevice->CreateCommandList(
		//�m�[�h�}�X�N��0���w��
		0,
		//�쐬����R�}���h�^�C�v��DIRECT(�ėp)���w��
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		//�R�}���h�A���P�[�^�[�͐�قǍ쐬�������̂��w��
		mCmdAllocator,
		//�����p�C�v���C���X�e�[�g�͎w�肵�Ȃ�
		nullptr,
		//�쐬�����R�}���h���X�g�̎󂯎���
		IID_PPV_ARGS(&mCmdList)
	);
	HRESULT_ASSERT(hr);
#if CMD_CLOSE_IMMEDIATELY
	hr = mCmdList->Close();
	HRESULT_ASSERT(hr);
#endif

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	//�쐬����R�}���h�^�C�v��DIRECT(�ėp)���w��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//���̃R�}���h�L���[�͒ʏ�̗D��x���w��
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//�t���O�͐ݒ�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//�m�[�h�}�X�N��0���w��
	cmdQueueDesc.NodeMask = 0;

	hr = mDevice->CreateCommandQueue(
		//�R�}���h�L���[�̐ݒ�
		&cmdQueueDesc,
		//�쐬�����R�}���h�L���[�̎󂯎���
		IID_PPV_ARGS(&mCmdQueue)
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateSwapChainAndRenderTargets(const HWND hWnd)
{
	IDXGIFactory2* dxgiFactory = nullptr;
	UINT createFactoryFlags = 0;

#if defined(DEBUG) | defined(_DEBUG)
	//�f�o�b�N���uDXGIDebug.dll�v��ǂݎ��悤�Ɏw��
	createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	HRESULT hr = CreateDXGIFactory2(
		//�t�@�N�g���[�쐬�t���O
		createFactoryFlags,
		//�쐬�����t�@�N�g���[�̎󂯎���
		IID_PPV_ARGS(&dxgiFactory)
	);
	HRESULT_ASSERT(hr);

	//ALT + Enter�𖳎�����悤�ɐݒ�
	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	RECT windowRect{};
	if (!GetClientRect(hWnd, &windowRect))
	{
		HRESULT_ASSERT(HRESULT_FROM_WIN32(GetLastError()));
	}
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//�������ݐ�̑傫���̓E�B���h�E�̑傫���𓯂��ɂȂ�悤�Ɏw��
	swapChainDesc.Width = static_cast<UINT>(windowRect.right - windowRect.left);
	swapChainDesc.Height = static_cast<UINT>(windowRect.bottom - windowRect.top);
	//�������ݐ��32 �r�b�g�̕����Ȃ����K�������`�����w��
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//���̎��͎g�p���Ȃ�
	swapChainDesc.Stereo = FALSE;
	//MSAA�̐ݒ�͂Ȃ�
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	//�o�b�t�@�̎g�p�p�r�͏������ݐ���w��
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//�o�b�N�o�b�t�@�̐�
	swapChainDesc.BufferCount = MBackBufferCount;
	//�E�B���h�E�̃T�C�Y�Ə������ݐ�̑傫�����Ⴄ�Ƃ������L�΂��悤�Ɏw��
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	//�t���b�v������A�o�b�N�o�b�t�@�̓��e��j������悤�Ɏw��
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//GPU�ɓ����x�̈�������C����
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//�t���O�̎w��Ȃ�
	swapChainDesc.Flags = 0;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc{};
	//���t���b�V�����[�g�̐ݒ�(1/60)
	swapChainFullscreenDesc.RefreshRate.Numerator = 1;
	swapChainFullscreenDesc.RefreshRate.Denominator = 60;
	//�������̏����w��Ȃ�
	swapChainFullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//�����L�΂�����
	swapChainFullscreenDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	//�E�B���h�E���[�h���w��
	swapChainFullscreenDesc.Windowed = TRUE;


	IDXGISwapChain1* tempSwapChain = nullptr;
	hr = dxgiFactory->CreateSwapChainForHwnd(
		//�f�o�C�X���ď����Ă邯�ǃR�}���h�L���[(DIRECT����OK)
		mCmdQueue,
		//�X���b�v�`�F�C���Ɗ֘A�t������E�B���h�E�n���h��
		hWnd,
		//�X���b�v�`�F�C���̐ݒ�
		&swapChainDesc,
		//�t���X�N���[���̐ݒ�
		&swapChainFullscreenDesc,
		//�o�͐�̃��j�^�[���w�肵�Ȃ��̂�nullptr�����Ă���
		nullptr,
		//�쐬�����X���b�v�`�F�C���̎󂯎���
		&tempSwapChain
	);
	HRESULT_ASSERT(hr);

	/*
		��قǍ쐬�����X���b�v�`�F�C���ł�
		�g�p�������o�[�W�����ƈႤ�̂ŁA
		�쐬�����X���b�v�`�F�C������g�p�������A
		�o�[�W�����̃X���b�v�`�F�C�������o��
	*/
	hr = tempSwapChain->QueryInterface(&mSwapChain);
	HRESULT_ASSERT(hr);

	//�N���A�l��ݒ�
	mClearValue.Format = swapChainDesc.Format;
	mClearValue.Color[0] = 0.0f;
	mClearValue.Color[1] = 0.0f;
	mClearValue.Color[2] = 0.0f;
	mClearValue.Color[3] = 0.0f;

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	//�q�[�v�L�q�q��RTV���w��
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	//�o�b�N�o�b�t�@�̐��p�ӂ���
	descriptorHeapDesc.NumDescriptors = swapChainDesc.BufferCount;
	//�t���O�w��Ȃ�
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	//�m�[�h�}�X�N��0���w��
	descriptorHeapDesc.NodeMask = 0;

	hr = mDevice->CreateDescriptorHeap(
		//�q�[�v�L�q�q�̐ݒ�
		&descriptorHeapDesc,
		//�쐬�����q�[�v�L�q�q�̎󂯎���
		IID_PPV_ARGS(&mRtvDescriptorHeap)
	);
	HRESULT_ASSERT(hr);

	//�����_�[�^�[�Q�b�g�̐擪Cpu�n���h�����擾
	auto rtvCpuHandle = mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//�����_�[�^�[�Q�b�g�n���h���̃C���N�������g�T�C�Y���擾
	mRtvIncrementSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�o�b�N�o�b�t�@�S�ĂŃ����_�[�^�[�Q�b�g���쐬
	for (UINT index = 0; index < swapChainDesc.BufferCount; ++index)
	{
		//�X���b�v�`�F�C������o�b�N�o�b�t�@���擾
		ID3D12Resource* backBuffer = nullptr;
		hr = mSwapChain->GetBuffer(index, IID_PPV_ARGS(&backBuffer));
		HRESULT_ASSERT(hr);

		//�����_�[�^�[�Q�b�g���쐬
		mDevice->CreateRenderTargetView(
			//�����_�[�^�[�Q�b�g�̂��ƂɂȂ�o�b�N�o�b�t�@
			backBuffer,
			//backBuffer���烌���_�[�^�[�Q�b�g�̐ݒ���z���o���Ă��炤
			nullptr,
			//�����_�[�^�[�Q�b�g�̍쐬��̃n���h��
			rtvCpuHandle
		);

		//�o�b�N�o�b�t�@�f�[�^��ۑ����Ă���
		mBackBaffers[index].mResource = backBuffer;
		mBackBaffers[index].mCpuHandle = rtvCpuHandle;

		//�n���h�����C���N�������g����
		rtvCpuHandle.ptr += static_cast<SIZE_T>(mRtvIncrementSize);
	}

	//�r���[�|�[�g�̐ݒ�
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<FLOAT>(swapChainDesc.Width);
	mViewport.Height = static_cast<FLOAT>(swapChainDesc.Height);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	//�V�U�[��`�̐ݒ�(���̋�`�O�̂��̂��J�����O���邽�߂Ɏg�p)
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
		//�t�F���X�̏����l
		mSignalValue,
		//�t�F���X�̃t���O�͓��ɂȂ�
		D3D12_FENCE_FLAG_NONE,
		//�쐬�����t�F���X�̎󂯎���
		IID_PPV_ARGS(&mFence)
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureDataRootSignature{};
	featureDataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	//���[�g�����̃o�[�W�������m�F
	HRESULT hr = mDevice->CheckFeatureSupport(
		D3D12_FEATURE_ROOT_SIGNATURE,
		&featureDataRootSignature,
		sizeof(featureDataRootSignature));
	//�o�[�W����1_1�ɑΉ����Ă��Ȃ��ꍇ1_0�ɕύX
	if (FAILED(hr))
	{
		featureDataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc{};
	//�쐬���郋�[�g�����̃o�[�W������ݒ�
	versionedRootSignatureDesc.Version =
		featureDataRootSignature.HighestVersion;

	//�o�[�W�����ɑΉ������p�����[�^��ݒ肵�Ă���
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
	//���[�g�������V���A���C�Y����
	hr = D3D12SerializeVersionedRootSignature(
		&versionedRootSignatureDesc,
		&rootSignatureBlob,
		&errorBlob
	);
	_ASSERT_EXPR_A(SUCCEEDED(hr), static_cast<LPCSTR>(errorBlob->GetBufferPointer()));

	//�V���A���C�Y���ꂽ�f�[�^���烋�[�g�������쐬
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
	//���[�g���� �V�F�[�_�[�ƌ݊��������邩���肳��܂��B
	graphicsPipelineStateDesc.pRootSignature = mRootSignature;
	CSO vertexShader;
	//���_�V�F�[�_�[�̓ǂݍ���
	LoadCSO("VertexShader.cso", &vertexShader);
	//���_�V�F�[�_�[�̐ݒ�
	graphicsPipelineStateDesc.VS.pShaderBytecode = vertexShader.data();
	graphicsPipelineStateDesc.VS.BytecodeLength = static_cast<SIZE_T>(vertexShader.size());
	CSO pixelShader;
	//�s�N�Z���V�F�[�_�[�̓ǂݍ���
	LoadCSO("PixelShader.cso", &pixelShader);
	//�s�N�Z���V�F�[�_�[�̐ݒ�
	graphicsPipelineStateDesc.PS.pShaderBytecode = pixelShader.data();
	graphicsPipelineStateDesc.PS.BytecodeLength = static_cast<SIZE_T>(pixelShader.size());
	//���̑��̃V�F�[�_�[�͎g�p���Ȃ�
	graphicsPipelineStateDesc.DS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	graphicsPipelineStateDesc.HS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	graphicsPipelineStateDesc.GS = D3D12_SHADER_BYTECODE{ nullptr,0 };
	//�X�g���[���o�͎͂g�p���Ȃ��̂Ŗ�������OK
	graphicsPipelineStateDesc.StreamOutput = D3D12_STREAM_OUTPUT_DESC{};
	//�u�����h�̐ݒ�
	SetBlendDesc(&graphicsPipelineStateDesc.BlendState);
	//�u�����h�̃T���v���}�X�N�̐ݒ�
	graphicsPipelineStateDesc.SampleMask = UINT_MAX;
	//���X�^���C�U�[�̐ݒ�
	SetRasterizerDesc(&graphicsPipelineStateDesc.RasterizerState);
	//�[�x�X�e���V���̐ݒ�
	SetDepthStencilDesc(&graphicsPipelineStateDesc.DepthStencilState);
	//���_�o�b�t�@�̃f�[�^�̏����擾
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[]
	{
		{
			/*
				�Z�}���e�B�b�N��
				�V�F�[�_�[�ɑ���ۂ̃f�[�^�̑����̖��O
				OpenGL��uniform�ϐ�����Ƃ���
				glGetUniformLocation(GLuint program,const GLchar *name);
				name�̂悤�Ȗ���
			*/
			"POSITION",
			/*
				�Z�}���e�B�b�N�C���f�b�N�X
				����悪�z��(��:COLOR[n])�̏ꍇ
				�̂����w�肷��
			*/
			0,
			/*
				����f�[�^�̃t�H�[�}�b�g
				DXGI_FORMAT_R32_FLOAT -> 32bit�����^��1��(x)
				DXGI_FORMAT_R32G32_FLOAT -> 32bit�����^��2��(x,y)
				DXGI_FORMAT_R32G32B32_FLOAT -> 32bit�����^��3��(x,y,z)
				DXGI_FORMAT_R32G32B32A32_FLOAT -> 32bit�����^��4��(x,y,z,w)
			*/
			DXGI_FORMAT_R32G32_FLOAT,
			/*
				���_�o�b�t�@�̃C���f�b�N�X
				�ȉ���3�̃o�b�t�@�𑗐M���ہA
				�ǂ̒��_�o�b�t�@���炱�̃f�[�^���擾���邩��
				�C���f�b�N�X
				[0]���_���W�݂̂𑗂钸�_�o�b�t�@
				[1]�@���݂̂𑗂钸�_�o�b�t�@
				[2]���_�F�݂̂𑗂钸�_�o�b�t�@
			*/
			0,
			/*
				���_�f�[�^�̐擪���炱�̃f�[�^�܂ł�
				�I�t�Z�b�g�l(byte�P��)
			*/
			offsetof(Vertex,mPosition),
			/*
				���_�f�[�^�̏ꍇ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
				�C���f�b�N�X�f�[�^�̏ꍇ D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
			*/
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			//���_�o�b�t�@�̏ꍇ0
			0
		},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex,mColor), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	//���̓f�[�^�̐ݒ�
	graphicsPipelineStateDesc.InputLayout.pInputElementDescs = inputElementDescs;
	graphicsPipelineStateDesc.InputLayout.NumElements = ARRAYSIZE(inputElementDescs);
	//�X�g���b�v�̐؂���l
	graphicsPipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//�v���~�e�B�u�g�|���W�[�̃^�C�v(���_���̉��ߕ��@)
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//�������ݐ�̐�
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	//�������ݐ�̃t�H�[�}�b�g
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//�[�x�o�b�t�@�̃t�H�[�}�b�g(�g�p���Ȃ��ꍇDXGI_FORMAT_UNKNOWN)
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	//�������ݐ��MSAA�̐ݒ�
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleDesc.Quality = 0;
	//�m�[�h�}�X�N��0
	graphicsPipelineStateDesc.NodeMask = 0;
	/*
		�L���b�V�����ꂽ�p�C�v���C���X�e�[�g�I�u�W�F�N�g(PSO)�f�[�^
		�����ݒ��PSO�𕡐��쐬����Ƃ��ɁA�ŏ��ɍ쐬����PSO����
		�L���b�V�����邱�Ƃō쐬�������ɂȂ�(�炵��)
	*/
	graphicsPipelineStateDesc.CachedPSO = D3D12_CACHED_PIPELINE_STATE{ nullptr,0 };
	//�t���O�Ȃ�
	graphicsPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	//�p�C�v���C���X�e�[�g�I�u�W�F�N�g�̍쐬
	HRESULT hr = mDevice->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&mPipelineState));
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CreateVertexBuffer()
{
	//�q�[�v�̃v���p�e�B���A�b�v���[�h�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc{};
	//���\�[�X�̃f�[�^�̃^�C�v(�o�b�t�@��ݒ�)
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	//�A���C�����g��0
	resourceDesc.Alignment = 0;
	//�f�[�^�T�C�Y
	resourceDesc.Width = sizeof(Vertex) * 3;
	//�uHeight�v�uDepthOrArraySize�v�uMipLevels�v�̓o�b�t�@�̏ꍇ1�Œ�
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	//�o�b�t�@�̏ꍇ��DXGI_FORMAT_UNKNOWN�Œ�
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	//�o�b�t�@�̏ꍇ��MSAA��(Count = 1,Quality = 0)�Œ�
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	//�o�b�t�@�̏ꍇ�ȉ���
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//�t���O�Ȃ�
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = mDevice->CreateCommittedResource(
		//�q�[�v�̃v���p�e�B
		&heapProperties,
		//�q�[�v�̃t���O�Ȃ�
		D3D12_HEAP_FLAG_NONE,
		//���\�[�X�̐ݒ�
		&resourceDesc,
		//�A�b�v���[�h�q�[�v�̏ꍇ D3D12_RESOURCE_STATE_GENERIC_READ
		D3D12_RESOURCE_STATE_GENERIC_READ,
		//�o�b�t�@�̏ꍇ�ȉ���
		nullptr,
		//�쐬�����o�b�t�@�̕ۑ���
		IID_PPV_ARGS(&mVertexBuffer)
	);
	HRESULT_ASSERT(hr);

	//���\�[�X����f�[�^�̃A�h���X���擾
	Vertex* vertices = nullptr;
	hr = mVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertices));
	HRESULT_ASSERT(hr);

	//���_�f�[�^�̐ݒ�

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

	//NDC��Ԃ֍��W�ϊ�
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
	//���\�[�X�o���A�̃^�C�v��J�ڂɐݒ�
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//�t���O�Ȃ�
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//���삷�郊�\�[�X�̃A�h���X
	resourceBarrier.Transition.pResource = mBackBaffers[mBackBufferIndex].mResource;
	//���삷��T�u���\�[�X(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES�ł��ׂĎw��)
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//���\�[�X�o���A�̑J�ڑO�̏��
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//���\�[�X�o���A�̑J�ڌ�̏��
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	mCmdList->ResourceBarrier(
		//�ݒ肷�郊�\�[�X�o���A�̐�
		1,
		//�ݒ肷�郊�\�[�X�o���A
		&resourceBarrier
	);

	//�������ݐ�̐ݒ�
	mCmdList->OMSetRenderTargets(
		1,
		&mBackBaffers[mBackBufferIndex].mCpuHandle,
		TRUE,
		nullptr
	);
	//�������ݐ�̃N���A
	mCmdList->ClearRenderTargetView(
		mBackBaffers[mBackBufferIndex].mCpuHandle,
		mClearValue.Color,
		0,
		nullptr
	);
	//�r���[�|�[�g�̐ݒ�
	mCmdList->RSSetViewports(1, &mViewport);
	//�V�U�[��`�̐ݒ�
	mCmdList->RSSetScissorRects(1, &mScissorRect);
}

void DirectX12Tutorial::DrawPolygon()
{
	//�p�C�v���C���X�e�[�g�̐ݒ�
	mCmdList->SetPipelineState(mPipelineState);
	//���[�g�����̐ݒ�
	mCmdList->SetGraphicsRootSignature(mRootSignature);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//���_�o�b�t�@�̃A�h���X
	vertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	//���_�o�b�t�@�̑傫��
	vertexBufferView.SizeInBytes = sizeof(Vertex) * 3;
	//1���_������̑傫��
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	//���_�o�b�t�@�̐ݒ�
	mCmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
	//�v���~�e�B�u�̉��ߕ��@
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//�h���[�R�[��
	mCmdList->DrawInstanced(3, 1, 0, 0);
}

void DirectX12Tutorial::EndRendering()
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	//���\�[�X�o���A�̃^�C�v��J�ڂɐݒ�
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//�t���O�Ȃ�
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//���삷�郊�\�[�X�̃A�h���X
	resourceBarrier.Transition.pResource = mBackBaffers[mBackBufferIndex].mResource;
	//���삷��T�u���\�[�X(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES�ł��ׂĎw��)
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//���\�[�X�o���A�̑J�ڑO�̏��
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//���\�[�X�o���A�̑J�ڌ�̏��
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	mCmdList->ResourceBarrier(
		//�ݒ肷�郊�\�[�X�o���A�̐�
		1,
		//�ݒ肷�郊�\�[�X�o���A
		&resourceBarrier
	);

	//�R�}���h���X�g�����
	HRESULT hr = mCmdList->Close();
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::ExecuteCmdLists()
{
	//���s����R�}���h���X�g���ЂƂ܂Ƃ߂ɂ���
	ID3D12CommandList* const cmdLists[]
	{
		mCmdList
	};

	//���s
	mCmdQueue->ExecuteCommandLists(
		//���s����R�}���h���X�g�̐�
		ARRAYSIZE(cmdLists),
		//���s����R�}���h���X�g�̃A�h���X
		cmdLists
	);
}

void DirectX12Tutorial::WaitForPreviousFrame()
{
	/*
		(���ݐς܂�Ă���)�R�}���h�L���[�̍Ō��
		�t�F���X���̐��l����͂������l�ɂ���
		�Ƃ������߂𑗐M
	*/
	HRESULT hr = mCmdQueue->Signal(mFence, ++mSignalValue);
	HRESULT_ASSERT(hr);

	//�ҋ@����2�ʂ�̕��@�̈Ⴂ�Ɩ��_
#if 0
	//���[�v������@
	/*
		�t�F���X�̎g�p���@����ł�
		while (mSignalValue != mFence->GetCompletedValue());
		�ł͋@�\���Ȃ��ꍇ������B
		Signal��K���ҋ@����ꍇ�͖��Ȃ���
		mSignalValue = 0;
		mCmdQueue->Signal(mFence, ++mSignalValue);
		//mSignalValue��1�ɏ����ς݂̒l��0
		~~~��������̏���~~~
		mCmdQueue->Signal(mFence, ++mSignalValue);
		//mSignalValue��2�Ɉ�O�̃V�O�i���������������ς݂̒l��1��

		//�ҋ@
		while (mSignalValue != mFence->GetCompletedValue());
		//�Ō�̃V�O�i�����̒l�ɂȂ��Ă��Ȃ���mSignalValue��
		//�����ς݂̒l�Ƃ͈Ⴄ�̂őf�ʂ肷��
	*/
	while (mSignalValue > mFence->GetCompletedValue());
#else
	//�C�x���g���g�p������@(�m����������)
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
		//������������
		1,
		//�t���O�̎w��Ȃ�
		0
	);
	HRESULT_ASSERT(hr);
}

void DirectX12Tutorial::CmdReset()
{
	//�R�}���h�A���P�[�^�[�����Z�b�g
	HRESULT hr = mCmdAllocator->Reset();
	HRESULT_ASSERT(hr);
	//�R�}���h���X�g�ɃR�}���h�A���P�[�^�[���Z�b�g
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
	//ComObject��G�邤���ł̋V���̂悤�Ȃ���
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
	HRESULT_ASSERT(hr);

#if defined(DEBUG) | defined(_DEBUG)
	//�f�o�b�N���C���[�̓K�p(�K���f�o�C�X�쐬�O�ɍs��)
	ApplyDebugLayer();
#endif
	//�f�o�C�X�쐬
	CreateDevice();
	//�R�}���h�֌W�̍쐬
	CreateCmdObjects();
	//�X���b�v�`�F�C���Ə������ݐ�̍쐬
	CreateSwapChainAndRenderTargets(hWnd);
	//�t�F���X�̍쐬
	CreateFence();
	//���[�g�����̍쐬
	CreateRootSignature();
	//�p�C�v���C���X�e�[�g�̍쐬
	CreatePipelineState();
	//���_�o�b�t�@�̍쐬
	CreateVertexBuffer();
}

void DirectX12Tutorial::Render()
{
	/*
		�R�}���h�̃��Z�b�g
		�R�}���h���X�g���쐬�������Ƃ����ɃN���[�Y�����
		�t���[���̍ŏ��Ɏ��s����
		�N���[�Y�ƃ��Z�b�g�����݂ɍs����悤�ɂ���
		�쐬��������̓��Z�b�g������Ԃł���
	*/
#if CMD_CLOSE_IMMEDIATELY
	CmdReset();
#endif
	//�`��J�n����
	BeginRendering();
	//�|���S���̕`��
	DrawPolygon();
	//�`��I������
	EndRendering();
	//�R�}���h���X�g�̎��s
	ExecuteCmdLists();
	//�`�悪��������܂őҋ@
	WaitForPreviousFrame();
	//�E�B���h�E�ɑ��M
	Present();
#if !CMD_CLOSE_IMMEDIATELY
	CmdReset();
#endif
}

DirectX12Tutorial::~DirectX12Tutorial()
{
	/*
		�R�}���h�L���[�ŏ������s���Ă���Œ���
		�f�[�^��������̂�h������
		�R�}���h�L���[�̒��g���Ȃ��Ȃ�܂őҋ@
	*/
	WaitForPreviousFrame();
	//�����J��
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
	//ComObject��G�邤���ł̋V���̂悤�Ȃ���
	CoUninitialize();
}

void LoadCSO(const char* filePath, CSO* cso)
{
	std::ifstream ifs{ filePath ,std::ios::binary };
	assert(ifs.is_open() && "file not found!!");

	//�t�@�C���T�C�Y���擾
	ifs.seekg(0, std::ios_base::end);
	auto fileSize = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);

	cso->resize(static_cast<size_t>(fileSize));

	//�t�@�C���̓ǂݍ���
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