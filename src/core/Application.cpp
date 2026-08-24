#include "Application.h"
#include "Render/VulkanRenderer.h"
#include "GlfwWindow.h"
#include "ImGuiLayer.h"
#include "Render/VulkanUtil.h"
bool Application::init = false;
bool Application::Init()
{
	GlfwWindow::CreateInfo createInfo;
	uint32_t extCount = 0;
	window = new GlfwWindow();
	if (!window->Init(createInfo)) {
		return false;
	}
	auto extensions = window->GetRequiredVulkanExtensions(extCount);
	renderer = new VulkanRenderer();
	renderer->AddEnabledInstanceExtensions(extCount, extensions);
	if (!renderer->InitVulkan()) {
		return false;
	}
	auto surface = window->CreateSurface(renderer->instance);
	renderer->Init(surface);


	guiLayer = new ImGuiLayer();	

	init = true;
	return true;
}

void Application::Destroy()
{
	init = false;
	if(renderer)
		delete renderer;
	if (window)
		delete window;
	if (guiLayer)
		delete guiLayer;
	renderer = nullptr;
	window = nullptr;
	guiLayer = nullptr;
}

void Application::Resize(int width, int height)
{

}

bool Application::Run()
{
	//while (!window->ShouldClose())
	//{
	//	window->PollEvents();
	//	BeginFrame();
	//	UpdateScene();
	//	Simulate();
	//	Render();
	//	EndFrame();
	//}
	renderer->renderLoop();
	return true;
}

void Application::BeginFrame()
{

}

void Application::UpdateScene()
{

}

void Application::Simulate()
{

}

void Application::Render()
{

}

void Application::EndFrame()
{

}


