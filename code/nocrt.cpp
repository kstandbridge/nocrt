#include "nocrt.h"
// TODO(kstandbridge): cpp files here

#define ID_BUTTON 1234

global_variable platform_api *Platform;

void CreateControls(platform_api *PlatformAPI)
{
    Assert(PlatformAPI);
    Platform = PlatformAPI;
    
    Platform->CreateControl(ControlType_Edit, "Hello", ID_BUTTON);
}

void HandleCommand(s64 Id)
{
    if(Id == ID_BUTTON)
    {
        Platform->DisplayMessage("Worked", "Yay");
    }
    else
    {
        Platform->DisplayMessage("Failed", "Boo");
    }
}

void __stdcall
_DllMainCRTStartup()
{
}