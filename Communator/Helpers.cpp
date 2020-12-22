#include "Helpers.h"
#include <ctime>

HWND makeStaticEmpty(const Win& w, HWND parent, int menu)
{
	HWND tmp = CreateWindowA("STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE,
							 w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);

	return tmp;
}

HWND makeStaticEmptyTop(const Win& w, HWND parent, int menu)
{
	HWND tmp = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "STATIC", NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTER | SS_CENTERIMAGE,
							   w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);

	return tmp;
}

HWND makeStaticEmptyTop(const Win& w, HWND parent, int menu, int styles)
{
	HWND tmp = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "STATIC", NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTER | SS_CENTERIMAGE | styles,
		w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);

	return tmp;
}


HFONT makeStaticText(const string& text, const Win& w, HWND parent, int menu, int font_size)
{
	HFONT font;
	int cell_size = -((font_size * GetDeviceCaps(GetDC(parent), LOGPIXELSY)) / 72);
	
	font = CreateFontA(cell_size, 0, 0, 0, FW_REGULAR/*FW_DONTCARE*/, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Impact");

	CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "STATIC", text.c_str(), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTER | SS_CENTERIMAGE,
					w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);
	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return font;
}

HWND makeStaticText(const string& text, const Win& w, HWND parent, int menu, HFONT font)
{
	HWND tmp = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "STATIC", text.c_str(), WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTER | SS_CENTERIMAGE,
							   w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);
	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return tmp;
}

HFONT makeStaticText(const string& text, const Win& w, HWND parent, int menu, int font_size, int styles)
{
	HFONT font;
	int cell_size = -((font_size * GetDeviceCaps(GetDC(parent), LOGPIXELSY)) / 72);

	font = CreateFontA(cell_size, 0, 0, 0, FW_REGULAR/*FW_DONTCARE*/, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Impact");

	CreateWindowA("STATIC", text.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | styles,
				  w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);
	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return font;
}

HWND makeStaticText(const string& text, const Win& w, HWND parent, int menu, HFONT font, int styles)
{
	HWND tmp = CreateWindowA("STATIC", text.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE | styles,
						     w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);
	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return tmp;
}

HFONT makeStaticTextW(LPCWSTR text, const Win& w, HWND parent, int menu, int font_size)
{
	HFONT font;
	int cell_size = -((font_size * GetDeviceCaps(GetDC(parent), LOGPIXELSY)) / 72);

	font = CreateFontW(cell_size, 0, 0, 0, FW_REGULAR/*FW_DONTCARE*/, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Impact");

	CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT, L"STATIC", text, WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE,
					w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);
	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return font;
}

HWND makeStaticTextW(LPCWSTR text, const Win& w, HWND parent, int menu, HFONT font)
{
	HWND tmp = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT, L"STATIC", text, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTER | SS_CENTERIMAGE,
							   w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);
	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return tmp;
}

HBITMAP makeStaticImage(int id, const Win& w, HWND parent, int menu)
{
	HBITMAP bm = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(id));
	
	CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "STATIC", NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTERIMAGE | SS_BITMAP,
					w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);

	SendMessage(GetDlgItem(parent, menu), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bm);

	return bm;
}

HBITMAP makeStaticImage(int id, const Win& w, HWND parent, int menu, int styles)
{
	HBITMAP bm = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(id));

	CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "STATIC", NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTERIMAGE | SS_BITMAP | styles,
		w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);

	SendMessage(GetDlgItem(parent, menu), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bm);

	return bm;
}

HWND makeStaticImage(HBITMAP bitmap, const Win& w, HWND parent, int menu)
{
	HWND tmp = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "STATIC", NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_CENTERIMAGE | SS_BITMAP,
							   w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);

	SendMessage(tmp, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmap);

	return tmp;
}

HFONT makeEdit(const string& text, const Win& w, HWND parent, int menu, int font_size)
{
	HFONT font;
	int cell_size = -((font_size * GetDeviceCaps(GetDC(parent), LOGPIXELSY)) / 72);

	font = CreateFontA(cell_size, 0, 0, 0, FW_REGULAR/*FW_DONTCARE*/, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Impact");

	CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "EDIT", text.c_str(), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |  ES_CENTER | ES_AUTOHSCROLL,
				   w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);

	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return font;
}

HWND makeEdit(const string& text, const Win& w, HWND parent, int menu, HFONT font)
{
	HWND tmp = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT, "EDIT", text.c_str(), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |  ES_CENTER | ES_AUTOHSCROLL,
							  w.x, w.y, w.w, w.h, parent, (HMENU)(UINT_PTR)menu, NULL, NULL);
	SendMessage(GetDlgItem(parent, menu), WM_SETFONT, WPARAM(font), TRUE);

	return tmp;
}

BOOL trackMouse(HWND target)
{
	tagTRACKMOUSEEVENT tmp;
	tmp.cbSize = sizeof(tagTRACKMOUSEEVENT);
	tmp.dwFlags = TME_HOVER | TME_LEAVE;
	tmp.hwndTrack = target;
	tmp.dwHoverTime = 1;

	return TrackMouseEvent((LPTRACKMOUSEEVENT)&tmp);
}

NMHDR makeNMHDR(int code, HWND hwnd)
{
	NMHDR nmh;
	nmh.code = code;
	nmh.idFrom = GetDlgCtrlID(hwnd);
	nmh.hwndFrom = hwnd;

	return nmh;
}

string dToStr(double d)
{
	string tmp(to_string(d));
	int dot = (int)tmp.find('.');

	string out = tmp.substr(0, (size_t)dot+1);
	
	if(tmp[dot+1] != '0')
	{
		out += tmp[dot+1];
	}
	else if(tmp[dot+2] != '0')
	{
		out += '0';
	}
	else
	{
		out = out.substr(0, (size_t)dot);
		return out;
	}
	
	out += tmp[dot+2];

	return out;
}

string dToStrFull(double d)
{
	string tmp(to_string(d));

	int n;
	for(int i = (int)tmp.size()-1; i >= 0; --i)
	{
		if(tmp[i] == '.')
		{
			n = i;
			break;
		}
		else if(tmp[i] != '0')
		{
			n = i+1;
			break;
		}
	}
	return tmp.substr(0, (size_t)n);
}

Word getDate()
{
	time_t t = time(nullptr);
	tm* tinfp = new tm;
	localtime_s(tinfp, &t);
	const tm& tinf = *tinfp;

	Byte y = tinf.tm_year + 1900 - 2000; // Years after 2000
	Byte m = tinf.tm_mon + 1;

	delete tinfp;
	return (Word)m << 8 | (Word)y;
}

string dateToStr(int t)
{
	time_t tt = t;
	tm* tinfp = new tm;
	localtime_s(tinfp, &tt);
	const tm& tinf = *tinfp;

	return to_string(tinf.tm_mday) + "." + to_string(tinf.tm_mon+1) + "." + to_string(tinf.tm_year+1900-2000);
}