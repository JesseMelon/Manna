#include "platform/platform.h"

#if M_PLATFORM_WINDOWS

#include "core/logger.h"
#include "core/input.h"
#include "containers/darray.h"
#include "renderer/vulkan/vulkan_platform.h"
#include "renderer/vulkan/vulkan_types.h"
#include "core/event.h"

#include <windows.h>
#include <windowsx.h> //for input 
#include <stdlib.h>

//surface creation
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_win32.h"
#include "renderer/vulkan/vulkan_types.h"

typedef struct platform_state {
	HINSTANCE instance;
	HWND window;
    VkSurfaceKHR surface; //probably wont stay here
    f64 clock_frequency;
    LARGE_INTEGER start_time;
} platform_state;

static platform_state* state_ptr;

LRESULT CALLBACK win32_window_process_message(HWND window, UINT message, WPARAM w_param, LPARAM l_param); //forward declare window event handler

b8 platform_startup(
    u64* memory_requirement,
	void* state,
	const char* title,
	i32 x,
	i32 y,
	i32 width,
	i32 height) {

    *memory_requirement = sizeof(platform_state);
    if (state == 0) {
        return TRUE;
    }

    state_ptr = state;

	state_ptr->instance = GetModuleHandle(NULL);					//returns handle to currently executing program

	//configure window class
	HICON icon = LoadIcon(state_ptr->instance, IDI_APPLICATION);	//TODO: use (HICON)LoadImageW(instance, MAKEINTRESOURCE(MY_ICON), IMAGE_ICON, 32, 32, LR_LOADFROMFILE); for finer control. 
	WNDCLASSA wc = {0};											//window class ANSI meaning 8 bit characters, as opposed to UNICODE - 16 bit characters
	wc.style = CS_DBLCLKS;										//flags for window performance and style. Accepts double clicks in window
	wc.lpfnWndProc = win32_window_process_message;				//long pointer to window procedure function, function for handling window events
	wc.cbClsExtra = 0;											//legacy class specific mem allocation shared by all windows from this class
	wc.cbWndExtra = 0;											//legacy window specific mem allocation
	wc.hInstance = state_ptr->instance;								//handle to the program containing window procedure
	wc.hIcon = icon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);					//load stock arrow cursor
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "manna_window_class";	

	//register window class
	if (!RegisterClassA(&wc)) {
		MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK); //error popup
		return FALSE;
	}

	//Create Window
	u32 window_style = WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
	u32 window_style_ex = WS_EX_APPWINDOW;

	//calculate border size
	RECT border_rect = {0};
	AdjustWindowRectEx(&border_rect, window_style, FALSE, window_style_ex);

	x += border_rect.left;
	y += border_rect.top;
	width += border_rect.right - border_rect.left;
	height += border_rect.bottom - border_rect.top;

	HWND window_handle = CreateWindowExA(
		window_style_ex,
		"manna_window_class",
		title,
		window_style,
		x,
		y,
		width,
		height,
		NULL,
		NULL,
		state_ptr->instance,
		NULL);

	if (!window_handle) {
		MessageBoxA(0, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK); //error popup
		LOG_FATAL("Window creation failed");
		return FALSE;
	} else {
		state_ptr->window = window_handle;
	}

	//set command flags
	b32 should_activivate = 1; //accept input
	i32 show_window_command_flags = should_activivate ? SW_SHOW : SW_SHOWNOACTIVATE;

	ShowWindow(state_ptr->window, show_window_command_flags);
	
	//setup clock
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	state_ptr->clock_frequency = 1.0 / (f64)frequency.QuadPart;
	QueryPerformanceCounter(&state_ptr->start_time);

	return TRUE;
}

void shutdown_platform(void* platform_state) {

	if (state_ptr && state_ptr->window) {
		DestroyWindow(state_ptr->window);
		state_ptr->window = 0;
	}
}

b8 platform_get_messages () {
    if (state_ptr) {
	    MSG msg;
    	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	    	TranslateMessage(&msg);
	    	DispatchMessage(&msg);
        }
	}

    
	return TRUE;
}

void* platform_allocate(u64 size, b8 aligned)
{
	return malloc(size);
}

void platform_free(void *ptr, b8 aligned)
{
	free(ptr);
}

void* platform_copy_memory(void* dest, const void* src, u64 size)
{
	return memcpy(dest, src, size);
}

void* platform_zero_memory(void* dest, u64 size)
{
	return memset(dest, 0, size);
}

void* platform_set_memory(void* dest, u8 value, u64 size)
{
	return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 color_index)
{
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };					//color for console text
	SetConsoleTextAttribute(console_handle, levels[color_index]);

	OutputDebugStringA(message);
	u64 length = strlen(message);
	LPDWORD bytes_written = 0;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, bytes_written, NULL);
}

void platform_console_write_error(const char* message, u8 color_index)
{

	HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);			//write to error stream
	static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };					//color for console text
	SetConsoleTextAttribute(console_handle, levels[color_index]);

	OutputDebugStringA(message);
	u64 length = strlen(message);
	LPDWORD bytes_written = 0;
	WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, bytes_written, NULL);
}

f64 platform_get_time()
{
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	return (f64)current_time.QuadPart * state_ptr->clock_frequency;
}

void platform_sleep(u64 ms)
{
	Sleep(ms);
}

void vulkan_platform_get_required_extension_names(const char ***names_darray) {
    darray_push(*names_darray, &"VK_KHR_win32_surface");
}

b8 platform_create_vulkan_surface(vulkan_context* context) {

    if (!state_ptr) {
        return FALSE;
    }

    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = state_ptr->instance;
    create_info.hwnd = state_ptr->window;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state_ptr->surface);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Vulkan surface creation failed");
        return FALSE;
    }
    context->surface = state_ptr->surface;
    return TRUE;
}

LRESULT CALLBACK win32_window_process_message(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	switch (message) {
		case WM_ERASEBKGND:
			return 1;	//handle erasing in application
		case WM_CLOSE:
		event_data data = {};
            trigger_event(EVENT_APPLICATION_QUIT, 0, data);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE: {
			RECT r;
			GetClientRect(window, &r);
			u32 width = r.right - r.left;
			u32 height = r.bottom - r.top;
            
            event_data data;
            data.u16[0] = (u16)width;
            data.u16[1] = (u16)height;
            trigger_event(EVENT_WINDOW_RESIZED, 0, data);

        } break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			//on key messages, w_param is a 16 bit data package with keycode, which is sent in to process key.
			b8 pressed = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
			keys key = (u16)w_param;

            //windows does not differentiate between left and right keys without this 
            // 0x8000 is the bit indicating the key is pressed
            if (w_param == VK_SHIFT) {
                if(GetKeyState(VK_RSHIFT) & 0x8000) {
                    key = KEY_RSHIFT;
                } else if(GetKeyState(VK_LSHIFT) & 0x8000) {
                    key = KEY_LSHIFT;
                }
            } else if(w_param == VK_CONTROL) {
                if(GetKeyState(VK_RCONTROL) & 0x8000) {
                    key = KEY_RCONTROL;
                } else if(GetKeyState(VK_LCONTROL) & 0x8000){
                    key = KEY_LCONTROL;
                }
            } else if(w_param == VK_MENU) {
                if(GetKeyState(VK_RMENU) & 0x8000) {
                    key = KEY_RALT;
                } else if(GetKeyState(VK_LMENU) & 0x8000){
                    key = KEY_LALT;
                }
            }

			process_key(key, pressed);
			
		} break;
		case WM_MOUSEMOVE: {
			i32 x = GET_X_LPARAM(l_param);
			i32 y = GET_Y_LPARAM(l_param);
			process_mouse_move(x, y);
		} break;
		case WM_MOUSEWHEEL: {
			i32 delta = GET_WHEEL_DELTA_WPARAM(w_param);
			if (delta != 0) {
				//handle scroll sensitivity within the engine by normalizing this delta
				delta = (delta < 0) ? -1 : 1;
				process_mouse_wheel(delta);
			}
		} break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP: {
			b8 pressed = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN;
			mouse_button button = MB_COUNT;
			switch (message) {
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				button = MB_LEFT;
				break;
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				button = MB_MIDDLE;
				break;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				button = MB_RIGHT;
				break;
			}

			if (button != MB_COUNT) {
				process_mouse_button(button, pressed);
			}
			else {
				LOG_WARN("Unset mouse button after handling mouse button window message");
			}

		} break;
	}

	return DefWindowProcA(window, message, w_param, l_param);
}

#endif // M_PLATFORM_WINDOWS
