#pragma once

#include <wx/treebase.h>

enum class ResourceType
{
	TYPE_ROOT,
	TYPE_GROUP,
	TYPE_RESOURCE
};
class ResourceItemData : public wxTreeItemData
{


public:
	ResourceItemData(std::string id, ResourceType type, std::string parent = "ROOT") : id(id), type(type), parent(parent) {}
	~ResourceItemData() {}

	std::string id;
	std::string parent;
	ResourceType type;

private:

};
