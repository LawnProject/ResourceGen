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
	ResourceItemData(ResourceType type): type(type) {}
	~ResourceItemData() {}

	ResourceType type;

private:

};
