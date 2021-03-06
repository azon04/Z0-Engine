#ifndef __ZE_RENDER_PASS_H__
#define __ZE_RENDER_PASS_H__

#include "RenderGatherer.h"
#include "Utils/Array.h"

namespace ZE
{
	class IGPUFrameBuffer;
	class GameContext;

	class RenderPassBase
	{
	public:
		virtual void prepare(GameContext* _gameContext) = 0;
		virtual void release(GameContext* _gameContext) = 0;

		virtual bool execute_CPU(GameContext* _gameContext) = 0;
		virtual bool execute_GPU(GameContext* _gameContext) = 0;

		bool Execute(GameContext* _gameContext)
		{
			return execute_CPU(_gameContext) && execute_GPU(_gameContext);
		}
	};

	class RenderPass : public RenderPassBase
	{
	public:

		virtual void begin(GameContext* _gameContext);
		virtual void end(GameContext* _gameContext);

		void addInputTextureBuffer(IGPUTexture* textureBuffer) { m_textureBufferInputs.push_back(textureBuffer); }
		void addOutputTextureBuffer(IGPUTexture* textureBuffer) { m_textureBufferOutputs.push_back(textureBuffer); }

		void addInputFrameBuffer(IGPUFrameBuffer* textureBuffer) { m_frameBufferInputs.push_back(textureBuffer); }
		void addOutputFrameBuffer(IGPUFrameBuffer* textureBuffer) { m_frameBufferOuputs.push_back(textureBuffer); }

		IGPUTexture* getTextureOutput(Int32 index) { return m_textureBufferOutputs[index]; }
		Int32 getTextureOuputCount() const { return m_textureBufferOutputs.size(); }
		
		IGPUFrameBuffer* getFrameBufferOutput(Int32 index) { return m_frameBufferOuputs[index]; }
		Int32 getFrameBufferOutputCount() const { return m_textureBufferOutputs.size(); }

		virtual const char* getRenderPassName() const = 0;

	protected:
		Array<IGPUTexture*> m_textureBufferInputs;
		Array<IGPUTexture*> m_textureBufferOutputs;

		Array<IGPUFrameBuffer*> m_frameBufferInputs;
		Array<IGPUFrameBuffer*> m_frameBufferOuputs;
	};
}
#endif
