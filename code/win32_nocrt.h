#ifndef WIN32_NOCRT_H
#define WIN32_NOCRT_H

typedef HWND create_window_ex_a(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
typedef LRESULT def_window_proc_a(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL destroy_window(HWND hWnd);
typedef LRESULT dispatch_message_a(const MSG *lpMsg);
typedef HWND get_dlg_item(HWND hDlg, int nIDDlgItem);
typedef HCURSOR load_cursor_a(HINSTANCE hInstance, LPCSTR lpCursorName);
typedef BOOL peek_message_a(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef ATOM register_class_a(const WNDCLASSA *lpWndClass);
typedef int message_box_a(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
typedef BOOL set_window_text_a(HWND hWnd, LPCSTR lpString);
typedef MMRESULT time_begin_period(UINT uPeriod);
typedef BOOL translate_message(const MSG *lpMsg);

struct win_32
{
    create_window_ex_a *CreateWindowExA;
    def_window_proc_a *DefWindowProcA;
    destroy_window *DestroyWindow;
    dispatch_message_a *DispatchMessageA;
    get_dlg_item *GetDlgItem;
    load_cursor_a *LoadCursorA;
    peek_message_a *PeekMessageA;
    register_class_a *RegisterClassA;
    message_box_a *MessageBoxA;
    set_window_text_a *SetWindowTextA;
    time_begin_period *TimeBeginPeriod;
    translate_message *TranslateMessage;
};


struct win32_app_code
{
    HMODULE AppCodeDLL;
    FILETIME DLLLastWriteTime;
    
    create_controls *CreateControls;
    handle_command *HandleCommand;
};

struct control
{
    s64 Id;
    control_type Type;
    HWND Hwnd;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_state
{
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
    
    b32 IsRunning;
    HINSTANCE Instance;
    HWND Window;
    
    control Controls[32];
    s32 CurrentControl;
    
};


#endif //WIN32_NOCRT_H
