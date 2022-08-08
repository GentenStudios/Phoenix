#include <Phoenix/DebugWindows.hpp>
#include <Phoenix/Phoenix.hpp>
#include <Phoenix/DebugUI.hpp>
#include <Phoenix/Chunk.hpp>
#include <Phoenix/World.hpp>

#include <ResourceManager/ResourceManager.hpp>

#include <Globals/Globals.hpp>


bool DisplayRenderStatistics = false;
bool DisplayMemoryUsage = false;

void DebugUIMainMenuBar(void* ref) 
{
	phx::Phoenix* engine = reinterpret_cast<phx::Phoenix*>(ref);

	if (ImGui::BeginMenu("Debug"))
	{
		ImGui::MenuItem("Show Render Statistics", NULL, &DisplayRenderStatistics, true);
		ImGui::MenuItem("Show Memory Usage", NULL, &DisplayMemoryUsage, true);


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

	ImGui::Text("FPS: %i", (int)fps);

	for (auto& it : engine->GetStatistics().GetRecordings())
	{
		ImGui::Text("%s: %.3gms", it.name.c_str(), it.time);
	}

	ImGui::SetWindowSize(ImVec2(220, ImGui::GetCursorPosY()));

	ImGui::End();
}

void DebugUIMemoryUsage(void* ref) 
{
	if (!DisplayMemoryUsage)
		return;

	phx::Phoenix* engine = reinterpret_cast<phx::Phoenix*>(ref);
	ResourceManager* resourceManager = engine->GetResourceManager();

	phx::World* world = resourceManager->GetResource<phx::World>("World");

	ImGui::SetNextWindowPos(ImVec2(20, 20));

	ImGuiWindowFlags flags =
	    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	if (!ImGui::Begin("Memory Usage", &DisplayRenderStatistics, flags))
	{
		ImGui::End();
		return;
	}

	unsigned int totalPools = TOTAL_VERTEX_PAGE_COUNT;
	unsigned int freePools     = world->GetFreeMemoryPoolCount();
	float        poolUsageFrac = 1.0f - ((float) freePools / (float) totalPools);

	const unsigned int  memoryPoolPageSize      = VERTEX_PAGE_SIZE * sizeof(phx::VertexData);
	const unsigned int  totalMemoryPoolMemory   = memoryPoolPageSize * TOTAL_VERTEX_PAGE_COUNT;
	unsigned int        memoryPoolmemoryUsage   = memoryPoolPageSize * (TOTAL_VERTEX_PAGE_COUNT - freePools);
	const float         totalMemoryPoolMemoryMB = (float) (totalMemoryPoolMemory) / 1024.0f / 1024.0f;
	float               memoryPoolmemoryUsageMB   = (float) (memoryPoolmemoryUsage) / 1024.0f / 1024.0f;


	ImGui::Text("Total Pools: %i | Free Pools: %i", totalPools, freePools);
	ImGui::Text("Total Pool Memory: %.3gmb | Used Pool Memory: %.3gmb", totalMemoryPoolMemoryMB, memoryPoolmemoryUsageMB);
	ImGui::ProgressBar(poolUsageFrac);



	ImGui::SetWindowSize(ImVec2(400, ImGui::GetCursorPosY()));

	ImGui::End();
}
