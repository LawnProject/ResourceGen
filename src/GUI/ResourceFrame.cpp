#include "ResourceFrame.h"
#include "ResourceItem.h"
#include "ResourceSourceGen.h"
#include <wx/treectrl.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/splitter.h>
#include <filesystem>
#include <wx/textfile.h>


bool HasPrevious(wxTreeCtrl* tree, wxTreeItemId item)
{
    wxTreeItemId prev = tree->GetPrevSibling(item);
    return prev.IsOk();
}

bool HasNext(wxTreeCtrl* tree, wxTreeItemId item)
{
    wxTreeItemId next = tree->GetNextSibling(item);
    return next.IsOk();
}

wxTreeItemId FindItemByID(wxTreeCtrl* tree, wxTreeItemId parent, std::string id)
{
    ResourceItemData* data = (ResourceItemData*)tree->GetItemData(parent);
    if (data)
    {
        std::string key = data->mParent + "/" + data->mID;
        if (key == id)
            return parent;
    }

    wxTreeItemIdValue cookie;
    wxTreeItemId child = tree->GetFirstChild(parent, cookie);

    while (child.IsOk())
    {
        wxTreeItemId result = FindItemByID(tree, child, id);
        if (result.IsOk())
            return result;

        child = tree->GetNextChild(parent, cookie);
    }

    return wxTreeItemId();
}

void CollectExpandedItems(wxTreeCtrl* tree, wxTreeItemId root, std::vector<std::string>& out)
{
    if (!root.IsOk())
        return;
    if (tree->IsExpanded(root))
    {
        ResourceItemData* data = (ResourceItemData*)tree->GetItemData(root);
        if (data)
            out.push_back(data->mParent + "/" + data->mID);
    }

    wxTreeItemIdValue cookie;
    wxTreeItemId child = tree->GetFirstChild(root, cookie);

    while (child.IsOk())
    {
        CollectExpandedItems(tree, child, out);
        child = tree->GetNextChild(root, cookie);
    }
}


ResourceFrame::ResourceFrame()
    : wxFrame(NULL, wxID_ANY, "ResourceGen - a1.0.1", wxDefaultPosition, wxSize(800, 500))
{
    wxInitAllImageHandlers();

    wxMenu* menuFile = new wxMenu();
    menuFile->Append(ID_FILE_NEW, "&New...",
        "Create a new resource project");
    menuFile->Append(ID_FILE_OPEN, "&Open...",
        "Open an existing resource project");
    menuFile->AppendSeparator();
    menuFile->Append(ID_FILE_SAVE, "&Save...",
        "Save the resource project to disk");
    menuFile->Append(ID_FILE_GENERATE_SRC, "&Generate Source...",
        "Generate the source files to be used in the engine");
    menuFile->AppendSeparator();
    menuFile->Append(ID_FILE_SET_ASSET_ROOT, "&Set Asset Root...",
        "Set the asset root from where you can preview resources from");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu();
    menuHelp->Append(wxID_ABOUT);

    wxMenu* menuSettings = new wxMenu();

    wxMenu* frameworkVersionRadio = new wxMenu();
    frameworkVersionRadio->AppendRadioItem(ID_FRAMEWORK_SAF, "SexyAppFramework");
    frameworkVersionRadio->AppendRadioItem(ID_FRAMEWORK_RESODDEDFRAMEWORK, "ResoddedFramework");
    frameworkVersionRadio->Check(ID_FRAMEWORK_SAF, true);
    menuSettings->AppendSubMenu(frameworkVersionRadio, "&Framework Version");

    Bind(wxEVT_MENU, &ResourceFrame::SetFrameworkToSAF, this, ID_FRAMEWORK_SAF);
    Bind(wxEVT_MENU, &ResourceFrame::SetFrameworkToResodded, this, ID_FRAMEWORK_RESODDEDFRAMEWORK);


    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuSettings, "&Settings");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);
    
    wxSplitterWindow* splitter = new wxSplitterWindow(this);

    mLeftPanel = new wxPanel(splitter);

    mResourceTree = new wxTreeCtrl(mLeftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS);
    mResourceTree->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &ResourceFrame::OnTreeRightClick, this);
    mResourceTree->Bind(wxEVT_TREE_SEL_CHANGED, &ResourceFrame::OnTreeClick, this);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(mResourceTree, 1, wxEXPAND);
    mLeftPanel->SetSizer(leftSizer);

    mRightPanel = new wxPanel(splitter);
    mRightPanel->SetBackgroundColour(*wxWHITE);

    splitter->SplitVertically(mLeftPanel, mRightPanel, 250);
    splitter->SetMinimumPaneSize(100);

    mRoot = mResourceTree->AddRoot("Resource Project");
    mResourceTree->SetItemData(mRoot, new ResourceItemData("ROOT", ResourceType::TYPE_ROOT));
    mResourceTree->ExpandAll();

    CreateStatusBar();
    SetStatusText("Welcome to ResourceGen");
    Bind(wxEVT_MENU, &ResourceFrame::OnNewFile, this, ID_FILE_NEW);
    Bind(wxEVT_MENU, &ResourceFrame::OnOpenFile, this, ID_FILE_OPEN);
    Bind(wxEVT_MENU, &ResourceFrame::OnSaveFile, this, ID_FILE_SAVE);
    Bind(wxEVT_MENU, &ResourceFrame::OnGenerateSourceFile, this, ID_FILE_GENERATE_SRC);
    Bind(wxEVT_MENU, &ResourceFrame::OnSetAssetRoot, this, ID_FILE_SET_ASSET_ROOT);
    Bind(wxEVT_MENU, &ResourceFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &ResourceFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_TREE_BEGIN_LABEL_EDIT, &ResourceFrame::OnItemRenameStart, this);
    Bind(wxEVT_TREE_END_LABEL_EDIT, &ResourceFrame::OnItemRenameEnd, this);
}

ResourceFrame::~ResourceFrame()
{

}

void ResourceFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void ResourceFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("ResourceGen is a modernized version of PopCap's ResourceGen Tool made for SexyAppFramework",
        "About ResourceGen", wxOK | wxICON_INFORMATION);
}

void ResourceFrame::OnNewFile(wxCommandEvent& event)
{
    mResourceManifest.mGroupMap.clear();
    mResourceTree->DeleteAllItems();
    mRoot = mResourceTree->AddRoot("Resource Project");
    mResourceTree->SetItemData(mRoot, new ResourceItemData("ROOT", ResourceType::TYPE_ROOT));
    mResourceTree->ExpandAll();    
    mRightPanel->DestroyChildren();
    mAlphaMaskField = nullptr;
    mAlphaGridField = nullptr;
    mAlphaColor = nullptr;
    mNoAlpha = nullptr;
    mPalletize = nullptr;
    mMinimizeSubdivisions = nullptr;
    mNoBits3D = nullptr;
    mNoBits2D = nullptr;
    mNoBits = nullptr;
    mDDSurface = nullptr;
    mVariantField = nullptr;
    mPixelFormats = nullptr;
    mPerFrameDelayField = nullptr;
    mAnimTypes = nullptr;
    mFrameMapField = nullptr;
    mPreviewImage = nullptr;
    mBold = nullptr;
    mItalic = nullptr;
    mShadow = nullptr;
    mUnderline = nullptr;
    mStepperSize = nullptr;
    mRightPanel->SetSizer(nullptr);
}

void ResourceFrame::OnOpenFile(wxCommandEvent& event)
{
    wxFileDialog openFileDialog(this, _("Open Resource file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    mResourceManifest.Import(openFileDialog.GetPath().ToStdString());
    mResourceTree->DeleteAllItems();
    mRoot = mResourceTree->AddRoot("Resource Project");
    mResourceTree->SetItemData(mRoot, new ResourceItemData("ROOT", ResourceType::TYPE_ROOT));
    mResourceTree->ExpandAll();

    for (auto group : mResourceManifest.mGroupMap)
    {
        AddGroupImpl(group.first);
        for (auto res : group.second.mResourceMap)
        {
            switch (res.second->mType)
            {
            case ResourceSubType::TYPE_IMAGE:
                AddImageImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_SOUND:
                AddSoundImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_FONT:
                AddFontImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_DEFAULT_SETTINGS:
                AddDefaultSettingsImpl(res.first, group.first);
                break;
            }
            
        }
    }
}

void ResourceFrame::OnItemRenameStart(wxTreeEvent& event)
{
    mItemStrEditingNow = event.GetLabel();
}

void ResourceFrame::OnItemRenameEnd(wxTreeEvent& event)
{
    if (event.IsEditCancelled())
        return;
    std::string aNewName = event.GetLabel();

    wxTreeItemId hoveredItem = event.GetItem();

    mResourceTree->SelectItem(hoveredItem);

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(hoveredItem);

    if (data->mType == ResourceType::TYPE_GROUP)
    {
        if (mResourceManifest.mGroupMap.count(aNewName))
        {
            wxLogError("You cannot name 2 groups the same");
            event.Veto();
            return;
        }
    }
    else if (data->mType == ResourceType::TYPE_RESOURCE)
    {
        if (mResourceManifest.GetResource(data->mParent, aNewName) != nullptr)
        {
            wxLogError("You cannot name 2 resources the same");
            event.Veto();
            return;
        }
    }
    else
    {
        event.Veto();
        return;
    }

    mResourceTree->SetItemText(hoveredItem, aNewName);
    data->mID = aNewName;

    mItems[aNewName] = mItems[mItemStrEditingNow];
    mItems.erase(mItemStrEditingNow);

    if (data->mType == ResourceType::TYPE_GROUP)
    {
        mResourceManifest.mGroupMap[aNewName] = mResourceManifest.mGroupMap[mItemStrEditingNow];
        mResourceManifest.mGroupMap.erase(mItemStrEditingNow);
        wxTreeItemIdValue cookie;
        wxTreeItemId ch = mResourceTree->GetFirstChild(hoveredItem, cookie);
        while (ch.IsOk()) 
        {
            ResourceItemData* childData = (ResourceItemData*)mResourceTree->GetItemData(ch);
            childData->mParent = aNewName;
            ch = mResourceTree->GetNextSibling(ch);
        }
    }
    else if (data->mType == ResourceType::TYPE_RESOURCE)
    {
        auto& vec = mResourceManifest.mGroupMap.at(data->mParent).mResourceMap;

        auto it = std::find_if(vec.begin(), vec.end(),
            [&](const auto& pair) { return pair.first == mItemStrEditingNow; });

        if (it != vec.end())
        {
            it->first = aNewName;
            it->second->mID = aNewName;
        }
    }
}

void ResourceFrame::SetFrameworkToSAF(wxCommandEvent& event)
{
    mResourceManifest.mFrameworkVersion = FrameworkVersion::VERSION_SEXYAPPFRAMEWORK;
    if (mPalletize != nullptr) mPalletize->Enable();
    if (mMinimizeSubdivisions != nullptr) mMinimizeSubdivisions->Enable();
    if (mDDSurface != nullptr) mDDSurface->SetLabel("Use a DDSurface");

}

void ResourceFrame::SetFrameworkToResodded(wxCommandEvent& event)
{
    mResourceManifest.mFrameworkVersion = FrameworkVersion::VERSION_RESODDEDFRAMEWORK;
    if (mPalletize != nullptr) mPalletize->Disable();
    if (mMinimizeSubdivisions != nullptr) mMinimizeSubdivisions->Disable();
    if (mDDSurface != nullptr) mDDSurface->SetLabel("Use a GPUSurface");
}

void ResourceFrame::OnSaveFile(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _("Resource file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    mResourceManifest.Export(saveFileDialog.GetPath().ToStdString());
}

void ResourceFrame::OnGenerateSourceFile(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _("Source files"), "", "", "", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::string aSrcOutput = ResourceSourceGen::GenerateSourceFile(mResourceManifest, saveFileDialog.GetFilename().ToStdString());
    std::string aHeaderOutput = ResourceSourceGen::GenerateHeaderFile(mResourceManifest, saveFileDialog.GetFilename().ToStdString());
    wxTextFile file;
    file.Create(saveFileDialog.GetPath() + ".cpp");
    file.Open(saveFileDialog.GetPath() + ".cpp");
    file.Clear();
    file.AddLine(aSrcOutput);
    file.Write();
    file.Close();    
    wxLogMessage("Succesfully exported " + saveFileDialog.GetFilename().ToStdString() + ".cpp");

    file.Create(saveFileDialog.GetPath() + ".h");
    file.Open(saveFileDialog.GetPath() + ".h");
    file.Clear();
    file.AddLine(aHeaderOutput);
    file.Write();
    file.Close();

    wxLogMessage("Succesfully exported " + saveFileDialog.GetFilename().ToStdString() + ".h");
    wxLogMessage("Succesfully exported source files");
}

void ResourceFrame::OnSetAssetRoot(wxCommandEvent& event)
{
    wxDirDialog folderDialog(this, _("Asset Root"), "", wxDD_DIR_MUST_EXIST);
    if (folderDialog.ShowModal() == wxID_CANCEL)
        return;

    mAssetRoot = folderDialog.GetPath().ToStdString();
}

void ResourceFrame::OnTreeClick(wxTreeEvent& event)
{
    mCurrentResource = event.GetItem();
    mResourceTree->SelectItem(mCurrentResource);
    ResourceItemData* aResourceData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    mRightPanel->Freeze();
    mRightPanel->DestroyChildren(); 
    //Manually null them here
    mAlphaMaskField = nullptr;
    mAlphaGridField = nullptr;
    mAlphaColor = nullptr;
    mNoAlpha = nullptr;
    mPalletize = nullptr;
    mMinimizeSubdivisions = nullptr;
    mNoBits3D = nullptr;
    mNoBits2D = nullptr;
    mNoBits = nullptr;
    mDDSurface = nullptr;
    mVariantField = nullptr;
    mPixelFormats = nullptr;
    mPerFrameDelayField = nullptr;
    mAnimTypes = nullptr;
    mFrameMapField = nullptr;
    mPreviewImage = nullptr;	
    mBold = nullptr;
    mItalic = nullptr;
    mShadow = nullptr;
    mUnderline = nullptr;
    mStepperSize = nullptr;
    mRightPanel->SetSizer(nullptr);

    switch (aResourceData->mType)
    {
        case ResourceType::TYPE_GROUP:
        {
            break;
        }
        case ResourceType::TYPE_RESOURCE:
        {
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

            wxScrolledWindow* settingsPanel = new wxScrolledWindow(
                mRightPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);

            settingsPanel->SetScrollRate(0, 10);

            wxBoxSizer* settingsSizer = new wxBoxSizer(wxVERTICAL);

            mPathField = new wxTextCtrl(settingsPanel, ID_RESOURCE_PATH_FIELD);
            mPathField->SetValue(mResourceManifest.mGroupMap[aResourceData->mParent].GetResource(aResourceData->mID)->mPath);
            Bind(wxEVT_TEXT, &ResourceFrame::SetResourcePath, this, ID_RESOURCE_PATH_FIELD);
            switch (aResourceData->mSubType)
            {
            case ResourceSubType::TYPE_IMAGE:
            {
                ResourceImage* anImageData = mResourceManifest.mGroupMap[aResourceData->mParent].GetImage(aResourceData->mID).get();

                settingsSizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Image Settings"), 0);
                wxBoxSizer* pathThing = new wxBoxSizer(wxHORIZONTAL);
                pathThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Path:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                pathThing->Add(mPathField, 1);
                settingsSizer->Add(pathThing, 0, wxEXPAND | wxALL, 5);

                wxSpinCtrl* aStepperCol = new wxSpinCtrl(settingsPanel, ID_RESOURCE_STEP_COL);
                aStepperCol->SetMin(1);
                wxBoxSizer* colThing = new wxBoxSizer(wxHORIZONTAL);
                colThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Columns:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                colThing->Add(aStepperCol, 1);
                settingsSizer->Add(colThing, 0, wxEXPAND | wxALL, 5);
                aStepperCol->SetValue(anImageData->mCols);
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetImageColumns, this, ID_RESOURCE_STEP_COL);

                wxSpinCtrl* aStepperRow = new wxSpinCtrl(settingsPanel, ID_RESOURCE_STEP_ROW);
                aStepperRow->SetMin(1);
                wxBoxSizer* rowThing = new wxBoxSizer(wxHORIZONTAL);
                rowThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Rows:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                rowThing->Add(aStepperRow, 1);
                settingsSizer->Add(rowThing, 0, wxEXPAND | wxALL, 5);
                aStepperRow->SetValue(anImageData->mRows);
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetImageRow, this, ID_RESOURCE_STEP_ROW);

                wxStaticLine* alphaLine = new wxStaticLine(settingsPanel, wxID_ANY);
                settingsSizer->Add(alphaLine, 0, wxEXPAND | wxALL, 5);

                mAlphaMaskField = new wxTextCtrl(settingsPanel, ID_RESOURCE_ALPHA_MASK_FIELD);
                wxBoxSizer* alpha1Thing = new wxBoxSizer(wxHORIZONTAL);
                alpha1Thing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Alpha Mask:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                alpha1Thing->Add(mAlphaMaskField, 1);
                settingsSizer->Add(alpha1Thing, 0, wxEXPAND | wxALL, 5);
                mAlphaMaskField->SetValue(anImageData->mAlphaMask);
                Bind(wxEVT_TEXT, &ResourceFrame::SetImageAlphaMask, this, ID_RESOURCE_ALPHA_MASK_FIELD);

                mAlphaGridField = new wxTextCtrl(settingsPanel, ID_RESOURCE_ALPHA_GRID_FIELD);
                wxBoxSizer* alpha2Thing = new wxBoxSizer(wxHORIZONTAL);
                alpha2Thing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Alpha Grid:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                alpha2Thing->Add(mAlphaGridField, 1);
                settingsSizer->Add(alpha2Thing, 0, wxEXPAND | wxALL, 5);
                mAlphaGridField->SetValue(anImageData->mAlphaGrid);
                Bind(wxEVT_TEXT, &ResourceFrame::SetImageAlphaGrid, this, ID_RESOURCE_ALPHA_GRID_FIELD);

                mAlphaColor = new wxTextCtrl(settingsPanel, ID_RESOURCE_ALPHA_COLOR_FIELD);
                wxBoxSizer* alpha3Thing = new wxBoxSizer(wxHORIZONTAL);
                alpha3Thing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Alpha Color (Hexidecimal Format):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                alpha3Thing->Add(mAlphaColor, 1);
                settingsSizer->Add(alpha3Thing, 0, wxEXPAND | wxALL, 5);
                mAlphaColor->SetValue(anImageData->mAlphaColor);
                Bind(wxEVT_TEXT, &ResourceFrame::SetImageAlphaColor, this, ID_RESOURCE_ALPHA_COLOR_FIELD);

                mNoAlpha = new wxCheckBox(settingsPanel, ID_RESOURCE_NO_ALPHA_BOX, "No Alpha");
                mNoAlpha->SetValue(anImageData->mNoAlpha);
                settingsSizer->Add(mNoAlpha, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetImageNoAlpha, this, ID_RESOURCE_NO_ALPHA_BOX);

                wxStaticLine* formatLine = new wxStaticLine(settingsPanel, wxID_ANY);
                settingsSizer->Add(formatLine, 0, wxEXPAND | wxALL, 5);

                mPalletize = new wxCheckBox(settingsPanel, ID_RESOURCE_PALLETIZE_BOX, "Palletize");
                mPalletize->SetValue(anImageData->mPalletize);
                settingsSizer->Add(mPalletize, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetImageNoAlpha, this, ID_RESOURCE_PALLETIZE_BOX);

                mNoBits = new wxCheckBox(settingsPanel, ID_RESOURCE_NOBITS_BOX, "No Bits");
                mNoBits->SetValue(anImageData->mNoBits);
                settingsSizer->Add(mNoBits, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetImageNoBits, this, ID_RESOURCE_NOBITS_BOX);

                mNoBits3D = new wxCheckBox(settingsPanel, ID_RESOURCE_NOBITS3D_BOX, "No Bits 3D");
                mNoBits3D->SetValue(anImageData->mNoBits3D);
                settingsSizer->Add(mNoBits3D, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetImageNoBits3D, this, ID_RESOURCE_NOBITS3D_BOX);

                mNoBits2D = new wxCheckBox(settingsPanel, ID_RESOURCE_NOBITS2D_BOX, "No Bits 2D");
                mNoBits2D->SetValue(anImageData->mNoBits2D);
                settingsSizer->Add(mNoBits2D, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetImageNoBits2D, this, ID_RESOURCE_NOBITS2D_BOX);

                wxArrayString aFormatVec = { "default", "a4r4g4b4", "a8r8g8b8" };
                mPixelFormats = new wxChoice(settingsPanel, ID_RESOURCE_PIXELFORMAT_BOX, wxDefaultPosition, wxDefaultSize, aFormatVec);
                wxString aFormatString = anImageData->mPixelFormat;
                mPixelFormats->SetSelection(mPixelFormats->FindString(aFormatString));
                wxBoxSizer* pixelThing = new wxBoxSizer(wxHORIZONTAL);
                pixelThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Pixel Format:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                pixelThing->Add(mPixelFormats, 1);
                settingsSizer->Add(pixelThing, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHOICE, &ResourceFrame::SetImagePixelFormat, this, ID_RESOURCE_PIXELFORMAT_BOX);

                mMinimizeSubdivisions = new wxCheckBox(settingsPanel, ID_RESOURCE_MINSUBDIVISION_BOX, "Minimize Texture Subdivisons");
                mMinimizeSubdivisions->SetValue(anImageData->mMinimizeSubdivisions);
                settingsSizer->Add(mMinimizeSubdivisions, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetImageMinSubdivision, this, ID_RESOURCE_MINSUBDIVISION_BOX);

                mDDSurface = new wxCheckBox(settingsPanel, ID_RESOURCE_DDSURFACE_BOX, "Use a DDSurface");
                mDDSurface->SetValue(anImageData->mDDSurface);
                settingsSizer->Add(mDDSurface, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetImageDDSurface, this, ID_RESOURCE_DDSURFACE_BOX);

                wxStaticLine* miscLine = new wxStaticLine(settingsPanel, wxID_ANY);
                settingsSizer->Add(miscLine, 0, wxEXPAND | wxALL, 5);

                mVariantField = new wxTextCtrl(settingsPanel, ID_RESOURCE_VARIANT_FIELD);
                wxBoxSizer* variantThing = new wxBoxSizer(wxHORIZONTAL);
                variantThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Variant:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                variantThing->Add(mVariantField, 1);
                settingsSizer->Add(variantThing, 0, wxEXPAND | wxALL, 5);
                mVariantField->SetValue(anImageData->mVariant);
                Bind(wxEVT_TEXT, &ResourceFrame::SetImageVariant, this, ID_RESOURCE_VARIANT_FIELD);

                wxStaticLine* animLine = new wxStaticLine(settingsPanel, wxID_ANY);
                settingsSizer->Add(animLine, 0, wxEXPAND | wxALL, 5);

                wxArrayString aAnimVec = { "none", "once", "loop" , "pingpong" };
                mAnimTypes = new wxChoice(settingsPanel, ID_RESOURCE_ANIMTYPE_BOX, wxDefaultPosition, wxDefaultSize, aAnimVec);
                wxString aAnimString = anImageData->mAnimationType;
                mAnimTypes->SetSelection(mAnimTypes->FindString(aAnimString));
                wxBoxSizer* animThing = new wxBoxSizer(wxHORIZONTAL);
                animThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Animation Type:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                animThing->Add(mAnimTypes, 1);
                settingsSizer->Add(animThing, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHOICE, &ResourceFrame::SetImageAnimType, this, ID_RESOURCE_ANIMTYPE_BOX);

                wxSpinCtrl* aStepperFrameDelay= new wxSpinCtrl(settingsPanel, ID_RESOURCE_FRAMEDELAY_STEP);
                aStepperFrameDelay->SetMin(-1);
                wxBoxSizer* frameThing1 = new wxBoxSizer(wxHORIZONTAL);
                frameThing1->Add(new wxStaticText(settingsPanel, wxID_ANY, "Frame Delay:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                frameThing1->Add(aStepperFrameDelay, 1);
                settingsSizer->Add(frameThing1, 0, wxEXPAND | wxALL, 5);
                aStepperFrameDelay->SetValue(anImageData->mFrameDelay);
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetImageFrameDelay, this, ID_RESOURCE_FRAMEDELAY_STEP);

                wxSpinCtrl* aStepperBeginDelay= new wxSpinCtrl(settingsPanel, ID_RESOURCE_BEGINDELAY_STEP);
                aStepperBeginDelay->SetMin(-1);
                wxBoxSizer* frameThing2 = new wxBoxSizer(wxHORIZONTAL);
                frameThing2->Add(new wxStaticText(settingsPanel, wxID_ANY, "Begin Delay:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                frameThing2->Add(aStepperBeginDelay, 1);
                settingsSizer->Add(frameThing2, 0, wxEXPAND | wxALL, 5);
                aStepperBeginDelay->SetValue(anImageData->mBeginDelay);
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetImageBeginFrameDelay, this, ID_RESOURCE_BEGINDELAY_STEP);

                wxSpinCtrl* aStepperEndDelay = new wxSpinCtrl(settingsPanel, ID_RESOURCE_ENDDELAY_STEP);
                aStepperEndDelay->SetMin(-1);
                wxBoxSizer* frameThing3 = new wxBoxSizer(wxHORIZONTAL);
                frameThing3->Add(new wxStaticText(settingsPanel, wxID_ANY, "End Delay:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                frameThing3->Add(aStepperEndDelay, 1);
                settingsSizer->Add(frameThing3, 0, wxEXPAND | wxALL, 5);
                aStepperEndDelay->SetValue(anImageData->mEndDelay);
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetImageEndDelay, this, ID_RESOURCE_ENDDELAY_STEP);

                mPerFrameDelayField = new wxTextCtrl(settingsPanel, ID_RESOURCE_PERFRAMEDELAY_FIELD);
                wxBoxSizer* frameThing4 = new wxBoxSizer(wxHORIZONTAL);
                frameThing4->Add(new wxStaticText(settingsPanel, wxID_ANY, "Per Frame Delay (1,2,3,4,....):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                frameThing4->Add(mPerFrameDelayField, 1);
                settingsSizer->Add(frameThing4, 0, wxEXPAND | wxALL, 5);
                mPerFrameDelayField->SetValue(anImageData->mPerFrameDelay);
                Bind(wxEVT_TEXT, &ResourceFrame::SetImagePerFrameDelay, this, ID_RESOURCE_PERFRAMEDELAY_FIELD);

                mFrameMapField = new wxTextCtrl(settingsPanel, ID_RESOURCE_FRAMEMAP_FIELD);
                wxBoxSizer* frameThing5 = new wxBoxSizer(wxHORIZONTAL);
                frameThing5->Add(new wxStaticText(settingsPanel, wxID_ANY, "Frame Map (1,4,-3,0,....):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                frameThing5->Add(mFrameMapField, 1);
                settingsSizer->Add(frameThing5, 0, wxEXPAND | wxALL, 5);
                mFrameMapField->SetValue(anImageData->mFrameMap);
                Bind(wxEVT_TEXT, &ResourceFrame::SetImageFrameMap, this, ID_RESOURCE_FRAMEMAP_FIELD);

                if (mResourceManifest.mFrameworkVersion == FrameworkVersion::VERSION_RESODDEDFRAMEWORK)
                {
                    mDDSurface->SetLabel("Use a GPUSurface");
                    mPalletize->Disable();
                    mMinimizeSubdivisions->Disable();
                }


                break;
            }
            case ResourceSubType::TYPE_DEFAULT_SETTINGS:
            {
                settingsSizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Default Settings"), 0);
                mIDPrefixField = new wxTextCtrl(settingsPanel, ID_RESOURCE_IDPREFIX_FIELD);
                wxBoxSizer* prefixThing = new wxBoxSizer(wxHORIZONTAL);
                prefixThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "ID Prefix:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                prefixThing->Add(mIDPrefixField, 1);
                wxBoxSizer* pathThing = new wxBoxSizer(wxHORIZONTAL);
                pathThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Path:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                pathThing->Add(mPathField, 1);
                settingsSizer->Add(pathThing, 0, wxEXPAND | wxALL, 5);
                settingsSizer->Add(prefixThing, 0, wxEXPAND | wxALL, 5);
                mIDPrefixField->SetValue(mResourceManifest.mGroupMap[aResourceData->mParent].GetDefaultSettings(aResourceData->mID)->mIDPrefix);
                Bind(wxEVT_TEXT, &ResourceFrame::SetDefaultSettingsIDPrefix, this, ID_RESOURCE_IDPREFIX_FIELD);
                break;
            }
            case ResourceSubType::TYPE_SOUND:
            {
                ResourceSound* aSoundData = mResourceManifest.mGroupMap[aResourceData->mParent].GetSound(aResourceData->mID).get();

                settingsSizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Sound Settings"), 0);
                wxBoxSizer* pathThing = new wxBoxSizer(wxHORIZONTAL);
                pathThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Path:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                pathThing->Add(mPathField, 1);
                settingsSizer->Add(pathThing, 0, wxEXPAND | wxALL, 5);


                wxSpinCtrl* aStepperVol = new wxSpinCtrl(settingsPanel, ID_RESOURCE_VOLUME_STEP);
                aStepperVol->SetMin(-1);
                aStepperVol->SetMax(100);
                wxBoxSizer* colThing = new wxBoxSizer(wxHORIZONTAL);
                colThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Volume (0-100):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                colThing->Add(aStepperVol, 1);
                settingsSizer->Add(colThing, 0, wxEXPAND | wxALL, 5);
                aStepperVol->SetValue(aSoundData->mVolume * 100); //Do the 0-100 range
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetSoundVolume, this, ID_RESOURCE_VOLUME_STEP);

                wxSpinCtrl* aStepperPan = new wxSpinCtrl(settingsPanel, ID_RESOURCE_PANNING_STEP);
                aStepperPan->SetMin(-1000); //todo: determine max bounds of panning
                aStepperPan->SetMax(1000);
                wxBoxSizer* rowThing = new wxBoxSizer(wxHORIZONTAL);
                rowThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Panning:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                rowThing->Add(aStepperPan, 1);
                settingsSizer->Add(rowThing, 0, wxEXPAND | wxALL, 5);
                aStepperPan->SetValue(aSoundData->mPanning * 100);
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetSoundPanning, this, ID_RESOURCE_PANNING_STEP);

                break;
            }
            case ResourceSubType::TYPE_FONT:
            {
                ResourceFont* aFontData = mResourceManifest.mGroupMap[aResourceData->mParent].GetFont(aResourceData->mID).get();

                settingsSizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Font Settings"), 0);
                wxBoxSizer* pathThing = new wxBoxSizer(wxHORIZONTAL);
                pathThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Path:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                pathThing->Add(mPathField, 1);
                settingsSizer->Add(pathThing, 0, wxEXPAND | wxALL, 5);


                mIsSystemFont = new wxCheckBox(settingsPanel, ID_RESOURCE_FONT_SYS_BOX, "System Font");
                mIsSystemFont->SetValue(aFontData->mIsSystemFont);
                settingsSizer->Add(mIsSystemFont, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetFontSys, this, ID_RESOURCE_FONT_SYS_BOX);

                wxStaticLine* settingsLine = new wxStaticLine(settingsPanel, wxID_ANY);
                settingsSizer->Add(settingsLine, 0, wxEXPAND | wxALL, 5);

                mBold = new wxCheckBox(settingsPanel, ID_RESOURCE_FONT_BOLD_BOX, "Bold");
                mBold->SetValue(aFontData->mBold);
                settingsSizer->Add(mBold, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetFontBold, this, ID_RESOURCE_FONT_BOLD_BOX);

                mItalic = new wxCheckBox(settingsPanel, ID_RESOURCE_FONT_ITALIC_BOX, "Italic");
                mItalic->SetValue(aFontData->mItalic);
                settingsSizer->Add(mItalic, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetFontItalic, this, ID_RESOURCE_FONT_ITALIC_BOX);

                mShadow = new wxCheckBox(settingsPanel, ID_RESOURCE_FONT_SHADOW_BOX, "Shadow");
                mShadow->SetValue(aFontData->mShadow);
                settingsSizer->Add(mShadow, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetFontShadow, this, ID_RESOURCE_FONT_SHADOW_BOX);

                mUnderline = new wxCheckBox(settingsPanel, ID_RESOURCE_FONT_UNDERLINE_BOX, "Underline");
                mUnderline->SetValue(aFontData->mUnderline);
                settingsSizer->Add(mUnderline, 0, wxEXPAND | wxALL, 5);
                Bind(wxEVT_CHECKBOX, &ResourceFrame::SetFontUnderline, this, ID_RESOURCE_FONT_UNDERLINE_BOX);

                mStepperSize = new wxSpinCtrl(settingsPanel, ID_RESOURCE_FONT_SIZE_STEP);
                mStepperSize->SetMin(1);
                wxBoxSizer* rowThing = new wxBoxSizer(wxHORIZONTAL);
                rowThing->Add(new wxStaticText(settingsPanel, wxID_ANY, "Size:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
                rowThing->Add(mStepperSize, 1);
                settingsSizer->Add(rowThing, 0, wxEXPAND | wxALL, 5);
                mStepperSize->SetValue(aFontData->mSize);
                Bind(wxEVT_SPINCTRL, &ResourceFrame::SetFontSize, this, ID_RESOURCE_FONT_SIZE_STEP);

                if (aFontData->mIsSystemFont)
                {
                    mBold->Enable();
                    mItalic->Enable();
                    mShadow->Enable();
                    mUnderline->Enable();
                    mStepperSize->Enable();
                }
                else
                {
                    mBold->Disable();
                    mItalic->Disable();
                    mShadow->Disable();
                    mUnderline->Disable();
                    mStepperSize->Disable();
                }
                break;
            }
            }
            
            settingsPanel->SetSizer(settingsSizer);
            settingsPanel->FitInside();

            if (aResourceData->mSubType != ResourceSubType::TYPE_DEFAULT_SETTINGS) //This means we can have a preview of the content
            {

                std::shared_ptr<DefaultSettings> aLastDefaultSettings;
                wxTreeItemId aParentGroup = mResourceTree->GetItemParent(mCurrentResource);
                wxTreeItemIdValue cookie;
                wxTreeItemId aResource = mResourceTree->GetFirstChild(aParentGroup, cookie);

                while (HasNext(mResourceTree, aResource))
                {
                    if (aResource.IsOk())
                    {
                        ResourceItemData* aResData = (ResourceItemData*)mResourceTree->GetItemData(aResource);
                        if (aResData->mSubType == ResourceSubType::TYPE_DEFAULT_SETTINGS)
                        {
                            aLastDefaultSettings = mResourceManifest.mGroupMap[aResourceData->mParent].GetDefaultSettings(aResData->mID);
                        }
                    }
                    aResource = mResourceTree->GetNextSibling(aResource);
                    if (aResource == mCurrentResource)
                        break;
                }

                std::string possiblePath = mAssetRoot + "/";
                if (aLastDefaultSettings.get())
                {
                    possiblePath = possiblePath + aLastDefaultSettings->mPath + "/";
                }
                possiblePath = possiblePath + mResourceManifest.mGroupMap[aResourceData->mParent].GetResource(aResourceData->mID)->mPath;

                if (aResourceData->mSubType == ResourceSubType::TYPE_IMAGE)
                {
                    auto anIMGData = mResourceManifest.mGroupMap[aResourceData->mParent].GetImage(aResourceData->mID);
                    wxStaticBoxSizer* previewSizer = new wxStaticBoxSizer(wxHORIZONTAL, mRightPanel, "Preview");

                    wxWindow* box = previewSizer->GetStaticBox();
                    sizer->Add(previewSizer, 1, wxEXPAND | wxALL, 5);

                    wxImage img;

                    const char* exts[] = { ".png", ".jpeg", ".jpg", ".gif" };
                    wxString finalPath;
                    for (auto ext : exts)
                    {
                        if (std::filesystem::exists(possiblePath + ext))
                        {
                            finalPath = possiblePath + ext;
                            break;
                        }
                    }
                    if (!finalPath.empty())
                    {
                        img.LoadFile(finalPath);
                        wxBitmap bmp(img);
                        mPreviewImage = new PreviewImage(box);
                        mPreviewImage->SetImage(bmp);
                        previewSizer->Add(mPreviewImage, wxSizerFlags(1).Border(wxALL, 5));
                    }

                    if (mPreviewImage != nullptr)
                        mPreviewImage->SetGridSize(anIMGData->mRows, anIMGData->mCols);
                }
                else if (aResourceData->mSubType == ResourceSubType::TYPE_SOUND)
                {
                    if (std::filesystem::exists(possiblePath + ".mp3"))
                    {
                        possiblePath += ".mp3";
                    }
                    else if (std::filesystem::exists(possiblePath + ".ogg"))
                    {
                        possiblePath += ".ogg";
                    }
                    else if (std::filesystem::exists(possiblePath + ".wav"))
                    {
                        possiblePath += ".wav";
                    }
                    else if (std::filesystem::exists(possiblePath + ".flac"))
                    {
                        possiblePath += ".flac";
                    }
                    else if (std::filesystem::exists(possiblePath + ".au"))
                    {
                        possiblePath += ".au";
                    }
                }
                wxButton* anExternalBttn = new wxButton(settingsPanel, wxID_ANY, "Open externally");
                anExternalBttn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&)
                {
                    wxLaunchDefaultApplication(possiblePath);
                });
                settingsSizer->Add(anExternalBttn, 0, wxALL, 5);
            }

            sizer->Add(settingsPanel, 2, wxEXPAND | wxALL, 5);

            mRightPanel->SetSizer(sizer);
            mRightPanel->Layout();
            break;
        }
        case ResourceType::TYPE_ROOT:
        default:
            break;
    }

    mRightPanel->Thaw();
}

void ResourceFrame::OnTreeRightClick(wxTreeEvent& event)
{
    wxTreeItemId hoveredItem = event.GetItem();

    mResourceTree->SelectItem(hoveredItem);

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(hoveredItem);

    wxMenu menu;
    
    if (data->mType == ResourceType::TYPE_ROOT)
    {
        menu.Append(ID_TREE_ADD_GROUP, "Add Group");
    }
    else if (data->mType == ResourceType::TYPE_GROUP)
    {
        menu.Append(ID_TREE_ADD_IMAGE, "Add Image");
        menu.Append(ID_TREE_ADD_SOUND, "Add Sound");
        menu.Append(ID_TREE_ADD_FONT, "Add Font");
        menu.Append(ID_TREE_ADD_DEFAULT_SETTINGS, "Add Default Settings");
        menu.AppendSeparator();
        menu.Append(ID_TREE_RENAME_ITEM, "Rename");
        menu.Append(ID_TREE_REMOVE_ITEM, "Delete");
    }
    else
    {
        mSwappingItems = { data->mParent, data->mID };
        menu.Append(ID_TREE_MOVE_UP_ITEM, "Move Up");
        menu.Append(ID_TREE_MOVE_DOWN_ITEM, "Move Down");
        menu.AppendSeparator();
        menu.Append(ID_TREE_RENAME_ITEM, "Rename");
        menu.Append(ID_TREE_REMOVE_ITEM, "Delete");
    }

    Bind(wxEVT_MENU, &ResourceFrame::AddGroup, this, ID_TREE_ADD_GROUP);
    Bind(wxEVT_MENU, &ResourceFrame::AddImage, this, ID_TREE_ADD_IMAGE);
    Bind(wxEVT_MENU, &ResourceFrame::AddSound, this, ID_TREE_ADD_SOUND);
    Bind(wxEVT_MENU, &ResourceFrame::AddFont, this, ID_TREE_ADD_FONT);
    Bind(wxEVT_MENU, &ResourceFrame::AddDefaultSettings, this, ID_TREE_ADD_DEFAULT_SETTINGS);
    Bind(wxEVT_MENU, &ResourceFrame::DeleteItem, this, ID_TREE_REMOVE_ITEM);
    Bind(wxEVT_MENU, &ResourceFrame::RenameItem, this, ID_TREE_RENAME_ITEM);
    Bind(wxEVT_MENU, &ResourceFrame::MoveUpItem, this, ID_TREE_MOVE_UP_ITEM);
    Bind(wxEVT_MENU, &ResourceFrame::MoveDownItem, this, ID_TREE_MOVE_DOWN_ITEM);

    PopupMenu(&menu);
}

void ResourceFrame::AddGroup(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    wxTreeItemId newItem = mResourceTree->AppendItem(root, "GROUP_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData("GROUP_NEW", ResourceType::TYPE_GROUP));
    mItems["GROUP_NEW"] = newItem;
    mResourceManifest.AddGroup("GROUP_NEW");
}

void ResourceFrame::AddImage(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(root);

    wxTreeItemId newItem = mResourceTree->AppendItem(root, "IMAGE_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData("IMAGE_NEW", ResourceType::TYPE_RESOURCE, data->mID, ResourceSubType::TYPE_IMAGE));

    mItems[data->mID + "/IMAGE_NEW"] = newItem;
    mResourceManifest.AddImage(data->mID, "IMAGE_NEW");
}

void ResourceFrame::AddSound(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(root);
    wxTreeItemId newItem = mResourceTree->AppendItem(root, "SOUND_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData("SOUND_NEW", ResourceType::TYPE_RESOURCE, data->mID, ResourceSubType::TYPE_SOUND));

    mItems[data->mID + "/SOUND_NEW"] = newItem;
    mResourceManifest.AddSound(data->mID, "SOUND_NEW");
}

void ResourceFrame::AddFont(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(root);
    wxTreeItemId newItem = mResourceTree->AppendItem(root, "FONT_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData("FONT_NEW", ResourceType::TYPE_RESOURCE, data->mID, ResourceSubType::TYPE_FONT));

    mItems[data->mID + "/FONT_NEW"] = newItem;
    mResourceManifest.AddFont(data->mID, "FONT_NEW");
}

void ResourceFrame::AddDefaultSettings(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(root);
    wxTreeItemId newItem = mResourceTree->AppendItem(root, "NEW_SETTINGS");
    mResourceTree->SetItemData(newItem, new ResourceItemData("NEW_SETTINGS", ResourceType::TYPE_RESOURCE, data->mID, ResourceSubType::TYPE_DEFAULT_SETTINGS));

    mItems[data->mID + "/NEW_SETTINGS"] = newItem;
    mResourceManifest.AddDefaultSettings(data->mID, "NEW_SETTINGS");
}

void ResourceFrame::DeleteItem(wxCommandEvent& event)
{
    wxTreeItemId item = mResourceTree->GetSelection();
    if (item == mResourceTree->GetRootItem())
        return;
    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(item);
    mResourceManifest.DeleteItem(data->mParent, data->mID);
    mItems.erase(data->mID);
    mResourceTree->Delete(item);
}

void ResourceFrame::RenameItem(wxCommandEvent& event)
{
    wxTreeItemId item = mResourceTree->GetSelection();
    if (item == mResourceTree->GetRootItem())
        return;
    mResourceTree->EditLabel(item);
}

void ResourceFrame::MoveUpItem(wxCommandEvent& event)
{
    wxTreeItemId selected = mResourceTree->GetSelection();

    if (!HasPrevious(mResourceTree, selected))
    {
        wxLogMessage("Already at top of group");
        return;
    }
    auto it = std::find_if(mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.begin(), mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.end(),
        [&](const auto& p) { return p.first == mSwappingItems.second; });

    size_t index = SIZE_MAX;
    if (it != mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.end())
    {
        index = std::distance(mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.begin(), it);
    }
    if (index == SIZE_MAX)
        return;
    swap(mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap[index], mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.begin()[index - 1]);
    std::vector<std::string> aExpandedData;
    float scrollFactor = mResourceTree->GetScrollPos(wxVERTICAL);
    auto* aData = (ResourceItemData*)mResourceTree->GetItemData(selected);
    std::string selectedItem = aData->mParent + "/" + aData->mID;
    std::string selectedRoot = ((ResourceItemData*)mResourceTree->GetItemData(mResourceTree->GetItemParent(selected)))->mParent + "/" + aData->mParent;
    CollectExpandedItems(mResourceTree, mRoot, aExpandedData);
    mResourceTree->Freeze();
    mResourceTree->DeleteAllItems();
    mRoot = mResourceTree->AddRoot("Resource Project");
    mResourceTree->SetItemData(mRoot, new ResourceItemData("ROOT", ResourceType::TYPE_ROOT));

    for (auto group : mResourceManifest.mGroupMap)
    {
        AddGroupImpl(group.first);
        for (auto res : group.second.mResourceMap)
        {
            switch (res.second->mType)
            {
            case ResourceSubType::TYPE_IMAGE:
                AddImageImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_SOUND:
                AddSoundImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_FONT:
                AddFontImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_DEFAULT_SETTINGS:
                AddDefaultSettingsImpl(res.first, group.first);
                break;
            }

        }
    }
    for (auto data : aExpandedData)
    {
        wxTreeItemId aItem = FindItemByID(mResourceTree, mRoot, data);
        if (data == selectedRoot)
        {
            wxTreeItemId aChildItem = FindItemByID(mResourceTree, aItem, selectedItem);

            mResourceTree->ToggleItemSelection(aChildItem);
        }
        mResourceTree->Expand(aItem);
    }
    mResourceTree->SetScrollPos(wxVERTICAL, scrollFactor, false);
    mResourceTree->Thaw();
}

void ResourceFrame::MoveDownItem(wxCommandEvent& event)
{
    wxTreeItemId selected = mResourceTree->GetSelection();

    if (!HasNext(mResourceTree, selected))
    {
        wxLogMessage("Already at bottom of group");
        return;
    }
    auto it = std::find_if(mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.begin(), mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.end(),
        [&](const auto& p) { return p.first == mSwappingItems.second; });

    size_t index = SIZE_MAX;
    if (it != mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.end())
    {
        index = std::distance(mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap.begin(), it);
    }
    if (index == SIZE_MAX)
        return;

    swap(mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap[index], mResourceManifest.mGroupMap[mSwappingItems.first].mResourceMap[index + 1]);
    std::vector<std::string> aExpandedData;
    float scrollFactor = mResourceTree->GetScrollPos(wxVERTICAL);
    auto* aData = (ResourceItemData*)mResourceTree->GetItemData(selected);
    std::string selectedItem = aData->mParent + "/" + aData->mID;
    std::string selectedRoot = ((ResourceItemData*)mResourceTree->GetItemData(mResourceTree->GetItemParent(selected)))->mParent + "/" + aData->mParent;
    CollectExpandedItems(mResourceTree, mRoot, aExpandedData);
    mResourceTree->Freeze();
    mResourceTree->DeleteAllItems();
    mRoot = mResourceTree->AddRoot("Resource Project");
    mResourceTree->SetItemData(mRoot, new ResourceItemData("ROOT", ResourceType::TYPE_ROOT));

    for (auto group : mResourceManifest.mGroupMap)
    {
        AddGroupImpl(group.first);
        for (auto res : group.second.mResourceMap)
        {
            switch (res.second->mType)
            {
            case ResourceSubType::TYPE_IMAGE:
                AddImageImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_SOUND:
                AddSoundImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_FONT:
                AddFontImpl(res.first, group.first);
                break;
            case ResourceSubType::TYPE_DEFAULT_SETTINGS:
                AddDefaultSettingsImpl(res.first, group.first);
                break;
            }

        }
    }
    for (auto data : aExpandedData)
    {
        wxTreeItemId aItem = FindItemByID(mResourceTree, mRoot, data);
        if (data == selectedRoot)
        {
            wxTreeItemId aChildItem = FindItemByID(mResourceTree, aItem, selectedItem);

            mResourceTree->ToggleItemSelection(aChildItem);
        }
        mResourceTree->Expand(aItem);
    }
    mResourceTree->SetScrollPos(wxVERTICAL, scrollFactor, false);
    mResourceTree->Thaw();
}

void ResourceFrame::SetResourcePath(wxCommandEvent& event)
{
    ResourceItemData* aResourceData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aResource = mResourceManifest.mGroupMap[aResourceData->mParent].GetResource(aResourceData->mID);
    aResource->mPath = mPathField->GetValue();
}

void ResourceFrame::SetImageColumns(wxSpinEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mCols = event.GetValue();
    if (mPreviewImage != nullptr)
        mPreviewImage->SetGridSize(anImage->mRows, anImage->mCols);
}

void ResourceFrame::SetImageRow(wxSpinEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mRows = event.GetValue();
    if (mPreviewImage != nullptr)
        mPreviewImage->SetGridSize(anImage->mRows, anImage->mCols);
}

void ResourceFrame::SetImageAlphaMask(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mAlphaMask = mAlphaMaskField->GetValue();
}

void ResourceFrame::SetImageVariant(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mVariant = mVariantField->GetValue();
}

void ResourceFrame::SetImageAlphaGrid(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mAlphaGrid = mAlphaGridField->GetValue();
}

void ResourceFrame::SetImageAlphaColor(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mAlphaColor = mAlphaColor->GetValue();
}

void ResourceFrame::SetImageNoAlpha(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mNoAlpha = mNoAlpha->GetValue();
}

void ResourceFrame::SetImagePalletize(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mPalletize = mPalletize->GetValue();
}

void ResourceFrame::SetImageNoBits(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mNoBits = mNoBits->GetValue();
}

void ResourceFrame::SetImageNoBits3D(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mNoBits3D = mNoBits3D->GetValue();
}

void ResourceFrame::SetImageNoBits2D(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mNoBits2D = mNoBits2D->GetValue();
}

void ResourceFrame::SetImageDDSurface(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mDDSurface = mDDSurface->GetValue();
}

void ResourceFrame::SetImageMinSubdivision(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mMinimizeSubdivisions = mMinimizeSubdivisions->GetValue();
}

void ResourceFrame::SetImagePixelFormat(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mPixelFormat = mPixelFormats->GetString(mPixelFormats->GetSelection());
}

void ResourceFrame::SetImageAnimType(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mAnimationType = mAnimTypes->GetString(mAnimTypes->GetSelection());
}

void ResourceFrame::SetImageBeginFrameDelay(wxSpinEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mBeginDelay = event.GetValue();
}

void ResourceFrame::SetImageFrameDelay(wxSpinEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mFrameDelay = event.GetValue();
}

void ResourceFrame::SetImageEndDelay(wxSpinEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mEndDelay = event.GetValue();
}

void ResourceFrame::SetImagePerFrameDelay(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mPerFrameDelay = mPerFrameDelayField->GetValue();
}

void ResourceFrame::SetImageFrameMap(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetImage(anImageData->mID);
    anImage->mFrameMap = mFrameMapField->GetValue();
}

void ResourceFrame::SetDefaultSettingsIDPrefix(wxCommandEvent& event)
{
    ResourceItemData* anImageData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto anImage = mResourceManifest.mGroupMap[anImageData->mParent].GetDefaultSettings(anImageData->mID);
    anImage->mIDPrefix = mIDPrefixField->GetValue();
}

void ResourceFrame::SetSoundVolume(wxSpinEvent& event)
{
    ResourceItemData* aSoundData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aSound = mResourceManifest.mGroupMap[aSoundData->mParent].GetSound(aSoundData->mID);
    aSound->mVolume = event.GetValue() / 100;
}

void ResourceFrame::SetSoundPanning(wxSpinEvent& event)
{
    ResourceItemData* aSoundData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aSound = mResourceManifest.mGroupMap[aSoundData->mParent].GetSound(aSoundData->mID);
    aSound->mPanning = event.GetValue() / 100;
}

void ResourceFrame::SetFontSys(wxCommandEvent& event)
{
    ResourceItemData* aFontData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aFont = mResourceManifest.mGroupMap[aFontData->mParent].GetFont(aFontData->mID);
    aFont->mIsSystemFont = mIsSystemFont->GetValue();
    if (aFont->mIsSystemFont)
    {
        mBold->Enable();
        mItalic->Enable();
        mShadow->Enable();
        mUnderline->Enable();
        mStepperSize->Enable();
    }
    else
    {
        mBold->Disable();
        mItalic->Disable();
        mShadow->Disable();
        mUnderline->Disable();
        mStepperSize->Disable();
    }
    
}

void ResourceFrame::SetFontSize(wxSpinEvent& event)
{
    ResourceItemData* aFontData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aFont = mResourceManifest.mGroupMap[aFontData->mParent].GetFont(aFontData->mID);
    aFont->mSize = event.GetValue();
}

void ResourceFrame::SetFontBold(wxCommandEvent& event)
{
    ResourceItemData* aFontData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aFont = mResourceManifest.mGroupMap[aFontData->mParent].GetFont(aFontData->mID);
    aFont->mBold = mBold->GetValue();
}

void ResourceFrame::SetFontItalic(wxCommandEvent& event)
{
    ResourceItemData* aFontData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aFont = mResourceManifest.mGroupMap[aFontData->mParent].GetFont(aFontData->mID);
    aFont->mItalic = mItalic->GetValue();
}

void ResourceFrame::SetFontShadow(wxCommandEvent& event)
{
    ResourceItemData* aFontData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aFont = mResourceManifest.mGroupMap[aFontData->mParent].GetFont(aFontData->mID);
    aFont->mShadow = mShadow->GetValue();
}

void ResourceFrame::SetFontUnderline(wxCommandEvent& event)
{
    ResourceItemData* aFontData = (ResourceItemData*)mResourceTree->GetItemData(mCurrentResource);
    auto aFont = mResourceManifest.mGroupMap[aFontData->mParent].GetFont(aFontData->mID);
    aFont->mUnderline = mUnderline->GetValue();
}

void ResourceFrame::AddGroupImpl(std::string theName)
{
    wxTreeItemId newItem = mResourceTree->AppendItem(mRoot, theName.c_str());
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_GROUP));
    mItems[theName] = newItem;
}

void ResourceFrame::AddImageImpl(std::string theName, std::string theGroup)
{
    wxTreeItemId aGroupItem = mItems[theGroup];
    wxTreeItemId newItem = mResourceTree->AppendItem(aGroupItem, theName.c_str());
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_RESOURCE, theGroup, ResourceSubType::TYPE_IMAGE));
    mItems[theGroup + "/" + theName] = newItem;
}

void ResourceFrame::AddSoundImpl(std::string theName, std::string theGroup)
{
    wxTreeItemId aGroupItem = mItems[theGroup];
    wxTreeItemId newItem = mResourceTree->AppendItem(aGroupItem, theName.c_str());
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_RESOURCE, theGroup, ResourceSubType::TYPE_SOUND));
    mItems[theGroup + "/" + theName] = newItem;
}

void ResourceFrame::AddFontImpl(std::string theName, std::string theGroup)
{
    wxTreeItemId aGroupItem = mItems[theGroup];
    wxTreeItemId newItem = mResourceTree->AppendItem(aGroupItem, theName.c_str());
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_RESOURCE, theGroup, ResourceSubType::TYPE_FONT));
    mItems[theGroup + "/" + theName] = newItem;
}

void ResourceFrame::AddDefaultSettingsImpl(std::string theName, std::string theGroup)
{
    wxTreeItemId aGroupItem = mItems[theGroup];
    wxTreeItemId newItem = mResourceTree->AppendItem(aGroupItem, theName.c_str());
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_RESOURCE, theGroup, ResourceSubType::TYPE_DEFAULT_SETTINGS));
    mItems[theGroup + "/" + theName] = newItem;
}
