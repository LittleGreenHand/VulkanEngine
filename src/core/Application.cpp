#include "Application.h"
#include "FrameClock.h"
#include <thread>
bool Application::init = false;
bool Application::Init()
{
	// 初始化窗口
	{
		GlfwWindow::CreateInfo createInfo;
		window = std::make_unique<GlfwWindow>();
		if (!window->Init(createInfo)) {
			return false;
		}
	}
	// 初始化Vulkan与渲染器
	{
		uint32_t extCount = 0;
		auto extensions = window->GetRequiredVulkanExtensions(extCount);
		renderer = std::make_unique<VulkanRenderer>();		
		renderer->AddEnabledInstanceExtensions(extCount, extensions);
		if (!renderer->InitVulkan()) {
			return false;
		}
		auto surface = window->CreateSurface(renderer->instance);
		renderer->Init(surface);
	}

	//绑定输入回调
	{
		window->SetInputCallbacks({
			.key = [this](const WindowKeyEvent& event)
			{
				OnKey(event.key, event.scancode, event.action, event.mods);
			},
			.mouseButton = [this](const WindowMouseButtonEvent& event)
			{
				OnMouseButton(event.button, event.action, event.mods, event.cursorX, event.cursorY);
			},
			.cursorPosition = [this](const WindowCursorPositionEvent& event)
			{
				OnMouseMove(event.x, event.y);
			},
			.scroll = [this](const WindowScrollEvent& event)
			{
				OnScroll(event.xOffset, event.yOffset);
			},
			.framebufferResize = [this](const WindowFramebufferResizeEvent& event)
			{
				OnFramebufferResize(event.width, event.height);
			}
			});
	}

	// ImGUI
	{
		guiLayer = std::make_unique<ImGuiLayer>();
		guiLayer->Init(window->GetNativeWindow());
	}
	init = true;
	return true;
}

void Application::Destroy()
{
	guiLayer.reset();
	renderer.reset();
	window.reset();
	init = false;
}

void Application::Resize(int width, int height)
{

}

bool Application::Run()
{
	while (!window->ShouldClose())
	{
		if (!BeginFrame())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		UpdateScene();
		Simulate();
		Render();
		EndFrame();		
	}
	return true;
}

bool Application::BeginFrame()
{
	window->PollEvents();

	// 后缓冲延迟Resize，避免连续reszie
	{
		if (m_needResize)
		{
			static auto ResizeDelay = std::chrono::milliseconds(150);
			auto now = std::chrono::steady_clock::now();
			auto time = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastResizeTime);
			if (time > ResizeDelay)
			{
				renderer->OnFramebufferResize(window->GetFramebufferWidth(), window->GetFramebufferHeight());
				m_needResize = false;
			}
			else
				return false;
		}
	}

	if (window->ShouldClose())
		return false;

	FrameClock::Get().Tick();
	if (!renderer->BeginFrame(FrameClock::Get().DeltaSeconds()))
	{
		std::cout << "BeginFrame false" << std::endl;
		return false;
	}
	guiLayer->BeginFrame();
	return true;
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

void Application::OnKey(int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			renderer->camera.keys.up = false;
			break;
		case GLFW_KEY_S:
			renderer->camera.keys.down = false;
			break;
		case GLFW_KEY_A:
			renderer->camera.keys.left = false;
			break;
		case GLFW_KEY_D:
			renderer->camera.keys.right = false;
			break;
		case GLFW_KEY_Q:
			renderer->camera.keys.top = false;
			break;
		case GLFW_KEY_E:
			renderer->camera.keys.bottom = false;
			break;
		}
		return;
	}

	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_P:
			renderer->paused = !renderer->paused;
			break;
		case GLFW_KEY_F1:
			// Reserved for toggling the UI overlay.
			break;
		case GLFW_KEY_F2:
			renderer->camera.type = renderer->camera.type == Camera::CameraType::lookat
				? Camera::CameraType::firstperson
				: Camera::CameraType::lookat;
			break;
		}
	}

	if (renderer->camera.type == Camera::CameraType::firstperson)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			renderer->camera.keys.up = true;
			break;
		case GLFW_KEY_S:
			renderer->camera.keys.down = true;
			break;
		case GLFW_KEY_A:
			renderer->camera.keys.left = true;
			break;
		case GLFW_KEY_D:
			renderer->camera.keys.right = true;
			break;
		case GLFW_KEY_Q:
			renderer->camera.keys.top = true;
			break;
		case GLFW_KEY_E:
			renderer->camera.keys.bottom = true;
			break;
		}
	}
}

void Application::OnMouseButton(int button, int action, int mods, double cursorX, double cursorY)
{
	if (action == GLFW_PRESS)
		renderer->mouseState.position = glm::vec2(static_cast<float>(cursorX), static_cast<float>(cursorY));

	const bool pressed = action == GLFW_PRESS;
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		renderer->mouseState.buttons.left = pressed;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		renderer->mouseState.buttons.right = pressed;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		renderer->mouseState.buttons.middle = pressed;
		break;
	}
}

void Application::OnMouseMove(double x, double y)
{
	double dx = static_cast<double>(renderer->mouseState.position.x) - x;
	double dy = static_cast<double>(renderer->mouseState.position.y) - y;

	if (guiLayer->WantCaptureMouse()) {
		renderer->mouseState.position = glm::vec2((float)x, (float)y);
		return;
	}

	if (renderer->mouseState.buttons.left) {
		renderer->camera.rotate(glm::vec3(static_cast<float>(dy) * renderer->camera.rotationSpeed, -static_cast<float>(dx) * renderer->camera.rotationSpeed, 0.0f));
	}
	if (renderer->mouseState.buttons.right) {
		renderer->camera.Translate(glm::vec3(static_cast<float>(dx) * .005f, 0.0f, static_cast<float>(dy) * .005f));
	}
	if (renderer->mouseState.buttons.middle) {
		renderer->camera.Translate(glm::vec3(-static_cast<float>(dx) * 0.005f, -static_cast<float>(dy) * 0.005f, 0.0f));
	}
	renderer->mouseState.position = glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

void Application::OnScroll(double xOffset, double yOffset)
{
	auto camFront = renderer->camera.GetFront() * (static_cast<float>(-yOffset) * 120.0f * 0.005f);
	renderer->camera.Translate(camFront);
}

void Application::OnFramebufferResize(int framebufferWidth, int framebufferHeight)
{
	lastResizeTime = std::chrono::steady_clock::now();
	m_needResize = true;
}

