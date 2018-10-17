#pragma once
#include "Animation.h"
#include "Model.h"
namespace AE
{
	class GraphicsHandler
	{
	public:
		GraphicsHandler();
		~GraphicsHandler();
		void Init();
		bool LoadStaticModel(std::string file, std::string name);
		bool LoadAnimatedModel(std::string file, std::string name);
	
		AE::AnimatedModelMap& GetAnimatedModelMap();
		AE::StaticModelMap& GetStaticModelMap();
		ComPtr<ID3D11InputLayout> GetStaticModelInputLayout();
		ComPtr<ID3D11InputLayout> GetAnimatedModelInputLayout();
		ComPtr<ID3D11VertexShader> GetStaticModelVertexShader();
		ComPtr<ID3D11VertexShader> GetAnimatedModelVertexShader();
		ComPtr<ID3D11PixelShader> GetPixelShader();
		SharedAnimatedModel GetAnimatedModel(std::string key);
	private:
		ComPtr<ID3D11InputLayout> m_StaticModelLayout;
		ComPtr<ID3D11InputLayout> m_AnimatedModelLayout;
		ComPtr<ID3D11VertexShader> m_StaticModelVertexShader;
		ComPtr<ID3D11PixelShader> m_StaticModelPixelShader;
		ComPtr<ID3D11VertexShader> m_AnimatedModelVertexShader;
		ComPtr<ID3D11PixelShader> m_AnimatedModelPixelShader;

		std::unordered_map<std::string, ComPtr<ID3D11Buffer>> m_BufferMap;
		AE::StaticModelMap m_StaticModelMap;
		AE::AnimatedModelMap m_AnimatedModelMap;
	
		void _initInputLayoutsAndShaders();
		void _createVertexBuffer(PVOID64 vertexData, size_t vertexCount, size_t vertexSize, std::string name);
	public:
		SharedStaticModel GetStaticModel(std::string key);
		void DuplicateModel(std::string key, std::string name);
	};
}