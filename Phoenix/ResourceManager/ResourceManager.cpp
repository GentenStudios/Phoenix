#include <Renderer/Device.hpp>
#include <Renderer/Renderpass.hpp>
#include <ResourceManager/RenderTechnique.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include <pugixml.hpp>

#include <assert.h>
#include <sstream>
#include <string.h>

std::string PIPELINES_ROOT = "data/Pipelines/";

ResourceManager::ResourceManager(RenderDevice* device) : mDevice(device) {}

ResourceManager::~ResourceManager()
{
	for (auto& it : mResourceInstances)
	{
		delete it;
	}
	mNamedResourceInstances.clear();
}

void ResourceManager::LoadPipelineDictionary(const char* name, RenderPass* renderPass)
{
	pugi::xml_document     doc;
	pugi::xml_parse_result result = doc.load_file((PIPELINES_ROOT + name).c_str());
	if (!result)
		return;
	pugi::xml_node rootNode = doc.child("Pipelines");

	for (pugi::xml_node pipeline : rootNode.children("Pipeline"))
	{
		LoadPipelineByName(pipeline.attribute("name").as_string(), renderPass);
	}
}

void ResourceManager::LoadPipelineByName(const char* name, RenderPass* renderPass)
{
	RegisterResource(name, new RenderTechnique(mDevice, this, renderPass, name, ((PIPELINES_ROOT + name + ".xml").c_str())), true);
}
