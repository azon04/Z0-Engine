#include "SceneEditor.h"

#include "ZEGameContext.h"

#include "Platform/Platform.h"

#include "ZooidEngine/Scene/SceneManager.h"
#include "ZooidEngine/FileSystem/DirectoryHelper.h"
#include "ZooidEngine/ResourceManagers/AnimationManager.h"

#include "ZooidEngine/Animation/AnimationComponent.h"
#include "ZooidEngine/Animation/Animation.h"

#include "ZooidEngine/Scene/CameraManager.h"
#include "ZooidEngine/Scene/CameraComponent.h"

namespace ZE
{
	void SceneEditor::Setup(GameContext* _gameContext)
	{
		int argCount = Platform::GetArgCount();
		String scenePath(1024);

		if (argCount >= 1)
		{
			Platform::GetArgByIndex(0, scenePath.c_str());
		}

		_gameContext->getSceneManager()->loadSceneFile(GetResourcePath(scenePath.const_str()).c_str());

		{
			CameraComponent* cameraComp = CameraManager::GetInstance()->getCurrentCamera();
			Quaternion q;
			Matrix4x4 transform;
			transform.fromQuaternion(q);
			transform.setPos(Vector3(0.0f, 1.0f, 5.0f));
			cameraComp->setWorldTransform(transform);
		}
	}

	void SceneEditor::Tick(GameContext* _gameContext, float deltaTime)
	{

	}

	void SceneEditor::Clean(GameContext* _gameContext)
	{

	}

	Application* Application::GetApplication()
	{
		static SceneEditor application;
		return &application;
	}

}
