#include "LightManager.h"

glm::vec3* LightManager::GetLightPosesPointer()
{
	if (lightPoses.size() > 0)
		return &lightPoses[0];

	return nullptr;
}

glm::vec3* LightManager::GetDiffuseColorPointer()
{
	if (diffuseColor.size() > 0)
		return &diffuseColor[0];

	return nullptr;
}

glm::vec3* LightManager::GetSpecularColorPointer()
{
	if (specularColor.size() > 0)
		return &specularColor[0];

	return nullptr;
}

glm::vec3* LightManager::GetLightAt(int i)
{
	return &lightPoses[i];
}

glm::vec3* LightManager::GetDiffuseAt(int i)
{
	return &diffuseColor[i];
}

glm::vec3* LightManager::GetSpecularAt(int i)
{
	return &specularColor[i];
}

bool LightManager::AddLight(glm::vec3 pos, glm::vec3 diffuse, glm::vec3 specular)
{
	if (lightPoses.size() > m_max)
		return false;

	lightPoses.push_back(pos);
	diffuseColor.push_back(diffuse);
	specularColor.push_back(specular);
}

void LightManager::RemoveLight(int at)
{
	lightPoses.erase(lightPoses.begin() + at);
	diffuseColor.erase(diffuseColor.begin() + at); 
	specularColor.erase(specularColor.begin() + at);
}
