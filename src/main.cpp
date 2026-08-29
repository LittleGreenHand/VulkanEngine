#include "base/VulkanRendererBase.h"
#include "core/Application.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR, _In_ int)
{
	for (size_t i = 0; i < __argc; i++) { VulkanRendererBase::args.push_back(__argv[i]); };
	VulkanRendererBase::args.push_back("--validation");
	VulkanRendererBase::args.push_back("--vsync");
	VulkanRendererBase::args.push_back("--shaders");
	VulkanRendererBase::args.push_back("slang");
	VulkanRendererBase::args.push_back("--width");
	VulkanRendererBase::args.push_back("2560");
	VulkanRendererBase::args.push_back("--height");
	VulkanRendererBase::args.push_back("1440");
	VulkanRendererBase::args.push_back("--resourcepath");
	VulkanRendererBase::args.push_back(ENGINE_SOURCE_DIR);
	VulkanRendererBase::args.push_back("--shadersspvpath");
	VulkanRendererBase::args.push_back(SHADERS_SPV_DIR);

	auto& application = Application::GetInstance();
	if (application.Init())
	{
		application.Run();
	}
	else
	{
		MessageBoxA(nullptr, "Failed to initialize application!", "Error", MB_OK | MB_ICONERROR);
	}
	application.Destroy();
	return 0;
}