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
	ResourceItemData(std::string id, ResourceType type): id(id), type(type) {}
	~ResourceItemData() {}

	std::string id;
	ResourceType type;

private:

};
