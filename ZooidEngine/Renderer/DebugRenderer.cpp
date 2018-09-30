#include "DebugRenderer.h"

#include "Memory/Handle.h"
#include "BufferData.h"

#include "ResourceManagers/BufferManager.h"
#include "Renderer/IGPUBufferArray.h"

#include "Events/Events.h"

#include "Renderer/ShaderAction.h"
#include "ZEGameContext.h"
#include "Renderer/DrawList.h"
#include "ResourceManagers/ShaderManager.h"
#include "Scene/TextComponent.h"

namespace ZE
{
	IMPLEMENT_CLASS_1(DebugRenderer, Component)

	void DebugRenderer::Init(GameContext* gameContext)
	{
		gameContext->getRenderer()->AcquireRenderThreadOwnership();

		Handle hDebugRenderer("Debug Renderer", sizeof(DebugRenderer));
		s_instance = new(hDebugRenderer) DebugRenderer(gameContext);

		Handle hBufferData("Buffer Data", sizeof(BufferData));
		s_instance->m_lineBufferData = new(hBufferData) BufferData(VERTEX_BUFFER);
		s_instance->m_lineBufferData->setBufferLayout( BUFFER_LAYOUT_V3_C3);
		s_instance->m_lineBufferData->SetData(s_instance->m_lineBuffers, sizeof(DebugPointStruct), MAX_LINE * 2);
		s_instance->m_lineBufferData->setStaticBuffer(false);
		s_instance->m_lineGPUBufferData = BufferManager::getInstance()->createGPUBufferFromBuffer(s_instance->m_lineBufferData, false);
		
		Handle hBufferArray = BufferManager::getInstance()->createBufferArray(s_instance->m_lineGPUBufferData, nullptr, nullptr);
		s_instance->m_lineBufferArray = hBufferArray.getObject<IGPUBufferArray>();

		gameContext->getRenderer()->ReleaseRenderThreadOwnership();

		s_instance->m_currentIndex = 0;
		s_instance->m_currentTextIndex = 0;

		s_instance->setupComponent();

		MemoryHelper::Zero(s_instance->m_textComponents, sizeof(s_instance->m_textComponents));
	}

	void DebugRenderer::Destroy()
	{

	}

	void DebugRenderer::DrawMatrixBasis(Matrix4x4& mats)
	{
		s_instance->drawMatrixBasis(mats);
	}

	void DebugRenderer::DrawLine(const Vector3& p1, const Vector3& p2, const Vector3& color)
	{
		s_instance->drawLine(p1, p2, color);
	}

	void DebugRenderer::DrawTextScreen(const char* text, const Vector2& _position, const Vector3& _color, Float32 _scale)
	{
		s_instance->drawScreenText(text, _position, _color, _scale);
	}

	void DebugRenderer::DrawTextWorld(const char* text, Matrix4x4& _transform)
	{
		s_instance->drawWorldText(text, _transform);
	}

	void DebugRenderer::setupComponent()
	{
		Component::setupComponent();

		addEventDelegate(Event_GATHER_RENDER, &DebugRenderer::handleGatherRender);
	}

	void DebugRenderer::handleGatherRender(Event* _event)
	{
		if (m_currentIndex > 0)
		{
			ShaderAction& shaderAction = m_gameContext->getDrawList()->getNextShaderAction();

			shaderAction.setShaderAndBuffer(ShaderManager::GetInstance()->getShaderChain(Z_SHADER_CHAIN_3D_DEFAULT_COLOR_LINE), m_lineBufferArray);
			shaderAction.setVertexSize(m_currentIndex);

			shaderAction.setShaderMatVar("modelMat", Matrix4x4());
			shaderAction.setConstantsBlockBuffer("shader_data", m_gameContext->getDrawList()->m_mainConstantBuffer);

			// Update Line data
			m_gameContext->getRenderer()->AcquireRenderThreadOwnership();

			m_lineGPUBufferData->refresh();
			
			m_gameContext->getRenderer()->ReleaseRenderThreadOwnership();
		}

		while (m_textComponents[m_currentTextIndex] != NULL)
		{
			m_textComponents[m_currentTextIndex++]->setVisible(false);
		}

		// Reset
		m_currentIndex = 0;
		m_currentTextIndex = 0;
	}

	void DebugRenderer::drawMatrixBasis(Matrix4x4& mat)
	{
		Vector3& pos = mat.getPos();
		drawLine(pos, pos + mat.getU(), Vector3(1.0f, 0.0f, 0.0f));
		drawLine(pos, pos + mat.getV(), Vector3(0.0f, 1.0f, 0.0f));
		drawLine(pos, pos + mat.getN(), Vector3(0.0f, 0.0f, 1.0f));
	}

	void DebugRenderer::drawLine(const Vector3& p1, const Vector3& p2, const Vector3& color)
	{
		if (m_currentIndex < MAX_LINE * 2)
		{
			DebugPointStruct& p1Struct = m_lineBuffers[m_currentIndex++];
			p1Struct.Point = p1;
			p1Struct.Color = color;

			DebugPointStruct& p2Struct = m_lineBuffers[m_currentIndex++];
			p2Struct.Point = p2;
			p2Struct.Color = color;
		}
		else
		{
			ZEWARNING("Not Enough Debug Lines Data to Render.");
		}
	}

	void DebugRenderer::drawScreenText(const char* text, const Vector2& _position, const Vector3& _color, Float32 _scale)
	{
		if (m_textComponents[m_currentTextIndex] == NULL)
		{
			Handle hTextComponent("Text Component", sizeof(TextComponent));
			m_textComponents[m_currentTextIndex] = new(hTextComponent) TextComponent(m_gameContext);

			addChild(m_textComponents[m_currentTextIndex]);
			m_textComponents[m_currentTextIndex]->setupComponent();
		}

		TextComponent* textComponent = m_textComponents[m_currentTextIndex++];
		textComponent->setVisible(true);
		textComponent->setText(text);
		textComponent->setWorldPosition(Vector3(_position.getX(), _position.getY(), 0.0f));
		textComponent->setColor(_color);
		textComponent->setScale(Vector3(_scale, _scale, 1.0f));
		textComponent->setDrawSpace(DRAW_SCREEN);
	}

	void DebugRenderer::drawWorldText(const char* text, Matrix4x4& _transform)
	{
		if(m_textComponents[m_currentTextIndex] == NULL)
		{
			Handle hTextComponent("Text Component", sizeof(TextComponent));
			m_textComponents[m_currentTextIndex] = new(hTextComponent) TextComponent(m_gameContext);

			addChild(m_textComponents[m_currentTextIndex]);
			m_textComponents[m_currentTextIndex]->setupComponent();
		}

		TextComponent* textComponent = m_textComponents[m_currentTextIndex++];
		textComponent->setVisible(true);
		textComponent->setText(text);
		textComponent->setColor(Vector3(1.0f, 1.0f, 1.0f));
		textComponent->setWorldTransform(_transform);
		textComponent->setDrawSpace(DRAW_WORLD_SPACE);
	}

	ZE::DebugRenderer* DebugRenderer::s_instance = nullptr;

}