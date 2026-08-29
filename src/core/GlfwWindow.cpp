#include "GlfwWindow.h"

#include <stdexcept>
#include <utility>

GlfwWindow::~GlfwWindow()
{
	Shutdown();
}

bool GlfwWindow::Init(const CreateInfo& createInfo)
{
	if (m_window)
		return true;

	if (glfwInit() != GLFW_TRUE)
		return false;

	if (glfwVulkanSupported() != GLFW_TRUE)
	{
		glfwTerminate();
		return false;
	}

	//不需要创建 OpenGL Context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_MAXIMIZED, createInfo.maximized ? GLFW_TRUE : GLFW_FALSE);

	m_window = glfwCreateWindow(
		static_cast<int>(createInfo.width),
		static_cast<int>(createInfo.height),
		createInfo.title.c_str(),
		nullptr,
		nullptr);

	if (!m_window)
	{
		glfwTerminate();
		return false;
	}

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
	glfwSetKeyCallback(m_window, GlfwWindow::KeyCallback);

	// 在高 DPI 屏幕上，framebuffer size 可能和 window size 不一样。
	int framebufferWidth = 0;
	int framebufferHeight = 0;

	glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

	m_width = static_cast<uint32_t>(framebufferWidth);
	m_height = static_cast<uint32_t>(framebufferHeight);

	return true;
}

void GlfwWindow::Shutdown()
{
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();

	m_width = 0;
	m_height = 0;
}

void GlfwWindow::PollEvents() const
{
	glfwPollEvents();
}

void GlfwWindow::WaitEvents() const
{
	glfwWaitEvents();
}

bool GlfwWindow::ShouldClose() const
{
	return m_window == nullptr ||
		glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void GlfwWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

float GlfwWindow::GetAspectRatio() const
{
	if (m_height == 0)
		return 0.0f;

	return static_cast<float>(m_width) / static_cast<float>(m_height);
}

VkSurfaceKHR GlfwWindow::CreateSurface(VkInstance instance) const
{
	if (!m_window)
		throw std::runtime_error("Cannot create Vulkan surface: GLFW window is null.");

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	const VkResult result = glfwCreateWindowSurface(instance, m_window, nullptr, &surface);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan window surface.");

	return surface;
}

const char** GlfwWindow::GetRequiredVulkanExtensions(uint32_t& extensionCount)
{
	const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

	if (!extensions)
	{
		extensionCount = 0;
		throw std::runtime_error("Failed to get required Vulkan extensions from GLFW.");
	}

	return extensions;
}

void GlfwWindow::FramebufferResizeCallback(	GLFWwindow* window, int width, int height)
{
	auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

	if (!self)
		return;

	self->m_width = static_cast<uint32_t>(width);
	self->m_height = static_cast<uint32_t>(height);
}