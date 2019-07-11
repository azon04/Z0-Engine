#include "LightComponent.h"

#include "ZEGameContext.h"
#include "Events/Events.h"
#include "Renderer/ShaderData.h"
#include "Renderer/IRenderer.h"
#include "Renderer/DrawList.h"
#include "Renderer/RenderZooid.h"
#include "Renderer/IGPUFrameBuffer.h"
#include "Renderer/IGPUTexture.h"
#include "Renderer/DebugRenderer.h"

#include "Math/MathUtil.h"
#include "Math/MathOps.h"

#include "ResourceManagers/TextureManager.h"
#include "ResourceManagers/ShaderManager.h"
#include "Scene/CameraManager.h"
#include "Scene/CameraComponent.h"

namespace ZE
{
	IMPLEMENT_CLASS_1(LightComponent, SceneComponent)

		LightComponent::LightComponent(GameContext* gameContext) :
		SceneComponent(gameContext),
		m_lightType(DIRECTIONAL_LIGHT),
		m_diffuse(1.0f, 1.0f, 1.0f),
		m_specular(0.5f, 0.5f, 0.5f),
		m_ambient(0.1f, 0.1f, 0.1f),
		m_attDistance(7.0f),
		m_attConstant(1.0f),
		m_attLinear(0.07f),
		m_attQuadratic(0.0017f),
		m_bGenerateShadow(true),
		m_innerRadius(DegToRad(17.0f)),
		m_outerRadius(DegToRad(20.0f)),
		m_shadowMapWidth(1024),
		m_shadowMapHeight(1024)
	{
		for (UInt32 i = 0; i < MAX_LIGHT_SHADOW_MAPS; i++)
		{
			m_dynamicShadowFrameBuffers[i] = nullptr;
			m_dynamicShadowTextures[i] = nullptr;
		}
	}

	LightComponent::LightComponent(GameContext* gameContext, LightType _lightType) : 
		LightComponent(gameContext)
	{
		m_lightType = _lightType;
	}

	void LightComponent::setupComponent()
	{
		addEventDelegate(Event_GATHER_LIGHT, &LightComponent::handleGatherLight);
		
		m_bLightSetup = false;
		setupLight();
	}

	void LightComponent::handleGatherLight(Event* event)
	{
		LightData& lightData = m_gameContext->getDrawList()->m_lightData;
		Int32 index = lightData.NumLight++;
		LightStruct& light = lightData.lights[index];
		light.reset();
		light.Type = m_lightType;

		light.setAmbient(m_ambient);
		light.setDiffuse(m_diffuse);
		light.setSpecular(m_specular);

		switch (m_lightType)
		{
		case ZE::DIRECTIONAL_LIGHT:
			light.setDirection(m_worldTransform.getN());
			if (m_bGenerateShadow) { setupShadowMapsDirectional(index); }
			break;
		case ZE::POINT_LIGHT:
			light.setPosition(m_worldTransform.getPos());
			light.Att_constant = m_attConstant;
			light.Att_linear = m_attLinear;
			light.Att_quadratic = m_attQuadratic;
			if (m_bGenerateShadow) { setupShadowMapsPointLight(index); }
			break;
		case ZE::SPOT_LIGHT:
			light.setPosition(m_worldTransform.getPos());
			light.setDirection(m_worldTransform.getN());
			light.Att_constant = m_attConstant;
			light.Att_linear = m_attLinear;
			light.Att_quadratic = m_attQuadratic;
			light.CutOff = cos(m_innerRadius);
			light.OuterCutOff = cos(m_outerRadius);
			if (m_bGenerateShadow) { setupShadowMapsSpotLight(index); }
			break;
		default:
			break;
		}

		DebugRenderer::DrawMatrixBasis(m_worldTransform);
	}

	void LightComponent::setStatic(bool bStatic)
	{
		m_bStatic = bStatic;
	}

	void LightComponent::setGenerateShadow(bool bGenerateShadow)
	{
		m_bGenerateShadow = bGenerateShadow;
	}

	void LightComponent::setStaticShadowTexture(const String& path)
	{
		m_staticShadowFile = path;
	}

	void LightComponent::setShadowResolution(UInt32 _resolution)
	{
		setShadowMapWidth(_resolution);
		setShadowMapHeight(_resolution);
	}

	void LightComponent::setShadowMapWidth(UInt32 _width)
	{
		m_shadowMapWidth = _width;
	}

	void LightComponent::setShadowMapHeight(UInt32 _height)
	{
		m_shadowMapHeight = _height;
	}

	ZE::IGPUFrameBuffer* LightComponent::getOrCreateShadowFrameBuffer(UInt32 _index)
	{
		if (m_dynamicShadowFrameBuffers[_index])
		{
			return m_dynamicShadowFrameBuffers[_index];
		}

		ScopedRenderThreadOwnership renderLock(m_gameContext->getRenderer());

		// Setup Dynamic Texture
		Handle hTexture("Texture", sizeof(Texture));
		Texture* pCPUTexture = new(hTexture) Texture();
		pCPUTexture->createEmpty(m_shadowMapWidth, m_shadowMapHeight, 1, TEX_DEPTH);
		pCPUTexture->setGenerateMipmap(false);
		pCPUTexture->setDataType(FLOAT);
		pCPUTexture->setWrapOnU(CLAMP_TO_BORDER);
		pCPUTexture->setWrapOnV(CLAMP_TO_BORDER);
		pCPUTexture->setMinFilter(LINEAR);
		pCPUTexture->setMagFilter(LINEAR);
		pCPUTexture->setBorderColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

		Handle hGPUTexture = m_gameContext->getRenderZooid()->CreateRenderTexture();
		if (hGPUTexture.isValid())
		{
			m_dynamicShadowTextures[_index] = hGPUTexture.getObject<IGPUTexture>();
			m_dynamicShadowTextures[_index]->fromTexture(pCPUTexture);
		}

		// Setup Frame Buffer
		Handle hFrameBuffer = m_gameContext->getRenderZooid()->CreateFrameBuffer();
		if (hFrameBuffer.isValid())
		{
			m_dynamicShadowFrameBuffers[_index] = hFrameBuffer.getObject<IGPUFrameBuffer>();
			m_dynamicShadowFrameBuffers[_index]->bind();
			m_dynamicShadowFrameBuffers[_index]->addTextureAttachment(DEPTH_ATTACHMENT, m_dynamicShadowTextures[_index]);
			m_dynamicShadowFrameBuffers[_index]->setupAttachments();
			m_dynamicShadowFrameBuffers[_index]->unbind();
		}

		return m_dynamicShadowFrameBuffers[_index];
	}

	void LightComponent::setupLight()
	{
		if (m_bLightSetup && !m_bGenerateShadow)
		{
			if (m_staticShadowTexture && m_staticShadowFile.length() > 0)
			{
				TextureManager::GetInstance()->unloadResource(m_staticShadowFile.const_str());
				m_staticShadowTexture = nullptr;
			}
			return;
		}

		if (!m_bGenerateShadow) { return; }

		m_staticShadowTexture = nullptr;

		if (m_staticShadowFile.length() > 0)
		{
			Handle hTexture = TextureManager::GetInstance()->loadResource(m_staticShadowFile.const_str());
			if (hTexture.isValid())
			{
				m_staticShadowTexture = hTexture.getObject<IGPUTexture>();
			}
		}

		m_bLightSetup = true;
	}

	// view: before transpose
	void LightComponent::calculateCascadeLightFustrum(Matrix4x4& view, Matrix4x4& projection, ViewFustrum* camFustrum, Float32 cascadeDistStart, Float32 cascadeDistEnd, 
		Float32 obMostRight, Float32 obMostLeft, Float32 obMostTop, Float32 obMostBottom, Float32 obMostNear)
	{
		Vector3 xAxis = view.getU();
		Vector3 yAxis = view.getV();
		Vector3 zAxis = view.getN();

		view = view.transpose();

		Float32 mostRight, mostLeft, mostTop, mostBottom, mostNear, mostFar;

		Vector3 fustrumPoints[8];

		fustrumPoints[FP_NTL] = camFustrum->getFustrumPoint(FP_NTL) + (cascadeDistStart * (camFustrum->getFustrumPoint(FP_FTL) - camFustrum->getFustrumPoint(FP_NTL)));
		fustrumPoints[FP_FTL] = camFustrum->getFustrumPoint(FP_NTL) + (cascadeDistEnd * (camFustrum->getFustrumPoint(FP_FTL) - camFustrum->getFustrumPoint(FP_NTL)));

		fustrumPoints[FP_NTR] = camFustrum->getFustrumPoint(FP_NTR) + (cascadeDistStart * (camFustrum->getFustrumPoint(FP_FTR) - camFustrum->getFustrumPoint(FP_NTR)));
		fustrumPoints[FP_FTR] = camFustrum->getFustrumPoint(FP_NTR) + (cascadeDistEnd * (camFustrum->getFustrumPoint(FP_FTR) - camFustrum->getFustrumPoint(FP_NTR)));

		fustrumPoints[FP_NBR] = camFustrum->getFustrumPoint(FP_NBR) + (cascadeDistStart * (camFustrum->getFustrumPoint(FP_FBR) - camFustrum->getFustrumPoint(FP_NBR)));
		fustrumPoints[FP_FBR] = camFustrum->getFustrumPoint(FP_NBR) + (cascadeDistEnd * (camFustrum->getFustrumPoint(FP_FBR) - camFustrum->getFustrumPoint(FP_NBR)));

		fustrumPoints[FP_NBL] = camFustrum->getFustrumPoint(FP_NBL) + (cascadeDistStart * (camFustrum->getFustrumPoint(FP_FBL) - camFustrum->getFustrumPoint(FP_NBL)));
		fustrumPoints[FP_FBL] = camFustrum->getFustrumPoint(FP_NBL) + (cascadeDistEnd * (camFustrum->getFustrumPoint(FP_FBL) - camFustrum->getFustrumPoint(FP_NBL)));

		for (int i = 0; i < 8; i++)
		{
			float right = xAxis | fustrumPoints[i];
			float top = yAxis | fustrumPoints[i];
			float nearD = zAxis | fustrumPoints[i];

			if (i == 0)
			{
				mostRight = mostLeft = right;
				mostTop = mostBottom = top;
				mostNear = mostFar = nearD;
			}
			else
			{
				if (mostRight < right) { mostRight = right; }
				if (mostLeft > right) { mostLeft = right; }
				if (mostTop < top) { mostTop = top; }
				if (mostBottom > top) { mostBottom = top; }
				if (mostNear < nearD) { mostNear = nearD; }
				if (mostFar > nearD) { mostFar = nearD; }
			}
		}

 		if (mostTop > obMostTop) { mostTop = obMostTop; }
 		if (mostBottom < obMostBottom) { mostBottom = obMostBottom; }
 		if (mostRight > obMostRight) { mostRight = obMostRight; }
 		if (mostLeft < obMostLeft) { mostLeft = obMostLeft; }
 		if (mostNear < obMostNear) { mostNear = obMostNear; }

		// Add a little bit buffer size to the depth texture to prevent some shadow hiccup/anomaly
		mostRight += 1.0f;
		mostLeft -= 1.0f;
		mostTop += 1.0f;
		mostBottom -= 1.0f;

		Vector3 deltaPos((mostRight + mostLeft) * 0.5f, (mostTop + mostBottom) * 0.5f, mostNear);

 		mostRight -= deltaPos.getX();
 		mostLeft -= deltaPos.getX();
 		mostTop -= deltaPos.getY();
 		mostBottom -= deltaPos.getY();

		Vector3 newPos;
		newPos = xAxis * deltaPos.getX();
		newPos = newPos + yAxis * deltaPos.getY();
		newPos = newPos + zAxis * deltaPos.getZ();
		view.setPos(Vector3(xAxis | newPos * -1.0f, yAxis | newPos * -1.0f, zAxis | newPos * -1.0f));

		MathOps::CreateOrthoProjEx(projection, mostBottom, mostTop, mostLeft, mostRight, 0.0f, mostNear - mostFar);
	}

	void LightComponent::setupShadowMapsDirectional(UInt32 lightIndex)
	{
		LightStruct& lightData = m_gameContext->getDrawList()->m_lightData.lights[lightIndex];
		DrawList* drawList = m_gameContext->getDrawList();

		Int32 numberOfCascade = 4;
		Float32 cascadedDistances[4] = { 0.025f, 0.05f, 0.25f, 1.0f };

		// TODO Determine how many cascade we need based on light and cam angle

		Matrix4x4 view, projection;

		Vector3 zAxis = lightData.getDirection();
		Vector3 xAxis = zAxis ^ Vector3(0.0f, 1.0f, 0.0f);
		if (xAxis.lengthSquare() == 0.0f)
		{
			xAxis = zAxis ^ Vector3(0.0f, 0.0f, 1.0f);
		}

		Vector3 yAxis = xAxis ^ zAxis;

		zAxis = zAxis * -1.0f;

		view.setU(xAxis);
		view.setV(yAxis);
		view.setN(zAxis);

		// Test Object bounding; make the light fustrum smaller
		float obMostRight = 0.0f;
		float obMostLeft = 0.0f;
		float obMostTop = 0.0f;
		float obMostBottom = 0.0f;
		float obMostNear = 0.0f;

		{
			Vector3 vertices[2] = { drawList->m_objectsBounding.m_min,  drawList->m_objectsBounding.m_max };
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					for (int k = 0; k < 2; k++)
					{
						Vector3 vertex(vertices[i].m_x, vertices[j].m_y, vertices[k].m_z);

						float right = xAxis | vertex;
						float top = yAxis | vertex;
						float nearD = zAxis | vertex;

						if (i == 0 && j == 0 && k == 0)
						{
							obMostRight = obMostLeft = right;
							obMostTop = obMostBottom = top;
						}
						else
						{
							if (obMostRight < right) { obMostRight = right; }
							if (obMostLeft > right) { obMostLeft = right; }
							if (obMostTop < top) { obMostTop = top; }
							if (obMostBottom > top) { obMostBottom = top; }
							if (obMostNear < nearD) { obMostNear = nearD; }
						}
					}
				}
			}
		}

// 		if (numberOfCascade == 1) // No cascade at all
// 		{
// 			calculateCascadeLightFustrum(view, projection, &(drawList->m_viewFustrum), 0.1f, 1.0f, obMostRight, obMostLeft, obMostTop, obMostBottom, obMostNear);
// 			
// 			lightData.setViewProjMatrix( view*projection );
// 
// 			// Setup Shadow Map Data
// 			LightShadowMapData& shadowMapData = m_gameContext->getDrawList()->getNextLightShadowMapData();
// 			shadowMapData.dynamicShadowFrameBuffer = getOrCreateShadowFrameBuffer(0);
// 			shadowMapData.dynamicShadowTexture = getDynamicShadowMap(0);
// 			shadowMapData.staticShadowTexture = m_staticShadowTexture;
// 			shadowMapData.lightIndex = lightIndex;
// 			shadowMapData.cascadeIndex = -1;
// 			shadowMapData.normalShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_CHAIN_SHADOW_DEPTH);
// 			shadowMapData.skinnedShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_cHAIN_SHADOW_DEPTH_SKINNED);
// 			shadowMapData.view = view;
// 			shadowMapData.projection = projection;
// 
// 		}
// 		else
		{
			{
				LightShadowMapData& shadowMapData = m_gameContext->getDrawList()->getNextLightShadowMapData();
				shadowMapData.dynamicShadowFrameBuffer = nullptr;
				shadowMapData.dynamicShadowTexture = nullptr;
				shadowMapData.staticShadowTexture = m_staticShadowTexture;
				shadowMapData.lightIndex = lightIndex;
				shadowMapData.cascadeIndex = -1;
				shadowMapData.normalShaderChain = nullptr;
				shadowMapData.skinnedShaderChain = nullptr;
			}
			

			float currentCascadeStart = 0.0f;
			float cameraRange = CameraManager::GetInstance()->getCurrentCamera()->m_far;// -CameraManager::GetInstance()->getCurrentCamera()->m_near;

			for (int i = 0; i < numberOfCascade; i++)
			{
				Matrix4x4 localView = view;
				int cascadeIndex = m_gameContext->getDrawList()->m_lightData.NumCascade++;
				CascadeShadowData& cascadeData = m_gameContext->getDrawList()->m_lightData.cascadeShadowData[cascadeIndex];
				
				calculateCascadeLightFustrum(localView, projection, &(drawList->m_viewFustrum), currentCascadeStart, cascadedDistances[i], 
					obMostRight, obMostLeft, obMostTop, obMostBottom, obMostNear);

				cascadeData.cascadeDistance = cascadedDistances[i] * cameraRange;
				currentCascadeStart = cascadedDistances[i] - 0.0025f;
				
				// Setup Shadow Map Data
				LightShadowMapData& shadowMapData = m_gameContext->getDrawList()->getNextLightShadowMapData();
				shadowMapData.dynamicShadowFrameBuffer = getOrCreateShadowFrameBuffer(i);
				shadowMapData.dynamicShadowTexture = getDynamicShadowMap(i);
				shadowMapData.staticShadowTexture = nullptr;
				shadowMapData.lightIndex = lightIndex;
				shadowMapData.cascadeIndex = cascadeIndex;
				shadowMapData.normalShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_CHAIN_SHADOW_DEPTH);
				shadowMapData.skinnedShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_cHAIN_SHADOW_DEPTH_SKINNED);
				shadowMapData.view = localView;
				shadowMapData.projection = projection;

				lightData.cascadeShadowIndices[i] = cascadeIndex;
			}
		}
	}

	void LightComponent::setupShadowMapsSpotLight(UInt32 lightIndex)
	{
		LightStruct& lightData = m_gameContext->getDrawList()->m_lightData.lights[lightIndex];
		Matrix4x4 view, projection;

		MathOps::LookAt(view, lightData.getPosition(), lightData.getPosition() + lightData.getDirection(), Vector3(0.0f, 1.0f, 0.0f));
		MathOps::CreatePerspectiveProjEx(projection, 1.0f, 2.0f * RadToDeg(acosf(lightData.OuterCutOff)), 0.1f, 10.0f);

		// Setup Shadow Map Data
		LightShadowMapData& shadowMapData = m_gameContext->getDrawList()->getNextLightShadowMapData();
		shadowMapData.dynamicShadowFrameBuffer = getOrCreateShadowFrameBuffer(0);
		shadowMapData.dynamicShadowTexture = getDynamicShadowMap(0);
		shadowMapData.staticShadowTexture = m_staticShadowTexture;
		shadowMapData.lightIndex = lightIndex;
		shadowMapData.cascadeIndex = -1;
		shadowMapData.normalShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_CHAIN_SHADOW_DEPTH);
		shadowMapData.skinnedShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_cHAIN_SHADOW_DEPTH_SKINNED);
		shadowMapData.view = view;
		shadowMapData.projection = projection;
	}

	void LightComponent::setupShadowMapsPointLight(UInt32 lightIndex)
	{
		LightStruct& lightData = m_gameContext->getDrawList()->m_lightData.lights[lightIndex];

		static Vector3 directions[6] = {
			Vector3(0.0, 1.0f, 0.0f),
			Vector3(0.0f, -1.0f, 0.0f),
			Vector3(0.0f, 0.0f, 1.0f),
			Vector3(0.0f, 0.0f, -1.0f),
			Vector3(1.0f, 0.0f, 0.0f),
			Vector3(-1.0f, 0.0f, 0.0f)
		};

		for(UInt32 i = 0; i < 6; i++)
		{
			Matrix4x4 view, projection;

			MathOps::LookAt(view, lightData.getPosition(), lightData.getPosition() + directions[i], Vector3(0.0f, 1.0f, 0.0f));
			MathOps::CreatePerspectiveProjEx(projection, 1.0f, RadToDeg(45.0f), 0.01f, m_attDistance);

			// Setup Shadow Map Data
			LightShadowMapData& shadowMapData = m_gameContext->getDrawList()->getNextLightShadowMapData();
			shadowMapData.dynamicShadowFrameBuffer = getOrCreateShadowFrameBuffer(i);
			shadowMapData.dynamicShadowTexture = getDynamicShadowMap(i);
			shadowMapData.staticShadowTexture = nullptr;
			shadowMapData.lightIndex = lightIndex;
			shadowMapData.cascadeIndex = -1;
			shadowMapData.normalShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_CHAIN_SHADOW_DEPTH);
			shadowMapData.skinnedShaderChain = ShaderManager::GetInstance()->getShaderChain(Z_SHADER_cHAIN_SHADOW_DEPTH_SKINNED);
			shadowMapData.view = view;
			shadowMapData.projection = projection;
		}
	}

}