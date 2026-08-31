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
	glfwSetMouseButtonCallback(m_window, GlfwWindow::MouseButtonCallback);
	glfwSetCursorPosCallback(m_window, GlfwWindow::CursorPositionCallback);
	glfwSetScrollCallback(m_window, GlfwWindow::ScrollCallback);

	// 在高 DPI 屏幕上，framebuffer size 可能和 window size 不一样。
	int framebufferWidth = 0;
	int framebufferHeight = 0;

	glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

	m_framebufferWidth = framebufferWidth > 0 ? static_cast<uint32_t>(framebufferWidth) : 0;
	m_framebufferHeight = framebufferHeight > 0 ? static_cast<uint32_t>(framebufferHeight) : 0;

	return true;
}

void GlfwWindow::Shutdown()
{
	m_inputListener.ClearCallbacks();

	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();

	m_framebufferWidth = 0;
	m_framebufferHeight = 0;
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

void GlfwWindow::SetInputCallbacks(WindowInputListener::Callbacks callbacks)
{
	m_inputListener.SetCallbacks(std::move(callbacks));
}

void GlfwWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
	if (self)
		self->m_inputListener.NotifyKey({ key, scancode, action, mods });
}

void GlfwWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
	if (!self)
		return;

	double cursorX = 0.0;
	double cursorY = 0.0;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	self->m_inputListener.NotifyMouseButton({ button, action, mods, cursorX, cursorY });
}

void GlfwWindow::CursorPositionCallback(GLFWwindow* window, double x, double y)
{
	auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
	if (self)
		self->m_inputListener.NotifyCursorPosition({ x, y });
}

void GlfwWindow::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
	if (self)
		self->m_inputListener.NotifyScroll({ xOffset, yOffset });
}

float GlfwWindow::GetAspectRatio() const
{
	if (m_framebufferHeight == 0)
		return 0.0f;

	return static_cast<float>(m_framebufferWidth) / static_cast<float>(m_framebufferHeight);
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

	self->m_framebufferWidth = width > 0 ? static_cast<uint32_t>(width) : 0;
	self->m_framebufferHeight = height > 0 ? static_cast<uint32_t>(height) : 0;
	self->m_inputListener.NotifyFramebufferResize({ width, height });
}
