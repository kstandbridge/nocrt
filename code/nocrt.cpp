#include "nocrt.h"
// TODO(kstandbridge): cpp files here

#define ID_BUTTON 1234

#define ID_TOP_PANEL 1000
#define ID_TOP_STATIC 1001
#define ID_TOP_EDIT 1002
#define ID_TOP_BUTTON 1003

#define ID_BOTTOM_PANEL 2000

global_variable platform_api *Platform;

void
CreateControls(platform_api *PlatformAPI)
{
    Assert(PlatformAPI);
    Platform = PlatformAPI;
    
    //Platform->CreateControl(ID_WINDOW, ID_TOP_PANEL, ControlType_Static, "");
    Platform->CreateControl(ID_WINDOW, ID_TOP_STATIC, ControlType_Static, "Input:");
    Platform->CreateControl(ID_WINDOW, ID_TOP_EDIT, ControlType_Edit, "something else");
    Platform->CreateControl(ID_WINDOW, ID_TOP_BUTTON, ControlType_Button, "Submit");
    
    //Platform->CreateControl(ID_WINDOW, ID_BOTTOM_PANEL, ControlType_Static, "");
}

void
HandleCommand(s64 Id)
{
    if(Id == ID_TOP_BUTTON)
    {
        
        char Buffer[128];
        Platform->GetControlText(ID_TOP_EDIT, Buffer, sizeof(Buffer));
        Platform->SetControlText(ID_TOP_STATIC, Buffer);
        
    }
}


void __stdcall
_DllMainCRTStartup()
{
}
