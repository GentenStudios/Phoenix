#include <Phoenix/DebugWindows.hpp>
#include <Phoenix/Phoenix.hpp>
#include <Phoenix/DebugUI.hpp>


bool DisplayRenderStatistics = false;

void DebugUIMainMenuBar(void* ref) 
{
	phx::Phoenix* engine = reinterpret_cast<phx::Phoenix*>(ref);

	if (ImGui::BeginMenu("Debug"))
	{
		ImGui::MenuItem("Show Render Statistics", NULL, &DisplayRenderStatistics, true);


		ImGui::EndMenu();
	}
}

void DebugUIRenderSystemStatistics(void* ref)
{
	if (!DisplayRenderStatistics)
		return;

	phx::Phoenix* engine = reinterpret_cast<phx::Phoenix*>(ref);

	ImGui::SetNextWindowPos(ImVec2(20, 20));

	ImGuiWindowFlags flags =
	    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	if (!ImGui::Begin("Render Statistics", &DisplayRenderStatistics, flags))
	{
		ImGui::End();
		return;
	}

	static float counterUpdateDelta = 2.0f;
	static float fps                = 0.0f;
	counterUpdateDelta += engine->GetStatistics().GetStatisticSecconds("Total Frametime");

	if (counterUpdateDelta > 1.0f)
	{
		counterUpdateDelta = 0.0f;
		fps                = 1000.0f / engine->GetStatistics().GetStatisticDelta("Total Frametime");
	}

	ImGui::Text("FPS: %.3g", fps);

	for (auto& it : engine->GetStatistics().GetRecordings())
	{
		ImGui::Text("%s: %.3gms", it.name.c_str(), it.time);
	}

	ImGui::SetWindowSize(ImVec2(220, ImGui::GetCursorPosY()));

	ImGui::End();
}