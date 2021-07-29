#ifndef WIN32_NOCRT_H
#define WIN32_NOCRT_H

typedef HWND create_window_ex_a(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
typedef LRESULT def_window_proc_a(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL destroy_window(HWND hWnd);
typedef LRESULT dispatch_message_a(const MSG *lpMsg);
typedef BOOL get_client_rect(HWND hWnd, RECT *lpRect);
typedef HWND get_dlg_item(HWND hDlg, int nIDDlgItem);
typedef HWND get_partent(HWND hWnd);
typedef HGDIOBJ get_stock_object(int i);
typedef int get_window_text_a(HWND hWnd, LPSTR lpString, int nMaxCount);
typedef BOOL invalidate_rect(HWND hWnd, const RECT *lpRect, BOOL bErase);
typedef HCURSOR load_cursor_a(HINSTANCE hInstance, LPCSTR lpCursorName);
typedef BOOL peek_message_a(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef ATOM register_class_a(const WNDCLASSA *lpWndClass);
typedef int map_window_points(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);
typedef int message_box_a(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
typedef BOOL move_window(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
typedef LRESULT send_message_a(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL set_window_text_a(HWND hWnd, LPCSTR lpString);
typedef MMRESULT time_begin_period(UINT uPeriod);
typedef BOOL translate_message(const MSG *lpMsg);

struct win_32
{
    create_window_ex_a *CreateWindowExA;
    def_window_proc_a *DefWindowProcA;
    destroy_window *DestroyWindow;
    dispatch_message_a *DispatchMessageA;
    get_client_rect *GetClientRect;
    get_dlg_item *GetDlgItem;
    get_partent *GetParent;
    get_stock_object *GetStockObject;
    get_window_text_a *GetWindowTextA;
    invalidate_rect *InvalidateRect;
    load_cursor_a *LoadCursorA;
    peek_message_a *PeekMessageA;
    register_class_a *RegisterClassA;
    map_window_points *MapWindowPoints;
    message_box_a *MessageBoxA;
    move_window *MoveWindow;
    send_message_a *SendMessageA;
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

#define MAX_CHILDREN 8
struct control
{
    s64 Id;
    HWND Hwnd;
    
    s64 ParentId;
    HWND ParentHwnd;
    
    control_type Type;
    
    control *Children;
    
    control *NextControl;
};

#define MAX_CONTROLS 32
#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_state
{
    s64 TotalSize;
    void *AppMemoryBlock;
    memory_arena Arena;
    
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
    
    b32 IsRunning;
    HINSTANCE Instance;
    HWND WindowHwnd;
    
    control SentinalControl;
    control *FirstFreeControl;
};


#endif //WIN32_NOCRT_H
