#pragma once

#include <wx/wx.h>
#include <wx/treebase.h>
#include "ResourceManifest.h"

class ResourceFrame : public wxFrame
{
	enum
	{
		ID_FILE_NEW = 1,
		ID_FILE_OPEN,
		ID_FILE_SAVE,
		ID_FILE_GENERATE_SRC,
		ID_TREE_ADD_GROUP = 1001,
		ID_TREE_ADD_IMAGE,
		ID_TREE_ADD_SOUND,
		ID_TREE_ADD_FONT,
		ID_TREE_ADD_DEFAULT_SETTINGS,
		ID_TREE_REMOVE_ITEM,
		ID_TREE_RENAME_ITEM,
		ID_TREE_MOVE_UP_ITEM,
		ID_TREE_MOVE_DOWN_ITEM,
		ID_RESOURCE_PATH_FIELD = 2001,
		ID_RESOURCE_STEP_COL,
		ID_RESOURCE_STEP_ROW,
		ID_RESOURCE_ALPHA_FIELD,
		ID_RESOURCE_IDPREFIX_FIELD,
	};

public:
	ResourceFrame();
	~ResourceFrame();

	wxTreeItemId mRoot;
	wxTreeItemId mCurrentResource;
	wxPanel* mLeftPanel;
	wxPanel* mRightPanel;

	wxTextCtrl* mPathField;

	//Image
	wxTextCtrl* mAlphaField;

	//Settings
	wxTextCtrl* mIDPrefixField;


	std::map<std::string, wxTreeItemId> mItems;
	wxTreeCtrl* mResourceTree;
	ResourceManifest mResourceManifest;
	std::string mItemStrEditingNow;
	std::string mAssetRoot;
	std::pair<std::string, std::string> mSwappingItems;

private:

	//GUI Interaction
	void OnNewFile(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnSaveFile(wxCommandEvent& event);
	void OnGenerateSourceFile(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnTreeClick(wxTreeEvent& event);
	void OnTreeRightClick(wxTreeEvent& event);
	void OnItemRenameStart(wxTreeEvent& event);
	void OnItemRenameEnd(wxTreeEvent& event);

	//Tree editing
	void AddGroup(wxCommandEvent& event);
	void AddImage(wxCommandEvent& event);
	void AddSound(wxCommandEvent& event);
	void AddFont(wxCommandEvent& event);
	void AddDefaultSettings(wxCommandEvent& event);
	void AddGroupImpl(std::string theName);
	void AddImageImpl(std::string theName, std::string theGroup);
	void AddSoundImpl(std::string theName, std::string theGroup);
	void AddFontImpl(std::string theName, std::string theGroup);
	void AddDefaultSettingsImpl(std::string theName, std::string theGroup);
	void DeleteItem(wxCommandEvent& event);
	void RenameItem(wxCommandEvent& event);
	void MoveUpItem(wxCommandEvent& event);
	void MoveDownItem(wxCommandEvent& event);

	void SetResourcePath(wxCommandEvent& event);

	//Image functions
	void SetImageColumns(wxSpinEvent& event);
	void SetImageRow(wxSpinEvent& event);
	void SetImageAlphaGrid(wxCommandEvent& event);

	//Setting functions
	void SetDefaultSettingsIDPrefix(wxCommandEvent& event);
};