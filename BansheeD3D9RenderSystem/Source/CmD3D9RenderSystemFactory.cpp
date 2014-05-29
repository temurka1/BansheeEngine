#include "CmD3D9RenderSystemFactory.h"
#include "CmRenderSystem.h"

namespace BansheeEngine
{
	void D3D9RenderSystemFactory::create()
	{
	#ifdef BS_STATIC_LIB
		HINSTANCE hInst = GetModuleHandle(NULL);
	#else
		HINSTANCE hInst = GetModuleHandle("BansheeD3D9RenderSystem.dll");
	#endif

		RenderSystem::startUp<D3D9RenderSystem>(hInst);
	}

	D3D9RenderSystemFactory::InitOnStart D3D9RenderSystemFactory::initOnStart;
}