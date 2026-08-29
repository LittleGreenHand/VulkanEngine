#include "Application.h"
#include "Render/VulkanRenderer.h"
#include "GlfwWindow.h"
#include "ImGuiLayer.h"
#include "Render/VulkanContext.h"
#include "FrameClock.h"
bool Application::init = false;
bool Application::Init()
{
	// 初始化窗口
	{
		GlfwWindow::CreateInfo createInfo;
		window = new GlfwWindow();
		if (!window->Init(createInfo)) {
			return false;
		}
	}
	// 初始化Vulkan与渲染器
	{
		uint32_t extCount = 0;
		auto extensions = window->GetRequiredVulkanExtensions(extCount);
		renderer = new VulkanRenderer();
		renderer->AddEnabledInstanceExtensions(extCount, extensions);
		if (!renderer->InitVulkan()) {
			return false;
		}
		auto surface = window->CreateSurface(renderer->instance);
		renderer->Init(surface);
	}

	// ImGUI
	{
		guiLayer = new ImGuiLayer();
		guiLayer->Init(window->GetNativeWindow());
	}
	init = true;
	return true;
}

void Application::Destroy()
{
	if (!init)
		return;

	init = false;
	if (guiLayer)
		delete guiLayer;
	if (window)
		delete window;
	if(renderer)
		delete renderer;
	renderer = nullptr;
	window = nullptr;
	guiLayer = nullptr;
}

void Application::Resize(int width, int height)
{

}

bool Application::Run()
{
	while (!window->ShouldClose())
	{
		BeginFrame();
		UpdateScene();
		Simulate();
		Render();
		EndFrame();		
	}
	return true;
}

void Application::BeginFrame()
{
	window->PollEvents();
	FrameClock::GetInstance().Tick();
	renderer->BeginFrame(FrameClock::GetInstance().DeltaSeconds());
	guiLayer->BeginFrame();
}

void Application::UpdateScene()
{
	guiLayer->Update();
}

void Application::Simulate()
{

}

void Application::Render()
{
	renderer->render();
	guiLayer->Render();
}
  
void Application::EndFrame()
{
	renderer->EndFrame();
}


