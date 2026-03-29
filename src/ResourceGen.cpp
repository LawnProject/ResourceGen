#include "GUI/ResourceApp.h"

int main(int argc, char** argv)
{
    wxDISABLE_DEBUG_SUPPORT();
    new ResourceApp;
    return wxEntry(argc, argv);
}