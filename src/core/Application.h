#pragma once

class VulkanRenderer;
class GlfwWindow;
class ImGuiLayer;
class Application
{
public:
	static bool IsInit() { return init; }
	static Application& GetInstance()
	{
		static Application instance;
		return instance;
	}
	Application() = default;
	~Application() { Destroy(); }
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

	bool Init();
	void Destroy();
	void Resize(int width, int height);
	bool Run();
	void BeginFrame();
	void UpdateScene();
	void Simulate();
	void Render();
	void EndFrame();
private:
	

	static bool init;
	VulkanRenderer* renderer = nullptr;
	GlfwWindow* window = nullptr;
	ImGuiLayer* guiLayer = nullptr;
};