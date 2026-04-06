#include "ResourceFrame.h"
#include "ResourceItem.h"
#include <wx/treectrl.h>
#include <wx/splitter.h>

ResourceFrame::ResourceFrame()
    : wxFrame(NULL, wxID_ANY, "ResourceGen - LawnProject", wxDefaultPosition, wxSize(800, 500))
{
    wxMenu* menuFile = new wxMenu;
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
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);
    
    wxSplitterWindow* splitter = new wxSplitterWindow(this);

    wxPanel* leftPanel = new wxPanel(splitter);

    mResourceTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS);
    mResourceTree->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK,
        &ResourceFrame::OnTreeRightClick, this);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(mResourceTree, 1, wxEXPAND);
    leftPanel->SetSizer(leftSizer);

    wxPanel* rightPanel = new wxPanel(splitter);
    rightPanel->SetBackgroundColour(*wxWHITE);

    splitter->SplitVertically(leftPanel, rightPanel, 250);
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
    wxMessageBox("ResourceGen is a modernized version of PopCap's ResourceGen Tool made for SexyAppFramework\n \nModern Version (add ver here please?) - Electr0Gunner",
        "About ResourceGen", wxOK | wxICON_INFORMATION);
}

void ResourceFrame::OnNewFile(wxCommandEvent& event)
{
    wxLogMessage("THIS FEATURE ISN'T IMPLEMENTED YET\n - Electr0Gunner");
}

void ResourceFrame::OnOpenFile(wxCommandEvent& event)
{
    wxFileDialog openFileDialog(this, _("Open Resource file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    mResourceManifest.Import(openFileDialog.GetPath().ToStdString());

    for (auto group : mResourceManifest.mGroupMap)
    {
        AddGroupImpl(group.first);

        for (auto image : group.second.mImageMap)
        {
            AddImageImpl(image.first, group.first);
        }
        for (auto sound : group.second.mSoundMap)
        {
            AddSoundImpl(sound.first, group.first);
        }
        for (auto font : group.second.mFontMap)
        {
            AddFontImpl(font.first, group.first);
        }
    }
}

void ResourceFrame::OnItemRenameStart(wxTreeEvent& event)
{
    mItemStrEditingNow = event.GetLabel();
}

void ResourceFrame::OnItemRenameEnd(wxTreeEvent& event)
{
    std::string aNewName = event.GetLabel();

    wxTreeItemId hoveredItem = event.GetItem();

    mResourceTree->SelectItem(hoveredItem);

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(hoveredItem);

    if (data->type == ResourceType::TYPE_GROUP)
    {
        if (mResourceManifest.mGroupMap.count(aNewName))
        {
            wxLogError("You cannot name 2 groups the same");
            event.Veto();
            return;
        }
    }
    else if (data->type == ResourceType::TYPE_RESOURCE)
    {
        if (mResourceManifest.mGroupMap[data->parent].mImageMap.count(aNewName) || mResourceManifest.mGroupMap[data->parent].mSoundMap.count(aNewName) || mResourceManifest.mGroupMap[data->parent].mFontMap.count(aNewName))
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


    data->id = aNewName;

    mItems[aNewName] = mItems[mItemStrEditingNow];
    mItems.erase(mItemStrEditingNow);

    if (data->type == ResourceType::TYPE_GROUP)
    {
        mResourceManifest.mGroupMap[aNewName] = mResourceManifest.mGroupMap[mItemStrEditingNow];
        mResourceManifest.mGroupMap.erase(mItemStrEditingNow);
        wxTreeItemIdValue cookie;
        wxTreeItemId ch = mResourceTree->GetFirstChild(hoveredItem, cookie);
        while (ch.IsOk()) 
        {
            ResourceItemData* childData = (ResourceItemData*)mResourceTree->GetItemData(ch);
            childData->parent = aNewName;
            ch = mResourceTree->GetNextSibling(hoveredItem);
        }
    }
    else if (data->type == ResourceType::TYPE_RESOURCE)
    {
        if (mResourceManifest.mGroupMap[data->parent].mImageMap.count(mItemStrEditingNow))
        {
            mResourceManifest.mGroupMap[data->parent].mImageMap[aNewName] = mResourceManifest.mGroupMap[data->parent].mImageMap[mItemStrEditingNow];
            mResourceManifest.mGroupMap[data->parent].mImageMap.erase(mItemStrEditingNow);
        }
        else if (mResourceManifest.mGroupMap[data->parent].mSoundMap.count(mItemStrEditingNow))
        {
            mResourceManifest.mGroupMap[data->parent].mSoundMap[aNewName] = mResourceManifest.mGroupMap[data->parent].mSoundMap[mItemStrEditingNow];
            mResourceManifest.mGroupMap[data->parent].mSoundMap.erase(mItemStrEditingNow);
        }
        else if (mResourceManifest.mGroupMap[data->parent].mFontMap.count(mItemStrEditingNow))
        {
            mResourceManifest.mGroupMap[data->parent].mFontMap[aNewName] = mResourceManifest.mGroupMap[data->parent].mFontMap[mItemStrEditingNow];
            mResourceManifest.mGroupMap[data->parent].mFontMap.erase(mItemStrEditingNow);
        }
    }
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
    wxFileDialog saveFileDialog(this, _("Source files"), "", "", "", wxFD_SAVE);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxLogMessage("THIS FEATURE ISN'T IMPLEMENTED YET\n - Electr0Gunner");
}

void ResourceFrame::OnTreeRightClick(wxTreeEvent& event)
{
    wxTreeItemId hoveredItem = event.GetItem();

    mResourceTree->SelectItem(hoveredItem);

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(hoveredItem);

    wxMenu menu;
    
    if (data->type == ResourceType::TYPE_ROOT)
    {
        menu.Append(ID_TREE_ADD_GROUP, "Add Group");
    }
    else if (data->type == ResourceType::TYPE_GROUP)
    {
        menu.Append(ID_TREE_ADD_IMAGE, "Add Image");
        menu.Append(ID_TREE_ADD_SOUND, "Add Sound");
        menu.Append(ID_TREE_ADD_FONT, "Add Font");
        menu.AppendSeparator();
        menu.Append(ID_TREE_RENAME_ITEM, "Rename");
        menu.Append(ID_TREE_REMOVE_ITEM, "Delete");
    }
    else
    {
        menu.Append(ID_TREE_RENAME_ITEM, "Rename");
        menu.Append(ID_TREE_REMOVE_ITEM, "Delete");
    }

    Bind(wxEVT_MENU, &ResourceFrame::AddGroup, this, ID_TREE_ADD_GROUP);
    Bind(wxEVT_MENU, &ResourceFrame::AddImage, this, ID_TREE_ADD_IMAGE);
    Bind(wxEVT_MENU, &ResourceFrame::AddSound, this, ID_TREE_ADD_SOUND);
    Bind(wxEVT_MENU, &ResourceFrame::AddFont, this, ID_TREE_ADD_FONT);
    Bind(wxEVT_MENU, &ResourceFrame::DeleteItem, this, ID_TREE_REMOVE_ITEM);
    Bind(wxEVT_MENU, &ResourceFrame::RenameItem, this, ID_TREE_RENAME_ITEM);

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
    mResourceTree->SetItemData(newItem, new ResourceItemData("IMAGE_NEW", ResourceType::TYPE_RESOURCE, data->id));

    mItems["IMAGE_NEW"] = newItem;
    mResourceManifest.AddFont(data->id, "IMAGE_NEW");
}

void ResourceFrame::AddSound(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(root);
    wxTreeItemId newItem = mResourceTree->AppendItem(root, "SOUND_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData("SOUND_NEW", ResourceType::TYPE_RESOURCE, data->id));

    mItems["SOUND_NEW"] = newItem;
    mResourceManifest.AddSound(data->id, "SOUND_NEW");
}

void ResourceFrame::AddFont(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(root);
    wxTreeItemId newItem = mResourceTree->AppendItem(root, "FONT_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData("FONT_NEW", ResourceType::TYPE_RESOURCE, data->id));

    mItems["FONT_NEW"] = newItem;
    mResourceManifest.AddFont(data->id, "FONT_NEW");
}

void ResourceFrame::DeleteItem(wxCommandEvent& event)
{
    wxTreeItemId item = mResourceTree->GetSelection();
    if (item == mResourceTree->GetRootItem())
        return;
    ResourceItemData* data = (ResourceItemData*)mResourceTree->GetItemData(item);
    mResourceManifest.DeleteItem(data->id);
    mItems.erase(data->id);
    mResourceTree->Delete(item);

}

void ResourceFrame::RenameItem(wxCommandEvent& event)
{
    wxTreeItemId item = mResourceTree->GetSelection();
    if (item == mResourceTree->GetRootItem())
        return;
    mResourceTree->EditLabel(item);
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
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_RESOURCE));
    mItems[theName] = newItem;
}

void ResourceFrame::AddSoundImpl(std::string theName, std::string theGroup)
{
    wxTreeItemId aGroupItem = mItems[theGroup];
    wxTreeItemId newItem = mResourceTree->AppendItem(aGroupItem, theName.c_str());
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_RESOURCE));
    mItems[theName] = newItem;
}

void ResourceFrame::AddFontImpl(std::string theName, std::string theGroup)
{
    wxTreeItemId aGroupItem = mItems[theGroup];
    wxTreeItemId newItem = mResourceTree->AppendItem(aGroupItem, theName.c_str());
    mResourceTree->SetItemData(newItem, new ResourceItemData(theName, ResourceType::TYPE_RESOURCE));
    mItems[theName] = newItem;
}
