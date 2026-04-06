#pragma once

#include <wx/treebase.h>

enum class ResourceType
{
	TYPE_ROOT,
	TYPE_GROUP,
	TYPE_RESOURCE
};
enum class ResourceSubType
{
	TYPE_NONE,
	TYPE_IMAGE,
	TYPE_SOUND,
	TYPE_FONT,
	TYPE_DEFAULT_SETTINGS,
};

class ResourceItemData : public wxTreeItemData
{


public:
	ResourceItemData(std::string id, ResourceType type, std::string parent = "ROOT", ResourceSubType subType = ResourceSubType::TYPE_NONE) : mID(id), mType(type), mParent(parent), mSubType(subType) {}
	~ResourceItemData() {}

	std::string mID;
	std::string mParent;
	ResourceSubType mSubType;
	ResourceType mType;

private:

};
