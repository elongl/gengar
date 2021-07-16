#include <windows.h>

void screenshot()
{
    int screen_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int screen_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    HDC desktop = GetDC(NULL);
    HBITMAP src_bitmap = CreateCompatibleBitmap(desktop, screen_width, screen_height);
    HDC dst_bitmap = CreateCompatibleDC(desktop);
    BITMAP screen = {};
    HANDLE hFile = NULL;
    BITMAPINFOHEADER bitmap_info = {
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = screen_width,
        .biHeight = screen_height,
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB,
    };

    SelectObject(dst_bitmap, src_bitmap);
    BitBlt(dst_bitmap, 0, 0, screen_width, screen_height, desktop, 0, 0, SRCCOPY);

    GetObject(src_bitmap, sizeof(screen), &screen);
    GetDIBits(desktop, src_bitmap, 0, screen_height, NULL, (BITMAPINFO *)&bitmap_info, DIB_RGB_COLORS);

    hFile = CreateFile("capture.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    DeleteObject(src_bitmap);
    DeleteObject(dst_bitmap);
    ReleaseDC(NULL, desktop);
}
