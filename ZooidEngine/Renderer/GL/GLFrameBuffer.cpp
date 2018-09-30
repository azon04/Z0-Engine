#include "GLFrameBuffer.h"

#include "GLTexture.h"
#include "GLRenderBuffer.h"
#include "Enums.h"

namespace ZE
{

	GLFrameBuffer::GLFrameBuffer()
		: m_bColorAttached(false),
		m_fbo(0)
	{

	}

	void GLFrameBuffer::create()
	{
		IGPUFrameBuffer::create();

		glGenFramebuffers(1, &m_fbo);
	}

	void GLFrameBuffer::addTextureAttachment(EFrameBuferAttachmentType attachType, IGPUTexture* texture, UInt32 attachIndex /*= 0*/)
	{
		GLenum _glAttachType = getGLAttachmentType(attachType, attachIndex);

		GLTexture* pRealTexture = (GLTexture*)texture;
		glFramebufferTexture2D(GL_FRAMEBUFFER, _glAttachType, GL_TEXTURE_2D, pRealTexture->getTextureBuffer(), 0);
		
		m_width = pRealTexture->getWidth();
		m_height = pRealTexture->getHeight();

		if (_glAttachType >= GL_COLOR_ATTACHMENT0 && _glAttachType <= GL_COLOR_ATTACHMENT0 + GL_MAX_COLOR_ATTACHMENTS - 1)
		{
			m_bColorAttached = true;
		}
	}

	void GLFrameBuffer::addRenderBufferAttachment(EFrameBuferAttachmentType attachType, IGPURenderBuffer* renderBuffer, UInt32 attachIndex /*= 0*/)
	{
		GLenum _glAttachType = getGLAttachmentType(attachType, attachIndex);

		GLRenderBuffer* pRealRenderBuffer = (GLRenderBuffer*)renderBuffer;
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, _glAttachType, GL_RENDERBUFFER, pRealRenderBuffer->getRenderBufferObject());

		if (_glAttachType >= GL_COLOR_ATTACHMENT0 && _glAttachType <= GL_COLOR_ATTACHMENT0 + GL_MAX_COLOR_ATTACHMENTS - 1)
		{
			m_bColorAttached = true;
		}
	}

	void GLFrameBuffer::setupAttachments()
	{
		if (!m_bColorAttached)
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		ZASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete. Try to attach all necessasy attachments.");

	}

	void GLFrameBuffer::release()
	{
		IGPUFrameBuffer::release();
		if (m_fbo > 0)
		{
			glDeleteFramebuffers(1, &m_fbo);
			m_fbo = 0;
		}
	}

	void GLFrameBuffer::bind()
	{
		if (m_fbo > 0)
		{
			glViewport(0, 0, m_width, m_height);
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		}
	}

	void GLFrameBuffer::unbind()
	{
		if (m_fbo > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

}