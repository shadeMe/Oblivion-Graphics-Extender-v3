#ifndef	U32_HPP
#define	U32_HPP

// Function pointer types.
typedef HWND (WINAPI *GetActiveWindow_t)(void);

// Function prototypes.
HWND WINAPI OBGEGetActiveWindow(void);

#endif
