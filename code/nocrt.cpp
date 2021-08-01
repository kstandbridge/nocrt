#include "nocrt.h"
// TODO(kstandbridge): cpp files here


#define ID_MAIN            1000

#define ID_TOP_PANEL       1100
#define ID_TOP_LEFT_SPACER 1101
#define ID_TOP_STATIC      1102
#define ID_TOP_EDIT        1103
#define ID_TOP_BUTTON      1104

#define ID_BOTTOM_PANEL    1200

global_variable platform_api *Platform;

void
CreateControls(platform_api *PlatformAPI)
{
    Assert(PlatformAPI);
    Platform = PlatformAPI;
    
    Platform->AddPanel(ID_WINDOW, ID_MAIN, ControlLayout_Verticle);
    
    // NOTE(kstandbridge): Top bar
    Platform->AddPanel(ID_MAIN, ID_TOP_PANEL, ControlLayout_Horizontal);
    Platform->AddSpacer(ID_TOP_PANEL, SIZE_FILL);
    Platform->AddStatic(ID_TOP_PANEL, ID_TOP_STATIC, "Input:", 80.0f);
    Platform->AddEdit(ID_TOP_PANEL, ID_TOP_EDIT, "<enter here>", SIZE_FILL);
    Platform->AddButton(ID_TOP_PANEL, ID_TOP_BUTTON, "Submit", 80.0f);
    Platform->AddSpacer(ID_TOP_PANEL, SIZE_FILL);
    
    // NOTE(kstandbridge): Bottom fill
    Platform->AddPanel(ID_MAIN, ID_BOTTOM_PANEL, ControlLayout_Horizontal);
    /*
    Platform->AddListView(ID_BOTTOM_PANEL, ID_WORLD_LIST, SIZE_FILL);
    Platform->AddListViewColumn(ID_WORLD_LIST, "Date");
    Platform->AddListViewColumn(ID_WORLD_LIST, "Name");
    */
    //Platform->AddListViewProgressColumn(ID_WORLD_LIST, "Status");
    
}

void
HandleCommand(s64 Id)
{
    if(Id == ID_TOP_BUTTON)
    {
        char Buffer[128];
        Platform->GetControlText(ID_TOP_EDIT, Buffer, sizeof(Buffer));
        Platform->SetControlText(ID_BOTTOM_PANEL, Buffer);
        
    }
}
//
//void
//GetListViewData(s64 ControlId, s32 Column, s32 Row, char *OutputBuffer)
//{
//Output = "Foo bar";
//}
//
//
//r32
//GetListViewProgressData(s64 ControlId, s32 Column, s32 Row, char *OutputBuffer)
//{
//r32 Result = 0.75f;
//
//Output = "Downloading... (75%)";
//
//return(Result);
//}
//