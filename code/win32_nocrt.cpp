#include "nocrt_platform.h"

#include <windows.h>

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
        TotalSize += Control->Size;
        if(Control->Size == 0.0f)
        {
            ++ControlWithoutSize;
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
            X = 0;
            Y = (r32)Rect.bottom;
            Width = (r32)Rect.right - (r32)Rect.left;
            Height = ((r32)Rect.bottom - (r32)Rect.top - TotalSize)/ControlWithoutSize;
        }
        else
        {
            X = (r32)Rect.right;
            Y = 0;
            Width = ((r32)Rect.right - (r32)Rect.left - TotalSize)/ControlWithoutSize;
            Height = (r32)Rect.bottom - (r32)Rect.top;
        }
        
        for(control *Control = Controls;
            Control;
            Control = Control->NextControl)
        {
            --ControlCount;
            if(IsVerticleLayout)
            {
                s32 CalcHeight = (s32)(Control->Size ? Control->Size : Height);
                Y -= CalcHeight;
                Win32.MoveWindow(Control->Hwnd, (s32)X, (s32)Y, 
                                 (s32)Width, 
                                 CalcHeight, 
                                 FALSE);
                
            }
            else
            {
                s32 CalcWidth = (s32)(Control->Size ? Control->Size : Width);
                X -= CalcWidth;
                Win32.MoveWindow(Control->Hwnd, (s32)X, (s32)Y, 
                                 CalcWidth,
                                 (s32)Height, 
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

internal void
CreateControl(s64 ParentId, s64 Id, control_type Type, char *Text)
{
    control *Control = 0;
    
    if(ParentId == ID_WINDOW)
    {
        Assert(Win32State.SentinalControl.Children == 0);
        
        if(Win32State.FirstFreeControl != 0)
        {
            control *FreeControl = Win32State.FirstFreeControl;
            Win32State.FirstFreeControl = FreeControl->NextControl;
            FreeControl->NextControl = Win32State.SentinalControl.Children;
            Win32State.SentinalControl.Children = FreeControl;
            
            Control = FreeControl;
        }
        else
        {
            Control = Win32State.SentinalControl.Children;
            while(Control != 0)
            {
                Control = Control->NextControl;
            }
            Control = PushStruct(&Win32State.Arena, control);
            Control->NextControl = Win32State.SentinalControl.Children;
            Win32State.SentinalControl.Children = Control;
        }
        
        Assert(Control);
        Control->ParentId = ID_WINDOW;
        Control->ParentHwnd = Win32State.WindowHwnd;
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
            Control = FreeControl;
        }
        else
        {
            Control = ParentControl->Children;
            while(Control != 0)
            {
                Control = Control->NextControl;
            }
            Control = PushStruct(&Win32State.Arena, control);
            Control->NextControl = ParentControl->Children;
            ParentControl->Children = Control;
        }
        
        Assert(Control);
        Control->ParentId = ParentControl->Id;
        Control->ParentHwnd = ParentControl->Hwnd;
    }
    
    Control->Id = Id;
    Control->Type = Type;
    Control->Layout = ControlLayout_Horizontal;
    Control->Size = 0.0f;
    
    switch(Type)
    {
        
        case ControlType_Static:
        {
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
        } break;
        
        case ControlType_Edit:
        {
            Control->Hwnd = Win32.CreateWindowExA(0,
                                                  "EDIT",
                                                  Text,
                                                  WS_VISIBLE|WS_CHILD,
                                                  210, 10,
                                                  100, 100,
                                                  Win32State.WindowHwnd, 
                                                  (HMENU)Control->Id, 
                                                  Win32State.Instance, 
                                                  0);
        } break;
        
        case ControlType_Button:
        {
            Control->Hwnd = Win32.CreateWindowExA(0,
                                                  "BUTTON",
                                                  Text,
                                                  WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
                                                  410, 10,
                                                  100, 100,
                                                  Win32State.WindowHwnd, 
                                                  (HMENU)Control->Id, 
                                                  Win32State.Instance, 
                                                  0);
        } break;
        
        default:
        {
            InvalidCodePath;
        } break;
    }
    
    Win32.SendMessageA(Control->Hwnd, WM_SETFONT, (LPARAM)Win32.GetStockObject(DEFAULT_GUI_FONT), TRUE);
    
    Assert(Control->Hwnd);
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
SetControlLayout(s64 ControlId, control_layout ControlLayout)
{
    Assert(Win32State.SentinalControl.Children);
    control *Control = GetControlById(Win32State.SentinalControl.Children, ControlId);
    Assert(Control);
    Control->Layout = ControlLayout;
}

internal void
SetControlSize(s64 ControlId, r32 Size)
{
    Assert(Win32State.SentinalControl.Children);
    control *Control = GetControlById(Win32State.SentinalControl.Children, ControlId);
    Assert(Control);
    Control->Size = Size;
}

internal void
Win32InitPlatformAPI(platform_api *PlatformAPI)
{
    PlatformAPI->CreateControl = CreateControl;
    PlatformAPI->DisplayMessage = DisplayMessage;
    PlatformAPI->GetControlText = GetControlText;
    PlatformAPI->SetControlText = SetControlText;
    PlatformAPI->SetControlLayout = SetControlLayout;
    PlatformAPI->SetControlSize = SetControlSize;
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