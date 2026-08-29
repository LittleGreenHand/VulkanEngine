#pragma once

#include <cstdint>
#include <string>

#define GLFW_INCLUDE_NONE
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


class GlfwWindow
{
public:
	struct CreateInfo
	{
		uint32_t width = 2560;
		uint32_t height = 1440;
		std::string title = "Application";

		bool resizable = true;
		bool maximized = false;
	};

public:
	GlfwWindow() = default;
	~GlfwWindow();
	GlfwWindow(const GlfwWindow&) = delete;
	GlfwWindow& operator=(const GlfwWindow&) = delete;

public:
	bool Init(const CreateInfo& createInfo);
	void Shutdown();
	void PollEvents() const;
	void WaitEvents() const;
	bool ShouldClose() const;
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	float GetAspectRatio() const;
	bool IsMinimized() const { return m_width == 0 || m_height == 0; }

public:
	GLFWwindow* GetNativeWindow() const { return m_window; }

public:
	VkSurfaceKHR CreateSurface(VkInstance instance) const;
	static const char** GetRequiredVulkanExtensions(uint32_t& extensionCount);

private:
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
	GLFWwindow* m_window = nullptr;

	uint32_t m_width = 0;
	uint32_t m_height = 0;
};