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
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);
    
    wxSplitterWindow* splitter = new wxSplitterWindow(this);

    wxPanel* leftPanel = new wxPanel(splitter);

    mResourceTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
    mResourceTree->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK,
        &ResourceFrame::OnTreeRightClick, this);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(mResourceTree, 1, wxEXPAND);
    leftPanel->SetSizer(leftSizer);

    wxPanel* rightPanel = new wxPanel(splitter);
    rightPanel->SetBackgroundColour(*wxWHITE);

    splitter->SplitVertically(leftPanel, rightPanel, 250);
    splitter->SetMinimumPaneSize(100);

    wxTreeItemId root = mResourceTree->AddRoot("Resource Project");
    mResourceTree->SetItemData(root, new ResourceItemData(ResourceType::TYPE_ROOT));
    mResourceTree->ExpandAll();

    CreateStatusBar();
    SetStatusText("Welcome to ResourceGen");
    Bind(wxEVT_MENU, &ResourceFrame::OnNewFile, this, ID_FILE_NEW);
    Bind(wxEVT_MENU, &ResourceFrame::OnOpenFile, this, ID_FILE_OPEN);
    Bind(wxEVT_MENU, &ResourceFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &ResourceFrame::OnExit, this, wxID_EXIT);
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
        menu.AppendSeparator();
        menu.Append(ID_TREE_REMOVE_ITEM, "Delete");
    }
    else
    {
        menu.Append(ID_TREE_REMOVE_ITEM, "Delete");
    }

    Bind(wxEVT_MENU, &ResourceFrame::AddGroup, this, ID_TREE_ADD_GROUP);
    Bind(wxEVT_MENU, &ResourceFrame::AddImage, this, ID_TREE_ADD_IMAGE);
    Bind(wxEVT_MENU, &ResourceFrame::AddSound, this, ID_TREE_ADD_SOUND);
    Bind(wxEVT_MENU, &ResourceFrame::DeleteItem, this, ID_TREE_REMOVE_ITEM);

    PopupMenu(&menu);
}

void ResourceFrame::AddGroup(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    wxTreeItemId newItem = mResourceTree->AppendItem(root, "GROUP_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData(ResourceType::TYPE_GROUP));
}
void ResourceFrame::AddImage(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    wxTreeItemId newItem = mResourceTree->AppendItem(root, "IMAGE_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData(ResourceType::TYPE_RESOURCE));
}

void ResourceFrame::AddSound(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (!root)
        return;

    wxTreeItemId newItem = mResourceTree->AppendItem(root, "SOUND_NEW");
    mResourceTree->SetItemData(newItem, new ResourceItemData(ResourceType::TYPE_RESOURCE));
}

void ResourceFrame::DeleteItem(wxCommandEvent& event)
{
    wxTreeItemId root = mResourceTree->GetSelection();
    if (root == mResourceTree->GetRootItem())
        return;

    mResourceTree->Delete(root);
}