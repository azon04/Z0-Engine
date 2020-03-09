#include "SceneManager.h"
#include "ZEGameContext.h"

#include "FileSystem/FileReader.h"
#include "FileSystem/DirectoryHelper.h"

#include "Scene/Light/LightComponent.h"
#include "Scene/RenderComponent.h"
#include "Scene/CameraComponent.h"
#include "Scene/Skybox.h"
#include "Collision/BoxComponent.h"
#include "Collision/SphereComponent.h"
#include "Collision/CapsuleComponent.h"
#include "Animation/AnimationComponent.h"
#include "Animation/AnimationSM.h"

#include "Logging/LogManager.h"
#include "Utils/ZEngineHelper.h"

#include "Resources/Material.h"

#include "ResourceManagers/AnimationManager.h"
#include "ResourceManagers/MaterialManager.h"

#include "Utils/StringFunc.h"

#include "Memory/Handle.h"

#include "SceneComponent.h"

namespace ZE
{
	IMPLEMENT_CLASS_1(SceneManager, Component);

	SceneManager* SceneManager::s_instance = nullptr;

	void SceneManager::loadSceneFile(const char* filePath)
	{
		loadSceneFileToComp(filePath, m_gameContext->getRootComponent());
	}

	Handle SceneManager::CreateSceneComponentByName(const char* componentTypeName)
	{
		if (StringFunc::Compare(componentTypeName, LightComponent::GetClassName()) == 0)
		{
			Handle h("Light Component", sizeof(LightComponent));
			new(h) LightComponent(m_gameContext);
			return h;
		}
		else if (StringFunc::Compare(componentTypeName, RenderComponent::GetClassName()) == 0)
		{
			Handle h("Render Component", sizeof(RenderComponent));
			new (h) RenderComponent(m_gameContext);
			return h;
		}
		else if (StringFunc::Compare(componentTypeName, CameraComponent::GetClassName()) == 0)
		{
			Handle h("Component", sizeof(CameraComponent));
			new(h) CameraComponent(m_gameContext);
			return h;
		}
		else if (StringFunc::Compare(componentTypeName, BoxComponent::GetClassName()) == 0)
		{
			Handle h("Component", sizeof(BoxComponent));
			new(h) BoxComponent(m_gameContext);
			return h;
		}
		else if (StringFunc::Compare(componentTypeName, SphereComponent::GetClassName()) == 0)
		{
			Handle h("Component", sizeof(SphereComponent));
			new(h) SphereComponent(m_gameContext);
			return h;
		}
		else if (StringFunc::Compare(componentTypeName, CapsuleComponent::GetClassName()) == 0)
		{
			Handle h("Component", sizeof(CapsuleComponent));
			new(h) CapsuleComponent(m_gameContext);
			return h;
		}
		else if (StringFunc::Compare(componentTypeName, Skybox::GetClassName()) == 0)
		{
			Handle h("Skybox", sizeof(Skybox));
			new(h) Skybox(m_gameContext);
			return h;
		}
		else
		{
			Handle h("Component", sizeof(SceneComponent));
			new(h) SceneComponent(m_gameContext);
			return h;
		}
	}

	void SceneManager::loadSceneFileToComp(const char* filePath, Component* parent)
	{
		FileReader fileReader(filePath);
		if (!fileReader.isValid())
		{
			ZEINFO("Failed to load scene file: %s", filePath);
		}

		char buff[256];
		
		// #TODO read Scene Name
		fileReader.readNextString(buff);
		fileReader.readNextString(buff);

		// #TODO read file version
		fileReader.readNextString(buff);
		fileReader.readNextString(buff);

		// Read component count
		fileReader.readNextString(buff);
		Int32 compCount = fileReader.readNextInt();

		ZELOG(LOG_GAME, Log, "Load scene \"%s\" with %d components", filePath, compCount);

		for (Int32 i = 0; i < compCount; i++)
		{
			loadSceneComponentToComp(&fileReader, parent);
		}
	}

	void SceneManager::loadSceneComponentToComp(FileReader* fileReader, Component* parent)
	{
		char buff[256];
		// Component type
		fileReader->readNextString(buff);
		Handle compHandle = CreateSceneComponentByName(buff);
		SceneComponent* pComp = compHandle.getObject<SceneComponent>();

		// #TODO component name
		fileReader->readNextString(buff);
		pComp->setObjectName(String(buff));

		ZELOG(LOG_GAME, Log, "Read component in scene with name \"%s\"", buff);

		m_componentMap.put(buff, compHandle);

		// BEGIN
		fileReader->readNextString(buff);

		fileReader->readNextString(buff);
		while (StringFunc::Compare(buff, "END") != 0)
		{
			if (StringFunc::Compare(buff, "R") == 0)
			{
				// Read Rotation
				Float32 x, y, z;
				x = fileReader->readNextFloat();
				y = fileReader->readNextFloat();
				z = fileReader->readNextFloat();

				Vector3 rotationInDeg(x, y, z);
				pComp->rotateInDeg(rotationInDeg);
			}
			else if (StringFunc::Compare(buff, "S") == 0)
			{
				// Read Scale
				Vector3 scale;
				scale.m_x = fileReader->readNextFloat();
				scale.m_y = fileReader->readNextFloat();
				scale.m_z = fileReader->readNextFloat();

				pComp->setScale(scale);
			}
			else if (StringFunc::Compare(buff, "T") == 0)
			{
				// Read Pos
				Vector3 pos;
				pos.m_x = fileReader->readNextFloat();
				pos.m_y = fileReader->readNextFloat();
				pos.m_z = fileReader->readNextFloat();

				pComp->setWorldPosition(pos);
			}
			else if (StringFunc::Compare(buff, "Mesh") == 0)
			{
				// Read Mesh
				fileReader->readNextString(buff);
				if (RenderComponent* pRendComp = (RenderComponent*) pComp)
				{
					pRendComp->loadMeshFromFile(GetResourcePath(buff).c_str());
				}
			}
			else if (StringFunc::Compare(buff, "Material") == 0)
			{
				// Read material
				fileReader->readNextString(buff);
				if (RenderComponent* pRendComp = (RenderComponent*)pComp)
				{
					Handle hMaterial = MaterialManager::GetInstance()->loadResource(GetResourcePath(buff).c_str());
					pRendComp->setMaterial(hMaterial.getObject<Material>());
				}
			}
			else if (StringFunc::Compare(buff, "LightType") == 0)
			{
				// Read Light Type
				int lightType = fileReader->readNextInt();
				if (LightComponent* pLightComp = (LightComponent*)pComp)
				{
					pLightComp->m_lightType = (LightType)lightType;
				}
			}
			else if (StringFunc::Compare(buff, "Att_Distance") == 0)
			{
				// Read the constant term for Attenuation Equation
				float val = fileReader->readNextFloat();
				if (LightComponent* pLightComp = (LightComponent*)pComp)
				{
					pLightComp->m_attDistance = val;
				}
			}
			else if (StringFunc::Compare(buff, "Att_Constant") == 0)
			{
				// Read the constant term for Attenuation Equation
				float val = fileReader->readNextFloat();
				if (LightComponent* pLightComp = (LightComponent*)pComp)
				{
					pLightComp->m_attConstant = val;
				}
			}
			else if (StringFunc::Compare(buff, "Att_Linear") == 0)
			{
				// Read the linear term for Attenuation Equation
				float val = fileReader->readNextFloat();
				if (LightComponent* pLightComp = (LightComponent*)pComp)
				{
					pLightComp->m_attLinear = val;
				}
			}
			else if (StringFunc::Compare(buff, "Att_Quadratic") == 0)
			{
				// Read the quadratic term for Attenuation Equation
				float val = fileReader->readNextFloat();
				if (LightComponent* pLightComp = (LightComponent*)pComp)
				{
					pLightComp->m_attQuadratic = val;
				}
			}
			else if (StringFunc::Compare(buff, "Children") == 0)
			{
				// Read Children of this comp
				Int32 count = fileReader->readNextInt();
				for (Int32 i = 0; i < count; i++)
				{
					loadSceneComponentToComp(fileReader, pComp);
				}
			}
			else if (StringFunc::Compare(buff, "Scene") == 0)
			{
				// Read Scene
				fileReader->readNextString(buff);
				loadSceneFileToComp(GetResourcePath(buff).c_str(), pComp);
			}
			else if (StringFunc::Compare(buff, "Physics") == 0)
			{
				fileReader->readNextString(buff);
				if (RenderComponent* pRendComp = dynamic_cast<RenderComponent*>(pComp))
				{
					pRendComp->setStatic(StringFunc::Compare(buff, "true") != 0);
					pRendComp->setPhysicsEnabled(StringFunc::Compare(buff, "true") == 0);
				}
			}
			else if (StringFunc::Compare(buff, "TriggerOnly") == 0)
			{
				fileReader->readNextString(buff);
				if (CollisionComponent* pCollisionComp = dynamic_cast<CollisionComponent*>(pComp))
				{
					pCollisionComp->setTrigger(StringFunc::Compare(buff, "true") == 0);
				}
			}
			else if (StringFunc::Compare(buff, "Extent") == 0)
			{
				if (BoxComponent* pBoxComponent = dynamic_cast<BoxComponent*>(pComp))
				{
					Vector3 halfExtent;
					halfExtent.setX(fileReader->readNextFloat());
					halfExtent.setY(fileReader->readNextFloat());
					halfExtent.setZ(fileReader->readNextFloat());
					pBoxComponent->setHalfExtent(halfExtent);
				}
			}
			else if (StringFunc::Compare(buff, "Radius") == 0)
			{
				if (SphereComponent* pSphereComp = dynamic_cast<SphereComponent*>(pComp))
				{
					pSphereComp->setRadius( fileReader->readNextFloat() );
				}
				else if (CapsuleComponent* pCapsuleComp = dynamic_cast<CapsuleComponent*>(pComp))
				{
					pCapsuleComp->setRadius( fileReader->readNextFloat() );
				}
				else if (LightComponent* pLightComponent = dynamic_cast<LightComponent*>(pComp))
				{
					pLightComponent->m_innerRadius = DegToRad(fileReader->readNextFloat());
				}
			}
			else if (StringFunc::Compare(buff, "OuterRadius") == 0)
			{
				if (LightComponent* pLightComponent = dynamic_cast<LightComponent*>(pComp))
				{
					pLightComponent->m_outerRadius = DegToRad(fileReader->readNextFloat());
				}
			}
			else if (StringFunc::Compare(buff, "Height") == 0)
			{
				if (CapsuleComponent* pCapsuleComp = dynamic_cast<CapsuleComponent*>(pComp))
				{
					pCapsuleComp->setHeight( fileReader->readNextFloat() );
				}
			}
			else if (StringFunc::Compare(buff, "Animation") == 0)
			{
				Handle hAnimationComp("Animation Component", sizeof(AnimationComponent));
				AnimationComponent* pAnimationComp = new(hAnimationComp) AnimationComponent(m_gameContext);
				pAnimationComp->setupComponent();
				
				// Read Animation path
				fileReader->readNextString(buff);
				Handle hAnimClip = AnimationManager::GetInstance()->loadResource(GetResourcePath(buff).c_str());
				bool bLoop = fileReader->readNextInt() == 1;
				float rate = fileReader->readNextFloat();

				if (hAnimClip.isValid())
				{
					AnimationClip* pAnimClip = hAnimClip.getObject<AnimationClip>();
					pAnimationComp->playAnimationClip(pAnimClip, bLoop, rate);
				}

				pComp->addChild(pAnimationComp);
			}
			else if (StringFunc::Compare(buff, "AnimationState") == 0)
			{
				Handle hAnimationSM("Animation SM", sizeof(AnimationSM));
				AnimationSM* pAnimSM = new(hAnimationSM) AnimationSM(m_gameContext);
				pAnimSM->setupComponent();

				// Read Animation State Def
				fileReader->readNextString(buff);
				pAnimSM->readAnimStateDef(GetResourcePath(buff).c_str());

				pComp->addChild(pAnimSM);
			}
			else if (StringFunc::Compare(buff, "Texture") == 0)
			{
				// Read Texture file
				fileReader->readNextString(buff);

				if (pComp->getClassID() == Skybox::GetClassID())
				{
					Skybox* pSkyBox = static_cast<Skybox*>(pComp);
					pSkyBox->loadFromPath(GetResourcePath(buff).c_str());
				}
			}

			fileReader->readNextString(buff);
		}

		pComp->setupComponent();
		parent->addChild(pComp);
	}

	ZE::Handle SceneManager::getCompByName(const char* name)
	{
		if (m_componentMap.hasKey(name))
		{
			return m_componentMap[name];
		}

		return Handle();
	}

	void SceneManager::Init(GameContext* _gameContext)
	{
		Handle handle("SceneManager", sizeof(SceneManager));
		s_instance = new(handle) SceneManager(_gameContext);
	}

	void SceneManager::Destroy()
	{

	}

}
