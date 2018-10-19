#include "AnimatedModel.h"

namespace AE
{
	
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
		m_MainClipData.clip = clip;
		m_MainClipData.frameCount = clip->GetFrameCount();
	
		if (m_ModelMatrices.size() != m_MainClipData.clip->GetSkeleton()->m_jointCount)
			m_ModelMatrices.resize(m_MainClipData.clip->GetSkeleton()->m_jointCount);
		if (m_SkinningMatrices.size() != m_MainClipData.clip->GetSkeleton()->m_jointCount)
			m_SkinningMatrices.resize(m_MainClipData.clip->GetSkeleton()->m_jointCount);
	}
	
	void AnimatedModel::AddAnimationLayer(std::shared_ptr<AE::AnimationClip> clip)
	{
		AnimationLayer layer;
		layer.clip = clip;
		layer.currentFrame = 0;
		layer.currentTime = m_MainClipData.currentTime;
		layer.frameCount = clip->GetFrameCount();
		layer.frameRate = 24;
		layer.isLooping = true;
		layer.isPlaying = true;
		layer.speedScale = 1.0F;
		m_AnimationLayers.push_back(layer);
	}
	
	void AnimatedModel::PopAnimationLayer(uint8_t layerIndex)
	{
		m_AnimationLayers.erase(m_AnimationLayers.begin() + layerIndex);
	}

	void AnimatedModel::SetAnimationLayer(std::shared_ptr<AnimationClip> clip, uint8_t layer)
	{
	
	}
	
	std::vector<DirectX::XMFLOAT4X4A>* AnimatedModel::GetSkinningMatrices()
	{
		return &m_SkinningMatrices;
	}
	
	const AE::PlaybackData& AnimatedModel::GetMainClipPlaybackData()
	{
		return m_MainClipData;
	}
	
	uint8_t AnimatedModel::GetLayerCount()
	{
		return static_cast<uint8_t>(m_AnimationLayers.size());
	}
	
	AE::AnimatedModelInformation AnimatedModel::GetInformation()
	{
		AnimatedModelInformation ami;
		ami.frameCount = m_MainClipData.frameCount;
		if (m_MainClipData.clip)
			ami.mainAnimationName = m_MainClipData.clip->GetName();
		if (m_MainClipData.clip && m_MainClipData.clip->GetSkeleton())
			ami.skeletonName = m_MainClipData.clip->GetSkeleton()->name;
		ami.scale = GetScale();
	
		return ami;
	}
	
	void AnimatedModel::Update(float deltaTime)
	{
		if (m_MainClipData.isPlaying && m_MainClipData.clip && m_AnimationLayers.empty())
		{
			/// increase local time
			//done in _computeIndexAndProgression()
	
			///calc the actual frame index and progression towards the next frame
			auto indexAndProgression = _computeIndexAndProgression(deltaTime, &m_MainClipData.currentTime, m_MainClipData.frameCount);
			int prevIndex = indexAndProgression.first;
			float progression = indexAndProgression.second;
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
				_computeSkinningMatrices(&m_MainClipData.clip->GetSkeletonPose(prevIndex), &m_MainClipData.clip->GetSkeletonPose(prevIndex + 1), progression);
		}
		else if (m_MainClipData.isPlaying && m_MainClipData.clip && !m_AnimationLayers.empty())
		{
			UpdateAdditive(deltaTime);
		}
	}
	
	void AnimatedModel::SetLayerWeight(float weight, int layer)
	{
		m_AnimationLayers[layer].weight = weight;
	}

	Animation::JointPose getAdditivePose(Animation::JointPose targetPose, Animation::JointPose differencePose)
	{
		using namespace DirectX;
	
		XMMATRIX targetPoseMatrix = _createMatrixFromSRT(targetPose.m_transformation);
		XMMATRIX differencePoseMatrix = _createMatrixFromSRT(differencePose.m_transformation);
		XMMATRIX additivePoseMatrix = XMMatrixMultiply(targetPoseMatrix, differencePoseMatrix);
	
	
		Animation::SRT additivePose = {};
		XMVECTOR s, r, t;
		XMMatrixDecompose(&s, &r, &t, additivePoseMatrix);
		XMStoreFloat4A(&additivePose.m_scale, s);
		XMStoreFloat4A(&additivePose.m_rotationQuaternion, r);
		XMStoreFloat4A(&additivePose.m_translation, t);
	
		return Animation::JointPose(additivePose);
	}

	Animation::JointPose getAdditivePose(Animation::JointPose targetPose, DirectX::XMMATRIX differencePose)
	{
		using namespace DirectX;

		XMMATRIX targetPoseMatrix = _createMatrixFromSRT(targetPose.m_transformation);
		XMMATRIX differencePoseMatrix = differencePose;
		XMMATRIX additivePoseMatrix = XMMatrixMultiply(differencePoseMatrix, targetPoseMatrix);


		Animation::SRT additivePose = {};
		XMVECTOR s, r, t;
		XMMatrixDecompose(&s, &r, &t, additivePoseMatrix);
		XMStoreFloat4A(&additivePose.m_scale, s);
		XMStoreFloat4A(&additivePose.m_rotationQuaternion, r);
		XMStoreFloat4A(&additivePose.m_translation, t);

		return Animation::JointPose(additivePose);
	}
	
	void AnimatedModel::UpdateAdditive(float deltaTime)
	{
		auto skeleton = m_MainClipData.clip->GetSkeleton();
		auto mainIndexAndProgression = _computeIndexAndProgression(deltaTime, &m_MainClipData.currentTime, m_MainClipData.frameCount);
		int mainPrevIndex = mainIndexAndProgression.first;
		float mainProgression = mainIndexAndProgression.second;

		// Interpolate main clip pose and store in newPose
		auto& mainSkeletonPoseFirst = m_MainClipData.clip->GetSkeletonPose(mainPrevIndex);
		auto& mainSkeletonPoseSecond = m_MainClipData.clip->GetSkeletonPose(mainPrevIndex + 1);

		Animation::SkeletonPose newPose;
		newPose.m_jointPoses = std::make_unique<Animation::JointPose[]>(skeleton->m_jointCount);
		_interpolatePose(&mainSkeletonPoseFirst, &mainSkeletonPoseSecond, mainProgression, &newPose, skeleton->m_jointCount);



		//Init vector of final layer matrices
		std::vector<DirectX::XMMATRIX> layerPosesMultiplied;
		layerPosesMultiplied.resize(skeleton->m_jointCount);
		for (int layer = 0; layer < m_AnimationLayers.size(); layer++)
		{
			auto layerIndexAndProgression = _computeIndexAndProgression(deltaTime, &m_AnimationLayers[layer].currentTime, m_AnimationLayers[layer].frameCount);
			for (int joint = 0; joint < skeleton->m_jointCount; joint++)
			{
				//Get the final matrix for this layer and this joint
				auto thisPoseMatrix = _createMatrixFromSRT(_getPoseFromLayer(m_AnimationLayers[layer], joint, deltaTime, layerIndexAndProgression).m_transformation);
				layer
					? layerPosesMultiplied[joint] = XMMatrixMultiply(thisPoseMatrix, layerPosesMultiplied[joint])
					: layerPosesMultiplied[joint] = thisPoseMatrix;
			}
		}
		
		//Multiply layer matrices with main clip
		for (int i = 0; i < skeleton->m_jointCount; i++)
			newPose.m_jointPoses[i] = getAdditivePose(newPose.m_jointPoses[i], layerPosesMultiplied[i]);

		_computeSkinningMatrices(&newPose);
	}
	
	void AnimatedModel::_weightPose(Animation::JointPose& jointPose, float weight)
	{
		using namespace DirectX;
		Animation::JointPose zeroPose;
		XMStoreFloat4A(&zeroPose.m_transformation.m_rotationQuaternion, XMQuaternionIdentity());
		zeroPose.m_transformation.m_scale = { 1.0, 1.0, 1.0, 1.0 };
		zeroPose.m_transformation.m_translation = { 0.0f, 0.0f, 0.0f, 1.0f };

		jointPose = _interpolateJointPose(&zeroPose, &jointPose, weight);
	}

	Animation::JointPose AnimatedModel::_getPoseFromLayer(AE::AnimationLayer& layer, uint8_t jointIndex, float deltaTime, std::pair<int, float>layerIndexAndProgression)
	{
		int layerPrevIndex = layerIndexAndProgression.first;
		float layerProgression = layerIndexAndProgression.second;

		auto& firstSkeletonPose = layer.clip->GetSkeletonPose(layerPrevIndex);
		auto& secondSkeletonPose = layer.clip->GetSkeletonPose(layerPrevIndex + 1);

		auto firstJointPose = firstSkeletonPose.m_jointPoses[jointIndex];
		auto secondJointPose = secondSkeletonPose.m_jointPoses[jointIndex];
		//#todo
		{
			if (layer.weight < 0.999999f)
			{
				_weightPose(firstJointPose, layer.weight);
				_weightPose(secondJointPose, layer.weight);
			}
		}

		return _interpolateJointPose(&firstJointPose, &secondJointPose, layerProgression);
	}

	void AnimatedModel::_computeSkinningMatrices(Animation::SkeletonPose * firstPose, Animation::SkeletonPose * secondPose, float weight)
	{
		using namespace DirectX;
	
		auto skeleton = m_MainClipData.clip->GetSkeleton();
	
		_computeModelMatrices(firstPose, secondPose, weight);
	
		for (int i = 0; i < skeleton->m_jointCount; i++)
		{
			const XMFLOAT4X4A& global = m_ModelMatrices[i];
			const XMFLOAT4X4A& inverseBindPose = skeleton->m_joints[i].m_inverseBindPose;
	
			XMMATRIX skinningMatrix = XMMatrixMultiply(XMLoadFloat4x4A(&inverseBindPose), XMLoadFloat4x4A(&global)); // #matrixmultiplication
	
			DirectX::XMStoreFloat4x4A(&m_SkinningMatrices[i], skinningMatrix);
		}
	}

	void AnimatedModel::_computeSkinningMatrices(Animation::SkeletonPose * preInterpolatedPose)
	{
		using namespace DirectX;

		auto skeleton = m_MainClipData.clip->GetSkeleton();

		_computeModelMatrices(preInterpolatedPose);

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
	
		auto skeleton = m_MainClipData.clip->GetSkeleton();
	
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
		auto skeleton = m_MainClipData.clip->GetSkeleton();
		auto rootJointPose = _interpolateJointPose(&firstPose->m_jointPoses[0], &secondPose->m_jointPoses[0], weight);
		DirectX::XMStoreFloat4x4A(&m_ModelMatrices[0], Animation::_createMatrixFromSRT(rootJointPose.m_transformation));
	
		for (int i = 1; i < skeleton->m_jointCount; i++) //start at second joint (first is root, already processed)
		{
			const int16_t parentIndex = skeleton->m_joints[i].parentIndex;
			const XMMATRIX parentGlobalMatrix = XMLoadFloat4x4A(&m_ModelMatrices[parentIndex]);
			auto jointPose = _interpolateJointPose(&firstPose->m_jointPoses[i], &secondPose->m_jointPoses[i], weight);
	
			DirectX::XMStoreFloat4x4A(&m_ModelMatrices[i], XMMatrixMultiply(Animation::_createMatrixFromSRT(jointPose.m_transformation), parentGlobalMatrix));
		}
	}

	void AnimatedModel::_computeModelMatrices(Animation::SkeletonPose * preInterpolatedPose)
	{
		using namespace DirectX;
		auto skeleton = m_MainClipData.clip->GetSkeleton();
		auto rootJointPose = preInterpolatedPose->m_jointPoses[0];
		DirectX::XMStoreFloat4x4A(&m_ModelMatrices[0], Animation::_createMatrixFromSRT(rootJointPose.m_transformation));

		for (int i = 1; i < skeleton->m_jointCount; i++) //start at second joint (first is root, already processed)
		{
			const int16_t parentIndex = skeleton->m_joints[i].parentIndex;
			const XMMATRIX parentGlobalMatrix = XMLoadFloat4x4A(&m_ModelMatrices[parentIndex]);
			auto jointPose = preInterpolatedPose->m_jointPoses[i];

			DirectX::XMStoreFloat4x4A(&m_ModelMatrices[i], XMMatrixMultiply(Animation::_createMatrixFromSRT(jointPose.m_transformation), parentGlobalMatrix));
		}
	}
	
	void AnimatedModel::_computeModelMatrices(Animation::SkeletonPose* firstPose1, Animation::SkeletonPose* secondPose1, float weight1, Animation::SkeletonPose* firstPose2, Animation::SkeletonPose* secondPose2, float weight2)
	{
	}
	
	void AnimatedModel::_interpolatePose(Animation::SkeletonPose* firstPose, Animation::SkeletonPose* secondPose, float weight, Animation::SkeletonPose* outPose, uint8_t jointCount)
	{
		for (int i = 0; i < jointCount; i++)
			outPose->m_jointPoses[i] = _interpolateJointPose(&firstPose->m_jointPoses[i], &secondPose->m_jointPoses[i], weight);
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

	std::pair<uint16_t, float> AnimatedModel::_computeIndexAndProgression(float deltaTime, float currentTime, uint16_t frameCount)
	{
		currentTime += deltaTime;
		frameCount -= 1;

		float properTime = std::fmod(currentTime, frameCount / 24.0);
		currentTime = properTime;
		///calc the actual frame index and progression towards the next frame
		float actualTime = properTime / (1.0 / 24.0);
		int prevIndex = (int)(actualTime);
		float progression = (actualTime)-(float)prevIndex;

		//return values
		return std::make_pair(static_cast<uint16_t>(prevIndex), progression);
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
		return m_MainClipData.currentTime / (1.0 / 24.0 * (m_MainClipData.frameCount - 1));
	}
}

