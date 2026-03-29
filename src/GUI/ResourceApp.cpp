#include "ResourceApp.h"
#include "ResourceFrame.h"

bool ResourceApp::OnInit()
{
    ResourceFrame* frame = new ResourceFrame();
    frame->Show(true);
    return true;
}