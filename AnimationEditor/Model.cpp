#include "Model.h"



namespace AE
{
	Model::Model()
	{
		using namespace DirectX;
	
		m_InputLayout->Release();
		m_VertexBuffer->Release();
	
		m_Translation = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_RotationQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_Scale = { 1.0f, 1.0f, 1.0f, 0.0f };
	
		XMStoreFloat4x4A(&m_WorldMatrix, XMMatrixIdentity());
	}
	
	
	Model::Model(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount) : m_VertexBuffer(buffer), m_InputLayout(layout), m_VertexCount(vertexCount)
	{
		using namespace DirectX;
		m_Translation = {0.0f, 0.0f, 0.0f, 1.0f};
		m_RotationQuaternion = {0.0f, 0.0f, 0.0f, 1.0f};
		m_Scale = {1.0f, 1.0f, 1.0f, 0.0f};
		
		XMStoreFloat4x4A(&m_WorldMatrix, XMMatrixIdentity());
	}
	
	Model::~Model()
	{
	}
	
	void Model::SetPosition(DirectX::XMFLOAT4A newPosition)
	{
		m_Translation = newPosition;
	}
	
	void Model::SetRotation(DirectX::XMFLOAT4A newRotation)
	{
		m_RotationQuaternion = newRotation;
	}
	
	void Model::SetScale(float newScale)
	{
		m_Scale = { newScale, newScale, newScale, 0.0 };
	}
	
	void Model::SetVertexBuffer(const ComPtr<ID3D11Buffer>& buffer)
	{
		m_VertexBuffer = buffer;
	}

	void Model::SetVertexCount(uint32_t count)
	{
		m_VertexCount = count;
	}
	
	void Model::SetVertexLayout(const ComPtr<ID3D11InputLayout>& layout)
	{
		m_InputLayout = layout;
	}
	
	std::shared_ptr<AnimatedModel> Model::AsAnimatedModel()
	{
		return std::dynamic_pointer_cast<AnimatedModel>(shared_from_this());
	}
	
	DirectX::XMFLOAT4X4A Model::GetWorldMatrix()
	{
		using namespace DirectX;
	
		XMVECTOR t = XMLoadFloat4A(&m_Translation);
		XMVECTOR r = XMLoadFloat4A(&m_RotationQuaternion);
		XMVECTOR s = XMLoadFloat4A(&m_Scale);
	
		XMStoreFloat4x4A(&m_WorldMatrix, DirectX::XMMatrixAffineTransformation(s, XMVectorZero(), r, t));
		return m_WorldMatrix;
	}
	
	const ComPtr<ID3D11Buffer>& Model::GetVertexBuffer()
	{
		return m_VertexBuffer;
	}

	uint32_t Model::GetVertexCount()
	{
		return m_VertexCount;
	}

	void Model::ToggleDrawState()
	{
		m_bDraw = !m_bDraw;
	}

	bool Model::GetDrawState()
	{
		return m_bDraw;
	}

	AnimatedModel::AnimatedModel()
	{
	}

	AnimatedModel::AnimatedModel(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount) : Model(buffer, layout, vertexCount)
	{
		//#URGENT_TODO
		m_SkinningMatrices.resize(128);
		for (auto& m : m_SkinningMatrices)
		{
			DirectX::XMStoreFloat4x4A(&m, DirectX::XMMatrixIdentity());
		}
	}

	AnimatedModel::AnimatedModel(const ComPtr<ID3D11Buffer>& buffer, const ComPtr<ID3D11InputLayout>& layout, uint32_t vertexCount, std::shared_ptr<AnimationClip> clip) : Model(buffer, layout, vertexCount)
	{
		SetMainClip(clip);
	}

	void AnimatedModel::SetMainClip(std::shared_ptr<AnimationClip> clip)
	{
		m_MainClip = clip;
		m_MainClipData.clip = clip;
		m_MainClipData.frameCount = clip->GetFrameCount();

		if (m_ModelMatrices.size() != m_MainClip->GetSkeleton()->m_jointCount)
			m_ModelMatrices.resize(m_MainClip->GetSkeleton()->m_jointCount);
		if (m_SkinningMatrices.size() != m_MainClip->GetSkeleton()->m_jointCount)
			m_SkinningMatrices.resize(m_MainClip->GetSkeleton()->m_jointCount);
	}

	void AnimatedModel::SetAnimationLayer(std::shared_ptr<AnimationClip> clip, uint8_t layer)
	{
		m_AnimationLayer1 = clip; //#todo
		m_AnimationLayer1Data.clip = clip;
	}

	std::vector<DirectX::XMFLOAT4X4A>* AnimatedModel::GetSkinningMatrices()
	{
		return &m_SkinningMatrices;
	}

	void AnimatedModel::Update(float deltaTime)
	{
		if (m_MainClipData.isPlaying && m_MainClipData.clip)
		{
			/// increase local time
			//done in _computeIndexAndProgression()

			///calc the actual frame index and progression towards the next frame
			int prevIndex = std::floorf(24 * m_MainClipData.currentTime);
			float progression = std::fmod(m_MainClipData.currentTime, 1.0 / 24.0) * 24.0;
			auto indexAndProgression = _computeIndexAndProgression(deltaTime, &m_MainClipData.currentTime, m_MainClipData.frameCount);
			prevIndex = indexAndProgression.first;
			progression = indexAndProgression.second;
			///if we exceeded clips time, set back to 0 ish if we are looping, or stop if we aren't
			if (prevIndex >= m_MainClipData.frameCount - 1) /// -1 because last frame is only used to interpolate towards
			{
				if (m_MainClipData.isLooping)
				{
					m_MainClipData.currentTime = 0.0 + progression;

					prevIndex = std::floorf(m_MainClipData.frameRate / 2 * m_MainClipData.currentTime);
					progression = std::fmod(m_MainClipData.currentTime, 1.0 / 24.0);
				}
				else
				{
					m_MainClipData.isPlaying = false;
				}
			}

			/// compute skinning matrices
			if (m_MainClipData.isPlaying)
				_computeSkinningMatrices(&m_MainClip->GetSkeletonPose(prevIndex), &m_MainClip->GetSkeletonPose(prevIndex + 1), progression);
		}
	}

	void AnimatedModel::_computeSkinningMatrices(Animation::SkeletonPose * firstPose, Animation::SkeletonPose * secondPose, float weight)
	{
		using namespace DirectX;

		auto skeleton = m_MainClip->GetSkeleton();

		_computeModelMatrices(firstPose, secondPose, weight);

		for (int i = 0; i < skeleton->m_jointCount; i++)
		{
			const XMFLOAT4X4A& global = m_ModelMatrices[i];
			const XMFLOAT4X4A& inverseBindPose = skeleton->m_joints[i].m_inverseBindPose;

			XMMATRIX skinningMatrix = XMMatrixMultiply(XMLoadFloat4x4A(&inverseBindPose), XMLoadFloat4x4A(&global)); // #matrixmultiplication

			DirectX::XMStoreFloat4x4A(&m_SkinningMatrices[i], skinningMatrix);
		}
	}

	void AnimatedModel::_computeSkinningMatrices(Animation::SkeletonPose * firstPose1, Animation::SkeletonPose * secondPose1, float weight1, Animation::SkeletonPose * firstPose2, Animation::SkeletonPose * secondPose2, float weight2)
	{
		using namespace DirectX;
		
		auto skeleton = m_MainClip->GetSkeleton();

		_computeModelMatrices(firstPose1, secondPose1, weight1, firstPose2, secondPose2, weight2);

		for (int i = 0; i < skeleton->m_jointCount; i++)
		{
			const XMFLOAT4X4A& global = m_ModelMatrices[i];
			const XMFLOAT4X4A& inverseBindPose = skeleton->m_joints[i].m_inverseBindPose;

			XMMATRIX skinningMatrix = XMMatrixMultiply(XMLoadFloat4x4A(&inverseBindPose), XMLoadFloat4x4A(&global)); // #matrixmultiplication

			DirectX::XMStoreFloat4x4A(&m_SkinningMatrices[i], skinningMatrix);
		}
	}

	void AnimatedModel::_computeModelMatrices(Animation::SkeletonPose * firstPose, Animation::SkeletonPose * secondPose, float weight)
	{
		using namespace DirectX;
		auto skeleton = m_MainClip->GetSkeleton();
		auto rootJointPose = _interpolateJointPose(&firstPose->m_jointPoses[0], &secondPose->m_jointPoses[0], weight);
		DirectX::XMStoreFloat4x4A(&m_ModelMatrices[0], Animation::_createMatrixFromSRT(rootJointPose.m_transformation));

		for (int i = 1; i < skeleton->m_jointCount; i++) //start at second joint (first is root, already processed)
		{
			const int16_t parentIndex = skeleton->m_joints[i].parentIndex;
			const XMMATRIX parentGlobalMatrix = XMLoadFloat4x4A(&m_ModelMatrices[parentIndex]);
			auto jointPose = _interpolateJointPose(&firstPose->m_jointPoses[i], &secondPose->m_jointPoses[i], weight);

			DirectX::XMStoreFloat4x4A(&m_ModelMatrices[i], XMMatrixMultiply(Animation::_createMatrixFromSRT(jointPose.m_transformation), parentGlobalMatrix)); // #matrixmultiplication
		}
	}

	void AnimatedModel::_computeModelMatrices(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2)
	{
	}

	void AnimatedModel::_interpolatePose(Animation::SkeletonPose * firstPose, Animation::SkeletonPose * secondPose, float weight)
	{
	}

	Animation::JointPose AnimatedModel::_interpolateJointPose(Animation::JointPose * firstPose, Animation::JointPose * secondPose, float weight)
	{
		using namespace DirectX;

		XMVECTOR firstRotation = XMLoadFloat4A(&firstPose->m_transformation.m_rotationQuaternion);
		XMVECTOR secondRotation = XMLoadFloat4A(&secondPose->m_transformation.m_rotationQuaternion);
		XMVECTOR firstTranslation = XMLoadFloat4A(&firstPose->m_transformation.m_translation);
		XMVECTOR secondTranslation = XMLoadFloat4A(&secondPose->m_transformation.m_translation);
		XMVECTOR firstScale = XMLoadFloat4A(&firstPose->m_transformation.m_scale);
		XMVECTOR secondScale = XMLoadFloat4A(&secondPose->m_transformation.m_scale);

		DirectX::XMVECTOR newRotation = XMQuaternionSlerp(firstRotation, secondRotation, weight);
		DirectX::XMVECTOR newTranslation = XMVectorLerp(firstTranslation, secondTranslation, weight);
		DirectX::XMVECTOR newScale = XMVectorLerp(firstScale, secondScale, weight);

		Animation::SRT srt = {};
		XMStoreFloat4A(&srt.m_rotationQuaternion, newRotation);
		XMStoreFloat4A(&srt.m_scale, newScale);
		XMStoreFloat4A(&srt.m_translation, newTranslation);

		return Animation::JointPose(srt);
	}

	std::pair<uint16_t, float> AnimatedModel::_computeIndexAndProgression(float deltaTime, float * currentTime, uint16_t frameCount)
	{
		*currentTime += deltaTime;
		frameCount -= 1;

		float properTime = std::fmod(*currentTime, frameCount / 24.0);
		*currentTime = properTime;
		///calc the actual frame index and progression towards the next frame
		float actualTime = properTime / (1.0 / 24.0);
		int prevIndex = (int)(actualTime);
		float progression = (actualTime)-(float)prevIndex;

		//return values
		return std::make_pair(static_cast<uint16_t>(prevIndex), progression);
	}

	float AnimatedModel::GetProgressNormalized() const
	{
		return m_MainClipData.currentTime / (1.0 / 24.0 * (m_MainClipData.frameCount-1));
	}

}
