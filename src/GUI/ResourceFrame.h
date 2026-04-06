#pragma once

#include <wx/wx.h>
#include <wx/treebase.h>
#include "ResourceManifest.h"

class ResourceFrame : public wxFrame
{
	enum
	{
		ID_FILE_NEW = 1,
		ID_FILE_OPEN = 2,
		ID_FILE_SAVE = 3,
		ID_FILE_GENERATE_SRC = 4,
		ID_TREE_ADD_GROUP = 1001,
		ID_TREE_ADD_IMAGE = 1002,
		ID_TREE_ADD_SOUND = 1003,
		ID_TREE_ADD_FONT = 1004,
		ID_TREE_REMOVE_ITEM = 1005,
		ID_TREE_RENAME_ITEM = 1006,
	};

public:
	ResourceFrame();
	~ResourceFrame();

	wxTreeItemId mRoot;
	std::map<std::string, wxTreeItemId> mItems;
	wxTreeCtrl* mResourceTree;
	ResourceManifest mResourceManifest;
	std::string mItemStrEditingNow;

private:
	void OnNewFile(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnSaveFile(wxCommandEvent& event);
	void OnGenerateSourceFile(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnTreeRightClick(wxTreeEvent& event);
	void OnItemRenameStart(wxTreeEvent& event);
	void OnItemRenameEnd(wxTreeEvent& event);
	void AddGroup(wxCommandEvent& event);
	void AddImage(wxCommandEvent& event);
	void AddSound(wxCommandEvent& event);
	void AddFont(wxCommandEvent& event);
	void AddGroupImpl(std::string theName);
	void AddImageImpl(std::string theName, std::string theGroup);
	void AddSoundImpl(std::string theName, std::string theGroup);
	void AddFontImpl(std::string theName, std::string theGroup);
	void DeleteItem(wxCommandEvent& event);
	void RenameItem(wxCommandEvent& event);
};