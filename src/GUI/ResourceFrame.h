#pragma once

#include <wx/wx.h>
#include <wx/treebase.h>
#include "ResourceManifest.h"
#include "PreviewImage.h"

class ResourceFrame : public wxFrame
{
	enum
	{
		ID_FILE_NEW = 1,
		ID_FILE_OPEN,
		ID_FILE_SAVE,
		ID_FILE_GENERATE_SRC,
		ID_FILE_SET_ASSET_ROOT,
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
		ID_RESOURCE_ALPHA_MASK_FIELD,
		ID_RESOURCE_ALPHA_GRID_FIELD,
		ID_RESOURCE_ALPHA_COLOR_FIELD,
		ID_RESOURCE_NO_ALPHA_BOX,
		ID_RESOURCE_PALLETIZE_BOX,
		ID_RESOURCE_NOBITS_BOX,
		ID_RESOURCE_NOBITS3D_BOX,
		ID_RESOURCE_NOBITS2D_BOX,
		ID_RESOURCE_MINSUBDIVISION_BOX,
		ID_RESOURCE_DDSURFACE_BOX,
		ID_RESOURCE_PIXELFORMAT_BOX,
		ID_RESOURCE_VARIANT_FIELD,
		ID_RESOURCE_ANIMTYPE_BOX,
		ID_RESOURCE_FRAMEDELAY_STEP,
		ID_RESOURCE_BEGINDELAY_STEP,
		ID_RESOURCE_ENDDELAY_STEP,
		ID_RESOURCE_PERFRAMEDELAY_FIELD,
		ID_RESOURCE_FRAMEMAP_FIELD,
		ID_RESOURCE_IDPREFIX_FIELD,
		ID_FRAMEWORK_SAF = 3001,
		ID_FRAMEWORK_RESODDEDFRAMEWORK,
	};

public:
	ResourceFrame();
	~ResourceFrame();

	wxTreeItemId mRoot;
	wxTreeItemId mCurrentResource;
	wxPanel* mLeftPanel = nullptr;
	wxPanel* mRightPanel = nullptr;

	wxTextCtrl* mPathField = nullptr;

	//Image
	wxTextCtrl* mAlphaMaskField = nullptr;
	wxTextCtrl* mAlphaGridField = nullptr;
	wxTextCtrl* mAlphaColor = nullptr;
	wxCheckBox* mNoAlpha = nullptr;
	wxCheckBox* mPalletize = nullptr;
	wxCheckBox* mMinimizeSubdivisions = nullptr;
	wxCheckBox* mNoBits3D = nullptr;
	wxCheckBox* mNoBits2D = nullptr;
	wxCheckBox* mNoBits = nullptr;
	wxCheckBox* mDDSurface = nullptr;
	wxChoice* mPixelFormats = nullptr;
	wxTextCtrl* mVariantField = nullptr;

	wxChoice* mAnimTypes = nullptr;
	wxTextCtrl* mPerFrameDelayField = nullptr;
	wxTextCtrl* mFrameMapField = nullptr;



	//Settings
	wxTextCtrl* mIDPrefixField;


	std::map<std::string, wxTreeItemId> mItems;
	wxTreeCtrl* mResourceTree;
	ResourceManifest mResourceManifest;
	std::string mItemStrEditingNow;
	std::string mAssetRoot;
	std::pair<std::string, std::string> mSwappingItems;

	PreviewImage* mPreviewImage = nullptr;

private:

	//GUI Interaction
	void OnNewFile(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnSaveFile(wxCommandEvent& event);
	void OnGenerateSourceFile(wxCommandEvent& event);
	void OnSetAssetRoot(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnTreeClick(wxTreeEvent& event);
	void OnTreeRightClick(wxTreeEvent& event);
	void OnItemRenameStart(wxTreeEvent& event);
	void OnItemRenameEnd(wxTreeEvent& event);

	void SetFrameworkToSAF(wxCommandEvent& event);
	void SetFrameworkToResodded(wxCommandEvent& event);


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
	void SetImageVariant(wxCommandEvent& event);
	void SetImageAlphaMask(wxCommandEvent& event);
	void SetImageAlphaGrid(wxCommandEvent& event);
	void SetImageAlphaColor(wxCommandEvent& event);
	void SetImageNoAlpha(wxCommandEvent& event);
	void SetImagePalletize(wxCommandEvent& event);
	void SetImageNoBits(wxCommandEvent& event);
	void SetImageNoBits3D(wxCommandEvent& event);
	void SetImageNoBits2D(wxCommandEvent& event);
	void SetImageDDSurface(wxCommandEvent& event);
	void SetImageMinSubdivision(wxCommandEvent& event);
	void SetImagePixelFormat(wxCommandEvent& event);
	void SetImageAnimType(wxCommandEvent& event);
	void SetImageFrameDelay(wxSpinEvent& event);
	void SetImageBeginFrameDelay(wxSpinEvent& event);
	void SetImageEndDelay(wxSpinEvent& event);
	void SetImagePerFrameDelay(wxCommandEvent& event);
	void SetImageFrameMap(wxCommandEvent& event);

	//Setting functions
	void SetDefaultSettingsIDPrefix(wxCommandEvent& event);
};