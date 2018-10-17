#include "GraphicsHandler.h"
#include "importer/formatImporter.h"
#include "AnimationEditorApplication.h"
#include <d3dcompiler.h>
AE::GraphicsHandler::GraphicsHandler()
{

}

AE::GraphicsHandler::~GraphicsHandler()
{

}

void AE::GraphicsHandler::Init()
{
	_initInputLayoutsAndShaders();
}

bool AE::GraphicsHandler::LoadStaticModel(std::string file, std::string name)
{
	MyLibrary::Loadera loader;
	auto meshData = loader.readMeshFile(file);

	//Make buffer
	_createVertexBuffer(meshData.mesh_vertices, meshData.mesh_nrOfVertices, sizeof(float) * 11, name);

	auto model = std::make_shared<Model>(m_BufferMap.at(name), m_StaticModelLayout, meshData.mesh_nrOfVertices);
	m_StaticModelMap.insert(std::make_pair(name, model));
	return false;
}

bool AE::GraphicsHandler::LoadAnimatedModel(std::string file, std::string name)
{
	MyLibrary::Loadera loader;
	auto meshData = loader.readAnimatedMeshFile(file);

	//Make buffer
	size_t vsize = sizeof(meshData.mesh_vertices[0]);
	//struct ProperAnimatedVertex
	//{
	//	DirectX::XMFLOAT4A pos;
	//	DirectX::XMFLOAT4A nor;
	//	DirectX::XMFLOAT4A tan;
	//	DirectX::XMFLOAT4 weights;
	//	DirectX::XMUINT4 influences;
	//	DirectX::XMFLOAT2A uv;
	//};

	//std::vector<ProperAnimatedVertex> verts;

	//for (int v = 0; v < meshData.mesh_nrOfVertices; v++)
	//{
	//	ProperAnimatedVertex pv;
	//	auto sv = meshData.mesh_vertices[v];
	//	pv.pos = { sv.vertex_position[0], sv.vertex_position[1], sv.vertex_position[2], 1.0f };
	//	pv.nor = { sv.vertex_position[0], sv.vertex_position[1], sv.vertex_position[2], 1.0f };
	//	pv.tan = { sv.vertex_position[0], sv.vertex_position[1], sv.vertex_position[2], 1.0f };
	//}

	_createVertexBuffer(meshData.mesh_vertices, meshData.mesh_nrOfVertices, (sizeof(float) * 15) + (sizeof(unsigned int) * 4), name);

	auto model = std::make_shared<AnimatedModel>(m_BufferMap.at(name), m_AnimatedModelLayout, meshData.mesh_nrOfVertices);
	m_AnimatedModelMap.insert(std::make_pair(name, model));
	delete[] meshData.mesh_vertices;
	return true;
}

AE::AnimatedModelMap & AE::GraphicsHandler::GetAnimatedModelMap()
{
	return m_AnimatedModelMap;
}

AE::StaticModelMap & AE::GraphicsHandler::GetStaticModelMap()
{
	return m_StaticModelMap;
}

ComPtr<ID3D11InputLayout> AE::GraphicsHandler::GetStaticModelInputLayout()
{
	return m_StaticModelLayout;
}

ComPtr<ID3D11InputLayout> AE::GraphicsHandler::GetAnimatedModelInputLayout()
{
	return m_AnimatedModelLayout;
}

ComPtr<ID3D11VertexShader> AE::GraphicsHandler::GetStaticModelVertexShader()
{
	return m_StaticModelVertexShader;
}

ComPtr<ID3D11VertexShader> AE::GraphicsHandler::GetAnimatedModelVertexShader()
{
	return m_AnimatedModelVertexShader;
}

ComPtr<ID3D11PixelShader> AE::GraphicsHandler::GetPixelShader()
{
	return m_StaticModelPixelShader;
}

AE::SharedAnimatedModel AE::GraphicsHandler::GetAnimatedModel(std::string key)
{
	return m_AnimatedModelMap.at(key);
}

void AE::GraphicsHandler::_initInputLayoutsAndShaders()
{
	// Binary Large OBject (BLOB), for compiled shader, and errors.
	ID3DBlob* pVS = nullptr;
	ID3DBlob* pAnimVS = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT result = S_OK;

	///Create Static VS
	{
		result = D3DCompileFromFile(
			L"BasicVS.hlsl", // filename
			nullptr,		// optional macros
			nullptr,		// optional include files
			"VS_main",		// entry point
			"vs_5_0",		// shader model (target)
			D3DCOMPILE_DEBUG,	// shader compile options (DEBUGGING)
			0,				// IGNORE...DEPRECATED.
			&pVS,			// double pointer to ID3DBlob		
			&errorBlob		// pointer for Error Blob messages.
		);

		// compilation failed?
		if (FAILED(result))
		{
			exit(-9);
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pVS)
				pVS->Release();

		}

		AEApp::gDevice->CreateVertexShader(
			pVS->GetBufferPointer(),
			pVS->GetBufferSize(),
			nullptr,
			m_StaticModelVertexShader.ReleaseAndGetAddressOf()
		);
	}
	///Create Animated VS
	{
		result = D3DCompileFromFile(
			L"AnimatedVS.hlsl", // filename
			nullptr,		// optional macros
			nullptr,		// optional include files
			"VS_main",		// entry point
			"vs_5_0",		// shader model (target)
			D3DCOMPILE_DEBUG,	// shader compile options (DEBUGGING)
			0,				// IGNORE...DEPRECATED.
			&pAnimVS,			// double pointer to ID3DBlob		
			&errorBlob		// pointer for Error Blob messages.
		);

		// compilation failed?
		if (FAILED(result))
		{
			exit(-9);
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pAnimVS)
				pAnimVS->Release();

		}

		AEApp::gDevice->CreateVertexShader(
			pAnimVS->GetBufferPointer(),
			pAnimVS->GetBufferSize(),
			nullptr,
			m_AnimatedModelVertexShader.ReleaseAndGetAddressOf()
		);
	}
	///Create Static input layout
	{
		D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
			{
				"POSITION",		// "semantic" name in shader
				0,				// "semantic" index (not used)
				DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
				0,							 // input slot
				0,							 // offset of first element
				D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
				0							 // used for INSTANCING (ignore)
			},
			{
				"UV",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"NORMAL",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"TANGENT",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
		};

		AEApp::gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), m_StaticModelLayout.ReleaseAndGetAddressOf());
		pVS->Release();
	}
	///Create Animated input layout
	{
		D3D11_INPUT_ELEMENT_DESC inputDescAnim[] = {
			{
				"POSITION",		// "semantic" name in shader
				0,				// "semantic" index (not used)
				DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
				0,							 // input slot
				0,							 // offset of first element
				D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
				0							 // used for INSTANCING (ignore)
			},
			{
				"UV",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"NORMAL",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"TANGENT",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
			{
				"JOINTINFLUENCES",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32B32A32_UINT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
						{
				"JOINTWEIGHTS",
				0,				// same slot as previous (same vertexBuffer)
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, // offset of FIRST element (after POSITION)
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			}
		};

		AEApp::gDevice->CreateInputLayout(inputDescAnim, ARRAYSIZE(inputDescAnim), pAnimVS->GetBufferPointer(), pAnimVS->GetBufferSize(), m_AnimatedModelLayout.ReleaseAndGetAddressOf());
		pAnimVS->Release();
	}
	///Create Static PS
	{
		//create pixel shader
		ID3DBlob* pPS = nullptr;
		if (errorBlob) errorBlob->Release();
		errorBlob = nullptr;

		result = D3DCompileFromFile(
			L"BasicPS.hlsl", // filename
			nullptr,		// optional macros
			nullptr,		// optional include files
			"PS_main",		// entry point
			"ps_5_0",		// shader model (target)
			D3DCOMPILE_DEBUG,	// shader compile options
			0,				// effect compile options
			&pPS,			// double pointer to ID3DBlob		
			&errorBlob			// pointer for Error Blob messages.
		);

		// compilation failed?
		if (FAILED(result))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				// release "reference" to errorBlob interface object
				errorBlob->Release();
			}
			if (pPS)
				pPS->Release();

		}

		AEApp::gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, m_StaticModelPixelShader.ReleaseAndGetAddressOf());
		// we do not need anymore this COM object, so we release it.
		pPS->Release();
	}
}

void AE::GraphicsHandler::_createVertexBuffer(PVOID64 vertexData, size_t vertexCount, size_t vertexSize, std::string name)
{
	ComPtr<ID3D11Buffer> buffer = nullptr;

	// Describe the Vertex Buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	// what type of buffer will this be?
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// what type of usage (press F1, read the docs)
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	bufferDesc.ByteWidth = vertexSize * vertexCount;

	// this struct is created just to set a pointer to the
	// data containing the vertices.
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertexData;

	// create a Vertex Buffer
	HRESULT hr = AEApp::gDevice->CreateBuffer(&bufferDesc, &data, buffer.ReleaseAndGetAddressOf());
	assert(hr == S_OK);
	m_BufferMap.insert(std::make_pair(name, buffer));
}

AE::SharedStaticModel AE::GraphicsHandler::GetStaticModel(std::string key)
{
	return m_StaticModelMap.at(key);
}

void AE::GraphicsHandler::DuplicateModel(std::string key, std::string name)
{
	AE::SharedAnimatedModel newModel = std::make_shared<AE::AnimatedModel>(*(m_AnimatedModelMap.at(key)));
	m_AnimatedModelMap.insert(std::make_pair(name, newModel));
	gAnimatedMeshNames.push_back(name);
}
