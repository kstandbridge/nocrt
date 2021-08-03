#include "nocrt_platform.h"

#include <windows.h>

#include "win32_resource.h"
#include "win32_nocrt.h"

global_variable win_32 Win32;
global_variable win32_state Win32State;
global_variable win32_app_code GlobalApp;

global_variable s64 GlobalPerfCountFrequency;

internal void
Win32InitAPI(win_32 *Result)
{
    HMODULE Gdi32Library = LoadLibraryA("Gdi32.dll");
    Assert(Gdi32Library);
    
    HMODULE User32Library = LoadLibraryA("User32.dll");
    Assert(User32Library);
    
    HMODULE WinmmLibrary = LoadLibraryA("Winmm.dll");
    Assert(WinmmLibrary);
    
    Result->CreateWindowExA = (create_window_ex_a *)GetProcAddress(User32Library, "CreateWindowExA");
    Assert(Result->CreateWindowExA);
    
    Result->DefWindowProcA = (def_window_proc_a *)GetProcAddress(User32Library, "DefWindowProcA");
    Assert(Result->DefWindowProcA);
    
    Result->DestroyWindow = (destroy_window *)GetProcAddress(User32Library, "DestroyWindow");
    Assert(Result->DestroyWindow);
    
    Result->DispatchMessageA = (dispatch_message_a *)GetProcAddress(User32Library, "DispatchMessageA");
    Assert(Result->DispatchMessageA);
    
    Result->GetClientRect = (get_client_rect *)GetProcAddress(User32Library, "GetClientRect");
    Assert(Result->GetClientRect);
    
    Result->GetDlgItem = (get_dlg_item *)GetProcAddress(User32Library, "GetDlgItem");
    Assert(Result->GetDlgItem);
    
    Result->GetParent = (get_partent *)GetProcAddress(User32Library, "GetParent");
    Assert(Result->GetParent);
    
    Result->GetStockObject = (get_stock_object *)GetProcAddress(Gdi32Library, "GetStockObject");
    Assert(Result->GetStockObject);
    
    Result->GetWindowTextA = (get_window_text_a *)GetProcAddress(User32Library, "GetWindowTextA");
    Assert(Result->GetWindowTextA);
    
    Result->InvalidateRect = (invalidate_rect *)GetProcAddress(User32Library, "InvalidateRect");
    Assert(Result->InvalidateRect);
    
    Result->LoadCursorA = (load_cursor_a *)GetProcAddress(User32Library, "LoadCursorA");
    Assert(Result->LoadCursorA);
    
    Result->PeekMessageA = (peek_message_a *)GetProcAddress(User32Library, "PeekMessageA");
    Assert(Result->PeekMessageA);
    
    Result->RegisterClassA = (register_class_a *)GetProcAddress(User32Library, "RegisterClassA");
    Assert(Result->RegisterClassA);
    
    Result->MapWindowPoints = (map_window_points *)GetProcAddress(User32Library, "MapWindowPoints");
    Assert(Result->MapWindowPoints);
    
    Result->MessageBoxA = (message_box_a *)GetProcAddress(User32Library, "MessageBoxA");
    Assert(Result->MessageBoxA);
    
    Result->MoveWindow = (move_window *)GetProcAddress(User32Library, "MoveWindow");
    Assert(Result->MoveWindow);
    
    Result->SendMessageA = (send_message_a *)GetProcAddress(User32Library, "SendMessageA");
    Assert(Result->SendMessageA);
    
    Result->SetWindowTextA = (set_window_text_a *)GetProcAddress(User32Library, "SetWindowTextA");
    Assert(Result->SetWindowTextA);
    
    Result->TimeBeginPeriod = (time_begin_period *)GetProcAddress(WinmmLibrary, "timeBeginPeriod");
    Assert(Result->TimeBeginPeriod);
    
    Result->TranslateMessage = (translate_message *)GetProcAddress(User32Library, "TranslateMessage");
    Assert(Result->TranslateMessage);
}

internal void
Win32DestroyControls(control *Controls)
{
    for(control *Control = Controls;
        Control;
        )
    {
        if(Control->Hwnd)
        {
            Assert(Win32.DestroyWindow(Control->Hwnd));
            if(Control->Children)
            {
                Win32DestroyControls(Control->Children);
            }
            Control->Id = 0;
            Control->Hwnd = 0;
            Control->Type = (control_type)0;
            Control->Children = 0;
            control *NextControl = Control->NextControl;
            control *DeletedControl = Control;
            DeletedControl->NextControl = Win32State.FirstFreeControl;
            Win32State.FirstFreeControl = DeletedControl;
            
            Control = NextControl;
        }
    }
    Win32State.SentinalControl.Children = 0;
}

internal control *
GetControlById(control *Controls, s64 ControlId)
{
    control *Result = 0;
    
    for(control *Control = Controls;
        Control;
        Control = Control->NextControl)
    {
        if(Control->Children)
        {
            Result = GetControlById(Control->Children, ControlId);
            if(Result)
            {
                break;
            }
        }
        if(Control->Id == ControlId)
        {
            Result = Control;
            break;
        }
    }
    
    return(Result);
}

internal void
Win32SizeControls_(control *Controls)
{
    // NOTE(kstandbridge): Get control count
    r32 ControlCount = 0;
    r32 ControlWithoutSize = 0;
    HWND ParentHwnd = 0;
    control *ParentControl = 0;
    r32 TotalSize = 0.0f;
    for(control *Control = Controls;
        Control;
        Control = Control->NextControl)
    {
        Assert((ParentHwnd == 0) || (ParentHwnd == Control->ParentHwnd));
        ParentHwnd = Control->ParentHwnd;
        if(Control->ParentId != ID_WINDOW)
        {            
            if(!ParentControl)
            {
                ParentControl = GetControlById(Win32State.SentinalControl.Children, Control->ParentId);
                Assert(ParentControl);
            }
        }
        if(Control->Size == SIZE_FILL)
        {
            ++ControlWithoutSize;
        }
        else
        {
            TotalSize += Control->Size;
        }
        ++ControlCount;
    }
    
    if(ControlCount > 0)
    {
        RECT Rect = {};
        Win32.GetClientRect(ParentHwnd, &Rect);
        if(ParentHwnd != Win32State.WindowHwnd)
        {
            HWND GrandParentHwnd = Win32.GetParent(ParentHwnd);
            Assert(GrandParentHwnd);
            Win32.MapWindowPoints(ParentHwnd, GrandParentHwnd, (POINT *)&Rect, 2);
        }
        
        r32 X;
        r32 Y;
        r32 Width;
        r32 Height;
        
        b32 IsVerticleLayout = (ParentControl && ParentControl->Layout == ControlLayout_Verticle);
        if(IsVerticleLayout)
        {
            if(ParentControl)
            {
                X = (r32)Rect.left + ParentControl->PaddingLeft;
                Y = (r32)Rect.bottom - ParentControl->PaddingBottom;
                Width = (r32)Rect.right - (r32)Rect.left - ParentControl->PaddingRight - ParentControl->PaddingLeft;
                Height = ((r32)Rect.bottom - ParentControl->PaddingBottom - (r32)Rect.top - TotalSize - ParentControl->PaddingTop)/ControlWithoutSize;
            }
            else
            {
                X = (r32)Rect.left;
                Y = (r32)Rect.bottom;
                Width = (r32)Rect.right - (r32)Rect.left;
                Height = ((r32)Rect.bottom - (r32)Rect.top - TotalSize)/ControlWithoutSize;
            }
        }
        else
        {
            if(ParentControl)
            {                
                X = (r32)Rect.right - ParentControl->PaddingRight;
                Y = (r32)Rect.top + ParentControl->PaddingTop;
                Width = ((r32)Rect.right - ParentControl->PaddingRight - (r32)Rect.left - TotalSize - ParentControl->PaddingLeft)/ControlWithoutSize;
                Height = (r32)Rect.bottom - (r32)Rect.top - ParentControl->PaddingBottom - ParentControl->PaddingTop;
            }
            else
            {                
                X = (r32)Rect.right;
                Y = (r32)Rect.top;
                Width = ((r32)Rect.right - (r32)Rect.left - TotalSize)/ControlWithoutSize;
                Height = (r32)Rect.bottom - (r32)Rect.top;
            }
            
        }
        
        for(control *Control = Controls;
            Control;
            Control = Control->NextControl)
        {
            --ControlCount;
            if(IsVerticleLayout)
            {
                r32 CalcHeight = (Control->Size != SIZE_FILL) ? Control->Size : Height;
                Y -= CalcHeight;
                Win32.MoveWindow(Control->Hwnd, 
                                 (s32)(X + Control->MarginLeft), 
                                 (s32)(Y + Control->MarginTop), 
                                 (s32)(Width - Control->MarginLeft - Control->MarginRight), 
                                 (s32)(CalcHeight - Control->MarginTop - Control->MarginBottom), 
                                 FALSE);
                
            }
            else
            {
                r32 CalcWidth = (Control->Size != SIZE_FILL) ? Control->Size : Width;
                X -= CalcWidth;
                Win32.MoveWindow(Control->Hwnd, 
                                 (s32)(X + Control->MarginLeft), 
                                 (s32)(Y + Control->MarginTop), 
                                 (s32)(CalcWidth - Control->MarginLeft - Control->MarginRight),
                                 (s32)(Height - Control->MarginTop - Control->MarginBottom), 
                                 FALSE);
            }
            
            if(Control->Children)
            {
                Win32SizeControls_(Control->Children);
            }
        }
    }
}

internal void
Win32SizeControls(control *Controls)
{
    Win32SizeControls_(Controls);
    RECT ClientRect;
    Win32.GetClientRect(Win32State.WindowHwnd, &ClientRect);
    Win32.InvalidateRect(Win32State.WindowHwnd, &ClientRect, TRUE);
}

inline control*
CreateControl_(s64 ParentId, s64 ControlId, r32 Size, control_type Type)
{
    control *Result = 0;
    
    if(ParentId == ID_WINDOW)
    {
        Assert(Win32State.SentinalControl.Children == 0);
        
        if(Win32State.FirstFreeControl != 0)
        {
            control *FreeControl = Win32State.FirstFreeControl;
            Win32State.FirstFreeControl = FreeControl->NextControl;
            FreeControl->NextControl = Win32State.SentinalControl.Children;
            Win32State.SentinalControl.Children = FreeControl;
            
            Result = FreeControl;
        }
        else
        {
            Result= Win32State.SentinalControl.Children;
            while(Result != 0)
            {
                Result = Result->NextControl;
            }
            Result = PushStruct(&Win32State.Arena, control);
            Result->NextControl = Win32State.SentinalControl.Children;
            Win32State.SentinalControl.Children = Result;
        }
        
        Assert(Result);
        Result->ParentId = ID_WINDOW;
        Result->ParentHwnd = Win32State.WindowHwnd;
    }
    else
    {
        Assert(Win32State.SentinalControl.Children);
        
        control *ParentControl = GetControlById(Win32State.SentinalControl.Children, ParentId);
        Assert(ParentControl);
        
        if(Win32State.FirstFreeControl != 0)
        {
            control *FreeControl = Win32State.FirstFreeControl;
            Win32State.FirstFreeControl = FreeControl->NextControl;
            FreeControl->NextControl = ParentControl->Children;
            ParentControl->Children = FreeControl;
            Result = FreeControl;
        }
        else
        {
            Result = ParentControl->Children;
            while(Result != 0)
            {
                Result= Result->NextControl;
            }
            Result= PushStruct(&Win32State.Arena, control);
            Result->NextControl = ParentControl->Children;
            ParentControl->Children = Result;
        }
        
        Assert(Result);
        Result->ParentId = ParentControl->Id;
        Result->ParentHwnd = ParentControl->Hwnd;
    }
    
    Result->Id = ControlId;
    Result->Type = Type;
    Result->Size = Size;
    Result->Layout = ControlLayout_Horizontal;
    Result->PaddingTop = 0.0;
    Result->PaddingLeft = 0.0;
    Result->PaddingRight = 0.0;
    Result->PaddingBottom = 0.0;
    Result->MarginTop = 0.0;
    Result->MarginLeft = 0.0;
    Result->MarginRight = 0.0;
    Result->MarginBottom = 0.0;
    
    return(Result);
}

inline void
SetDefaultFont(HWND Hwnd)
{
    Win32.SendMessageA(Hwnd, WM_SETFONT, (LPARAM)Win32.GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

internal void
AddButton(s64 ParentId, s64 ControlId, char *Text, r32 Size)
{
    control *Control = CreateControl_(ParentId, ControlId, Size, ControlType_Button);
    
    Control->Hwnd = Win32.CreateWindowExA(0,
                                          "BUTTON",
                                          Text,
                                          WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
                                          10, 10,
                                          100, 100,
                                          Win32State.WindowHwnd, 
                                          (HMENU)Control->Id, 
                                          Win32State.Instance, 
                                          0);
    
    Assert(Control->Hwnd);
    SetDefaultFont(Control->Hwnd);
}

internal void
AddEdit(s64 ParentId, s64 ControlId, char *Text, r32 Size)
{
    control *Control = CreateControl_(ParentId, ControlId, Size, ControlType_Edit);
    
    Control->Hwnd = Win32.CreateWindowExA(0,
                                          "EDIT",
                                          Text,
                                          WS_VISIBLE|WS_CHILD,
                                          10, 10,
                                          100, 100,
                                          Win32State.WindowHwnd, 
                                          (HMENU)Control->Id, 
                                          Win32State.Instance, 
                                          0);
    
    Assert(Control->Hwnd);
    SetDefaultFont(Control->Hwnd);
}

internal void
AddPanel(s64 ParentId, s64 ControlId, control_layout Layout)
{
    control *Control = CreateControl_(ParentId, ControlId, SIZE_FILL, ControlType_Panel);
    Control->Layout = Layout;
    
    Control->Hwnd = Win32.CreateWindowExA(0,
                                          "STATIC",
                                          "",
                                          WS_VISIBLE|WS_CHILD,
                                          10, 10,
                                          100, 100,
                                          Win32State.WindowHwnd, 
                                          (HMENU)Control->Id, 
                                          Win32State.Instance, 
                                          0);
    
    Assert(Control->Hwnd);
}

internal void
AddGroupBox(s64 ParentId, s64 ControlId, char *Text, control_layout Layout)
{
    control *Control = CreateControl_(ParentId, ControlId, SIZE_FILL, ControlType_Panel);
    Control->Layout = Layout;
    
    Control->PaddingTop = 16.0f;
    Control->PaddingLeft = 8.0f;
    Control->PaddingRight = 8.0f;
    Control->PaddingBottom = 8.0f;
    
    Control->Hwnd = Win32.CreateWindowExA(0,
                                          "BUTTON",
                                          Text,
                                          WS_VISIBLE|WS_CHILD|BS_GROUPBOX,
                                          10, 10,
                                          100, 100,
                                          Win32State.WindowHwnd, 
                                          (HMENU)Control->Id, 
                                          Win32State.Instance, 
                                          0);
    Assert(Control->Hwnd);
    SetDefaultFont(Control->Hwnd);
}

internal void
AddSpacer(s64 ParentId, r32 Size)
{
    control *Control = CreateControl_(ParentId, IDC_STATIC, Size, ControlType_Spacer);
    
    Control->Hwnd = Win32.CreateWindowExA(0,
                                          "STATIC",
                                          "",
                                          WS_VISIBLE|WS_CHILD,
                                          10, 10,
                                          100, 100,
                                          Win32State.WindowHwnd, 
                                          (HMENU)Control->Id, 
                                          Win32State.Instance, 
                                          0);
    
    Assert(Control->Hwnd);
}

internal void
AddStatic(s64 ParentId, s64 ControlId, char *Text, r32 Size)
{
    control *Control = CreateControl_(ParentId, ControlId, Size, ControlType_Static);
    
    Control->Hwnd = Win32.CreateWindowExA(0,
                                          "STATIC",
                                          Text,
                                          WS_VISIBLE|WS_CHILD,
                                          10, 10,
                                          100, 100,
                                          Win32State.WindowHwnd, 
                                          (HMENU)Control->Id, 
                                          Win32State.Instance, 
                                          0);
    
    Assert(Control->Hwnd);
    SetDefaultFont(Control->Hwnd);
}



internal void
SetControlMargin(s64 ControlId, r32 Top, r32 Left, r32 Right, r32 Bottom)
{
    control *Control = GetControlById(Win32State.SentinalControl.Children, ControlId);
    Assert(Control);
    Control->MarginTop = Top;
    Control->MarginLeft = Left;
    Control->MarginRight = Right;
    Control->MarginBottom = Bottom;
}

internal void
DisplayMessage(char *Title, char *Message)
{
    Win32.MessageBox(Win32State.WindowHwnd, Title, Message, MB_OK);
}

internal void
GetControlText(s64 ControlId, char *Buffer, s32 BufferSize)
{
    control *Control = GetControlById(Win32State.SentinalControl.Children, ControlId);
    Assert(Control);
    Win32.GetWindowTextA(Control->Hwnd, Buffer, BufferSize);
}

internal void
SetControlText(s64 ControlId, char *Buffer)
{
    control *Control = GetControlById(Win32State.SentinalControl.Children, ControlId);
    Assert(Control);
    Win32.SetWindowTextA(Control->Hwnd, Buffer);
}

internal void
Win32InitPlatformAPI(platform_api *PlatformAPI)
{
    PlatformAPI->AddButton = AddButton;
    PlatformAPI->AddEdit = AddEdit;
    PlatformAPI->AddPanel = AddPanel;
    PlatformAPI->AddGroupBox = AddGroupBox;
    PlatformAPI->AddSpacer = AddSpacer;
    PlatformAPI->AddStatic = AddStatic;
    
    PlatformAPI->SetControlMargin = SetControlMargin;
    
    PlatformAPI->DisplayMessage = DisplayMessage;
    PlatformAPI->GetControlText = GetControlText;
    PlatformAPI->SetControlText = SetControlText;
    
}

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{       
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_SIZE:
        {
            Win32SizeControls(Win32State.SentinalControl.Children);
        } break;
        
        case WM_CLOSE:
        {
            Win32State.IsRunning = false;
        } break;
        
        case WM_COMMAND:
        {
            if(HIWORD(WParam) == EN_CHANGE)
            {
                // TODO(kstandbridge): Handle text changed?
            }
            else
            {
                if(GlobalApp.HandleCommand)
                {
                    GlobalApp.HandleCommand(WParam);
                }
            }
        } break;
        
        default:
        {
            Result = Win32.DefWindowProcA(Window, Message, WParam, LParam);
            
        } break;
    }
    
    return(Result);
}

internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE(kstandbridge): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO(kstandbridge): Dest bounds checking!
    
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }
    
    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }
    
    *Dest++ = 0;
}

internal int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}


inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}

internal win32_app_code
Win32LoadAppCode(char *SourceDLLName, char *TempDLLName, char *LockFileName)
{
    win32_app_code Result = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
        
        CopyFile(SourceDLLName, TempDLLName, FALSE);
        
        Result.AppCodeDLL = LoadLibraryA(TempDLLName);
        Assert(Result.AppCodeDLL);
        
        Result.CreateControls = (create_controls *)GetProcAddress(Result.AppCodeDLL, "CreateControls");
        Assert(Result.CreateControls);
        
        Result.HandleCommand = (handle_command *)GetProcAddress(Result.AppCodeDLL, "HandleCommand");
        Assert(Result.HandleCommand);
    }
    
    return(Result);
}

internal void
Win32UnloadAppCode(win32_app_code *AppCode)
{
    if(AppCode->AppCodeDLL)
    {
        FreeLibrary(AppCode->AppCodeDLL);
        AppCode->AppCodeDLL = 0;
    }
    AppCode->CreateControls = 0;
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline r32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    r32 Result = ((r32)(End.QuadPart - Start.QuadPart) /
                  (r32)GlobalPerfCountFrequency);
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    Win32InitAPI(&Win32);
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    // NOTE(kstandbridge): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    b32 SleepIsGranular = (Win32.TimeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    Win32State.Instance = Instance;
    Win32GetEXEFileName(&Win32State);
    
    char SourceAppCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "nocrt.dll",
                              sizeof(SourceAppCodeDLLFullPath), SourceAppCodeDLLFullPath);
    
    char TempAppCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "nocrt_temp.dll",
                              sizeof(TempAppCodeDLLFullPath), TempAppCodeDLLFullPath);
    
    char AppCodeLockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "lock.tmp",
                              sizeof(AppCodeLockFullPath), AppCodeLockFullPath);
    
    WNDCLASSA WindowClass = {};
    WindowClass.style = 0;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Win32State.Instance;
    WindowClass.hCursor = Win32.LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "NoCRTWindowClass";
    
    Assert(Win32.RegisterClassA(&WindowClass));
    
    Win32State.WindowHwnd = Win32.CreateWindowExA(WS_EX_CLIENTEDGE,
                                                  WindowClass.lpszClassName,
                                                  "No CRT",
                                                  WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                                  CW_USEDEFAULT,
                                                  CW_USEDEFAULT,
                                                  CW_USEDEFAULT,
                                                  CW_USEDEFAULT,
                                                  0,
                                                  0,
                                                  Win32State.Instance,
                                                  &Win32State);
    Assert(Win32State.WindowHwnd);
    
    
    platform_api PlatformAPI;
    Win32InitPlatformAPI(&PlatformAPI);
    
#if NOCRT_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif
    
    app_memory AppMemory = {};
    AppMemory.PermanentStorageSize = Megabytes(4);
    AppMemory.TransientStorageSize = Megabytes(4);
    Win32State.TotalSize = AppMemory.PermanentStorageSize + AppMemory.TransientStorageSize;
    Win32State.AppMemoryBlock = VirtualAlloc(BaseAddress, (memory_index)Win32State.TotalSize,
                                             MEM_RESERVE|MEM_COMMIT,
                                             PAGE_READWRITE);
    AppMemory.PermanentStorage = Win32State.AppMemoryBlock;
    AppMemory.TransientStorage = ((u8 *)AppMemory.PermanentStorage + AppMemory.PermanentStorageSize);
    
    InitializeArena(&Win32State.Arena, Megabytes(4), AppMemory.PermanentStorage);
    
    int MonitorRefreshHz = 60;
    // TODO(kstandbridge): Get monitor refresh rate?
    r32 AppUpdateHz = (MonitorRefreshHz / 2.0f);
    r32 TargetSecondsPerFrame = 1.0f / (r32)AppUpdateHz;
    
    LARGE_INTEGER LastCounter = Win32GetWallClock();
    Win32State.IsRunning = true;
    while(Win32State.IsRunning)
    {
        MSG Message;
        while(Win32.PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            switch(Message.message)
            {
                case WM_QUIT:
                {
                    
                } break;
                
                default:
                {
                    Win32.TranslateMessage(&Message);
                    Win32.DispatchMessageA(&Message);
                }
            }
        }
        
        FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceAppCodeDLLFullPath);
        if(CompareFileTime(&NewDLLWriteTime, &GlobalApp.DLLLastWriteTime) != 0)
        {
            Win32UnloadAppCode(&GlobalApp);
            
            Win32DestroyControls(Win32State.SentinalControl.Children);
            
            GlobalApp = Win32LoadAppCode(SourceAppCodeDLLFullPath, TempAppCodeDLLFullPath, AppCodeLockFullPath);
            
            if(GlobalApp.CreateControls)
            {
                GlobalApp.CreateControls(&PlatformAPI);
            }
            
            Win32SizeControls(Win32State.SentinalControl.Children);
            
        }
        
        LARGE_INTEGER WorkCounter = Win32GetWallClock();
        r32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
        
        // TODO(kstandbridge): NOT TESTED YET!  PROBABLY BUGGY!!!!!
        r32 SecondsElapsedForFrame = WorkSecondsElapsed;
        if(SecondsElapsedForFrame < TargetSecondsPerFrame)
        {                        
            if(SleepIsGranular)
            {
                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame -
                                                   SecondsElapsedForFrame));
                if(SleepMS > 0)
                {
                    Sleep(SleepMS);
                }
            }
            
            r32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                    Win32GetWallClock());
            if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
            {
                // TODO(kstandbridge): LOG MISSED SLEEP HERE
            }
            
            while(SecondsElapsedForFrame < TargetSecondsPerFrame)
            {                            
                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                Win32GetWallClock());
            }
        }
        else
        {
            // TODO(kstandbridge): MISSED FRAME RATE!
            // TODO(kstandbridge): Logging
        }
        
        LARGE_INTEGER EndCounter = Win32GetWallClock();
        //r32 MSPerFrame = 1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter);                    
        LastCounter = EndCounter;
        
    }
    
    return(0);
}

void __stdcall
WinMainCRTStartup()
{
    int Result = WinMain(GetModuleHandle(0), 0, 0, 0);
    ExitProcess(Result);
}