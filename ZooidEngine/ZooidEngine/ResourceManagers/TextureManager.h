#ifndef __ZE_TEXTURE_MANAGER_H__
#define __ZE_TEXTURE_MANAGER_H__

#include "ResourceManager.h"

namespace ZE 
{
	class TextureManager : public ResourceManager
	{

	public:
		
		static void Init();
		static TextureManager* getInstance();
		static void Destroy();

		virtual Handle loadResource(const char* resourceFilePath);
		virtual void preUnloadResource(Resource* _resource);

	protected:

		static TextureManager* s_instance;
	};
};
#endif
