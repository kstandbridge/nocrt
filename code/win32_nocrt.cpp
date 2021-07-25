
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
    
    Result->GetDlgItem = (get_dlg_item *)GetProcAddress(User32Library, "GetDlgItem");
    Assert(Result->GetDlgItem);
    
    Result->LoadCursorA = (load_cursor_a *)GetProcAddress(User32Library, "LoadCursorA");
    Assert(Result->LoadCursorA);
    
    Result->PeekMessageA = (peek_message_a *)GetProcAddress(User32Library, "PeekMessageA");
    Assert(Result->PeekMessageA);
    
    Result->RegisterClassA = (register_class_a *)GetProcAddress(User32Library, "RegisterClassA");
    Assert(Result->RegisterClassA);
    
    Result->MessageBoxA = (message_box_a *)GetProcAddress(User32Library, "MessageBoxA");
    Assert(Result->MessageBoxA);
    
    Result->SetWindowTextA = (set_window_text_a *)GetProcAddress(User32Library, "SetWindowTextA");
    Assert(Result->SetWindowTextA);
    
    Result->TimeBeginPeriod = (time_begin_period *)GetProcAddress(WinmmLibrary, "timeBeginPeriod");
    Assert(Result->TimeBeginPeriod);
    
    Result->TranslateMessage = (translate_message *)GetProcAddress(User32Library, "TranslateMessage");
    Assert(Result->TranslateMessage);
}

internal void
CreateControl(control_type ControlType, char *Text, s64 Id)
{
    char *Type = 0;
    if(ControlType == ControlType_Button)
    {
        Type = "BUTTON";
    }
    
    control *Control = Win32State.Controls + Win32State.CurrentControl++;
    Assert(Win32State.CurrentControl < ArrayCount(Win32State.Controls));
    
    Control->Id = Id;
    Control->Type = ControlType;
    Control->Hwnd = Win32.CreateWindowExA(WS_EX_CLIENTEDGE,
                                          Type,
                                          Text,
                                          WS_VISIBLE|WS_CHILD,
                                          10, 10,
                                          100, 100,
                                          Win32State.Window, 
                                          (HMENU)Control->Id, 
                                          Win32State.Instance, 
                                          0);
    Assert(Control->Hwnd);
}

internal void
DisplayMessage(char *Title, char *Message)
{
    Win32.MessageBox(Win32State.Window, Title, Message, MB_OK);
}

internal void
Win32InitPlatformAPI(platform_api *PlatformAPI)
{
    PlatformAPI->CreateControl = CreateControl;
    PlatformAPI->DisplayMessage = DisplayMessage;
}

extern "C"
{
    int _fltused;
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
        case WM_CLOSE:
        {
            Win32State.IsRunning = false;
        } break;
        
        case WM_COMMAND:
        {
            if(GlobalApp.HandleCommand)
            {
                GlobalApp.HandleCommand(WParam);
            }
            /*if(WParam == ID_BUTTON)
            {
                HWND HwndButton = Win32.GetDlgItem(Window, ID_BUTTON);
                Assert(HwndButton);
                Win32.SetWindowTextA(HwndButton, "World");
            }
            else
            {
                Win32.SetWindowTextA(Window, "Failed");
            }
            */
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
    
    Win32State.Window = Win32.CreateWindowExA(WS_EX_CLIENTEDGE,
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
    Assert(Win32State.Window);
    
    
    platform_api PlatformAPI;
    Win32InitPlatformAPI(&PlatformAPI);
    
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
            
            for(s32 ControlIndex = 0;
                ControlIndex < ArrayCount(Win32State.Controls);
                ++ControlIndex)
            {
                control *Control = Win32State.Controls + ControlIndex;
                if(Control->Id)
                {
                    Control->Id = 0;
                    Assert(Win32.DestroyWindow(Control->Hwnd));
                }
            }
            Win32State.CurrentControl = 0;
            
            GlobalApp = Win32LoadAppCode(SourceAppCodeDLLFullPath, TempAppCodeDLLFullPath, AppCodeLockFullPath);
            
            if(GlobalApp.CreateControls)
            {
                GlobalApp.CreateControls(&PlatformAPI);
            }
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