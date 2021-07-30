#include "nocrt.h"
// TODO(kstandbridge): cpp files here


#define ID_MAIN 1000
#define ID_TOP_PANEL 1100
#define ID_TOP_STATIC 1101
#define ID_TOP_EDIT 1102
#define ID_TOP_BUTTON 1103
#define ID_BOTTOM_PANEL 1200

global_variable platform_api *Platform;

void
CreateControls(platform_api *PlatformAPI)
{
    Assert(PlatformAPI);
    Platform = PlatformAPI;
    
    Platform->CreateControl(ID_WINDOW, ID_MAIN, ControlType_Static, "");
    Platform->SetControlLayout(ID_MAIN, ControlLayout_Verticle);
    
    Platform->CreateControl(ID_MAIN, ID_TOP_PANEL, ControlType_Static, "TOP HERE");
    Platform->CreateControl(ID_TOP_PANEL, ID_TOP_STATIC, ControlType_Static, "LEFT Input:");
    Platform->CreateControl(ID_TOP_PANEL, ID_TOP_EDIT, ControlType_Edit, "MIDDLE some text");
    Platform->CreateControl(ID_TOP_PANEL, ID_TOP_BUTTON, ControlType_Button, "RIGHT Submit");
    
    Platform->CreateControl(ID_MAIN, ID_BOTTOM_PANEL, ControlType_Static, "BOTTOM HERE");
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
