#include "VulkanEngine.h"

// OS specific main entry points
// Most of the code base is shared for the different supported operating systems, but stuff like message handling differs

// Windows entry point
VulkanEngine* vulkanEngineBase;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (vulkanEngineBase != NULL)
	{
		vulkanEngineBase->handleMessages(hWnd, uMsg, wParam, lParam);
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int)
{
	for (size_t i = 0; i < __argc; i++) { VulkanEngine::args.push_back(__argv[i]); };
	VulkanEngine::args.push_back("--validation");
	VulkanEngine::args.push_back("--vsync");
	VulkanEngine::args.push_back("--shaders");
	VulkanEngine::args.push_back("slang");
	VulkanEngine::args.push_back("--width");
	VulkanEngine::args.push_back("2560");
	VulkanEngine::args.push_back("--height");
	VulkanEngine::args.push_back("1440");
#if defined(ENGINE_SOURCE_DIR)
	VulkanEngine::args.push_back("--resourcepath");
	VulkanEngine::args.push_back(ENGINE_SOURCE_DIR);
#endif
#if defined(SHADERS_SPV_DIR)
	VulkanEngine::args.push_back("--shadersspvpath");
	VulkanEngine::args.push_back(SHADERS_SPV_DIR);
#endif

	vulkanEngineBase = new VulkanEngine();
	vulkanEngineBase->initVulkan();
	vulkanEngineBase->setupWindow(hInstance, WndProc);
	vulkanEngineBase->prepare();
	vulkanEngineBase->renderLoop();
	delete(vulkanEngineBase);
	return 0;
}