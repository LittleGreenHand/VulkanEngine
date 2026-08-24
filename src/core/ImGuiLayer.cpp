#include "ImGuiLayer.h"

//void VulkanRendererBase::updateOverlay()
//{
//	if (!settings.overlay)
//		return;
//
//	ImGuiIO& io = ImGui::GetIO();
//
//	io.DisplaySize = ImVec2((float)width, (float)height);
//	io.DeltaTime = frameTimer;
//
//	ImGui::NewFrame();
//	ImGui_ImplWin32_NewFrame();
//	//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
//	//ImGui::SetNextWindowPos(ImVec2(10 * ui.scale, 10 * ui.scale));
//	//ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
//	ImGui::Begin("全局设置", nullptr, 0);
//	//ImGui::TextUnformatted(title.c_str());
//	//ImGui::TextUnformatted(deviceProperties.deviceName);
//	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);
//
//#if defined(VK_USE_PLATFORM_ANDROID_KHR)
//	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f * ui.scale));
//#endif
//	//ImGui::PushItemWidth(110.0f * ui.scale);
//	OnUpdateUIOverlay(&uiOverlay);
//	//ImGui::PopItemWidth();
//#if defined(VK_USE_PLATFORM_ANDROID_KHR)
//	ImGui::PopStyleVar();
//#endif
//
//	ImGui::End();
//	//ImGui::PopStyleVar();
//#if defined(VK_USE_PLATFORM_ANDROID_KHR)
//	ImGui::PopStyleVar();
//#endif
//	drawNodeTree();
//	ImGui::Render();
//
//	uiOverlay.update(currentBuffer);
//
//#if defined(VK_USE_PLATFORM_ANDROID_KHR)
//	if (mouseState.buttons.left) {
//		mouseState.buttons.left = false;
//	}
//#endif
//}