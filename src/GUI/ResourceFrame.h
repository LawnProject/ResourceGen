#pragma once

#include <wx/wx.h>
#include <wx/treebase.h>

class ResourceFrame : public wxFrame
{
	enum
	{
		ID_FILE_NEW = 1,
		ID_FILE_OPEN = 2,
		ID_TREE_ADD_GROUP = 1001,
		ID_TREE_ADD_IMAGE = 1002,
		ID_TREE_ADD_SOUND = 1003,
		ID_TREE_REMOVE_ITEM = 1004,
	};

public:
	ResourceFrame();
	~ResourceFrame();

	wxTreeCtrl* mResourceTree;

private:
	void OnNewFile(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnTreeRightClick(wxTreeEvent& event);
	void AddGroup(wxCommandEvent& event);
	void AddImage(wxCommandEvent& event);
	void AddSound(wxCommandEvent& event);
	void DeleteItem(wxCommandEvent& event);
};