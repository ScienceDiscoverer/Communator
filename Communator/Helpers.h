#ifndef HELPERS_H
#define HELPERS_H

#define _WINSOCKAPI_

#include <string>
#include <Windows.h>
#include "Resource.h"

typedef unsigned short Word;
typedef unsigned char Byte;

using namespace std;

struct Win
{
	Win() = default;
	Win(int xpos, int ypos, int width, int height) : x(xpos), y(ypos), w(width), h(height) {}

	int x, y;
	int w, h;
};

HWND makeStaticEmpty(const Win& w, HWND parent, int menu);
HWND makeStaticEmptyTop(const Win& w, HWND parent, int menu);
HWND makeStaticEmptyTop(const Win& w, HWND parent, int menu, int styles);
HFONT makeStaticText(const string& text, const Win& w, HWND parent, int menu, int font_size);
HWND makeStaticText(const string& text, const Win& w, HWND parent, int menu, HFONT font);
HFONT makeStaticText(const string& text, const Win& w, HWND parent, int menu, int font_size, int styles);
HWND makeStaticText(const string& text, const Win& w, HWND parent, int menu, HFONT font, int styles);
HFONT makeStaticTextW(LPCWSTR text, const Win& w, HWND parent, int menu, int font_size);
HWND makeStaticTextW(LPCWSTR text, const Win& w, HWND parent, int menu, HFONT font);

HBITMAP makeStaticImage(int id, const Win& w, HWND parent, int menu);
HBITMAP makeStaticImage(int id, const Win& w, HWND parent, int menu, int styles);
HWND makeStaticImage(HBITMAP bitmap, const Win& w, HWND parent, int menu);

HFONT makeEdit(const string& text, const Win& w, HWND parent, int menu, int font_size);
HWND makeEdit(const string& text, const Win& w, HWND parent, int menu, HFONT font);

BOOL trackMouse(HWND target);

NMHDR makeNMHDR(int code, HWND hwnd);

string dToStr(double d);
string dToStrFull(double d);

Word getDate();
string dateToStr(int t);

#endif /* HELPERS_H */
