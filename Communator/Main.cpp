#include <string>
#include <map>
#include "Defines.h"
#include "Helpers.h"
#include "DB.h"
#include "Server.h"

#include <objidl.h>
#include <gdiplus.h>

/*
IMPLEMENT:
1. <DONE> Always on top
2. <DONE> Make real date collection in db and in month creation
3. <DONE> Add previous monthes and tariffs from existing exel stats
4. <DONE> Add csv export, mb even public exel format too
5. <DONE> Implement year-graph of 3 main communes

NON PRIORITY:
1. Add form to for users to input previous date|readings|tariffs from their archive
2. Add animated indicator of waiting for client readings info
3. Add coefficients in Month.h to user interface to let user change it at runtime
4. Subclass edit boxes so then only recive can recive numbers + 1 floating dot
5. Add Pie Chart window on left button

BUGZ:
1. <FIXED> <r14b> MAJOR BUG!! AFTER RECALCULATING MONTH IT IS NOT SAVED IN DB CORRECTLY!!!
2. <FIXED> <r1b> Crash on saving attempt if sums are negative floats. Default switch 0 -> destruct. of empty linked list
3. <FIXED> Graph is not redrawing after new month arrive, and on recalculation

BUILDS:
0.2.1.14 - 14 builds for 1 little bug >_<
0.2.2.6 - implemented 2, 3, 4
0.2.3.101 - implemented 5, added icon
0.2.4.12 - in-depth testing for 4 years period, fixed 3, fixed other bugs, ready for beta release; 2926 code lines
*/

using namespace std;
using namespace Gdiplus;

// Pragma comments
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#pragma comment(lib, "Gdiplus.lib")

// Typedefs
typedef pair<int, HBITMAP> Pib;
typedef pair<int, HFONT> Pif;
typedef pair<int, HBRUSH> Pibrush;
typedef map<int, HBITMAP> MapIb;
typedef map<int, HFONT> MapIf;
typedef map<int, HBRUSH> MapIbrush;

// Constants
const int buflen = 16;
const int wndw = 800;
const int wndh = 800;
const int side_wndw = 300;
const int side_wndh = 300;
const int init_x = 200;
const int init_y = 200;
const int xbutt = 20; // Close button size
const string wnd_title = "Communator";
const string wnd_class = "Comm Window Class";
const string side_wnd_class = "Side Window Class";

// Windows variables
HWND hwindow, hsidewnd;
HINSTANCE hinstance;
WNDPROC oldWndProc;
WNDPROC oldGraphWndProc;
WSADATA wsa;

// Custom variables
DataBase db;
Server serv;
int mx, my;
int mxp, myp;
bool mwprs; // Mouse was pressed
bool mouseOverButton;
bool mwprsOverButton;
bool tarif_on;
int lbutcol = BUTTONS, rbutcol = BUTTONS, mbutcol = BUTTONS, xbutcol = BUTTONS;
int lperccol = BOTINF_PTU, mperccol = BOTINF_PTD, rperccol = BOTINF_PTU;
int ecb = MUTIL_B, gcb = MUTIL_B, wcb = MUTIL_B, pcb = SUTIL_B, icb = SUTIL_B, rcb = RESULT_B,
ect = MUTIL_T, gct = MUTIL_T, wct = MUTIL_T, pct = SUTIL_T, ict = SUTIL_T, rct = RESULT_T;
POINT ele_line[13], gas_line[13], wat_line[13];
int lines_num;
MapIb bitmaps;
MapIf fonts;
MapIbrush brushes;

// Functions declarations
LRESULT CALLBACK winProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK buttonWinProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK graphWinProc(HWND, UINT, WPARAM, LPARAM);
bool summonWindow();
bool summonSideWindow();
void moveWindow(WPARAM wparam);
void summonChildWindows();
void mapGraph();
void drawGraph(HDC hdc);
void onNotify(WPARAM wp, LPARAM lp);
bool calcNewMonthUI();
bool recalcMonth();
bool calcNewMonthNet(Readings rd);
ReadTariff getReadTariff();
void displaySums(const Sums& s, const Deltas& d, Percents p);
void setTariffs(const Readings& r);
LRESULT onCtlColorStatic(HDC hdc, HWND hcont);
void copyToClipboard(int menu);
void copyResultToClipboard();
void loadAdditButImg();
int editTxti(int menu);
double editTxtd(int menu);
void cleanUp();

int main()
{
	MSG msg;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if(!db.Load())
	{
		MessageBox(hwindow, "Database loading failed! Creating empty Database...", "Error", MB_OK | MB_ICONERROR);
	}

	// Initialize Winsock
	int res;
	res = WSAStartup(MAKEWORD(2, 2), &wsa);
	if(res != 0)
	{
		MessageBox(NULL, ("WSAStartup() failed! Error: " + to_string(res)).c_str(), "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	summonWindow();
	summonSideWindow();
	summonChildWindows();

	serv.Init(hwindow);

	const Month& lm = db.GetLastMonth();
	displaySums(lm.GetSums(), lm.GetDeltas(), lm.GetPercents());
	setTariffs(lm.GetReadings());
	mapGraph();

	int result;
	while((result = GetMessage(&msg, NULL, NULL, NULL)) != 0)
	{
		if(result == -1)
		{
			int err = GetLastError();
			MessageBox(NULL, ("GetMessage() failed! Error: " + to_string(err)).c_str(), "Error", MB_OK | MB_ICONERROR);
			break;
		}
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if(!db.Save())
	{
		MessageBox(hwindow, "Database saving failed!", "Error", MB_OK | MB_ICONERROR);
	}

	WSACleanup();
	GdiplusShutdown(gdiplusToken);
	cleanUp();

	return (int)msg.wParam;
}

// Functions defenitions
LRESULT CALLBACK winProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	{
	case WM_CREATE:
		break;

	case WM_ACTIVATE:
		break;

	case NET_EVENT:
		switch(lparam)
		{
		case FD_ACCEPT:
			serv.Accept();
			break;

		case FD_CONNECT:
			break;

		case FD_READ:
			calcNewMonthNet(serv.Read());
			break;

		case FD_CLOSE:
			serv.Shutdown();
			break;
		}
		break;

	case WM_SIZE:
		if(wparam == SIZE_RESTORED)
		{
			if(tarif_on)
			{
				ShowWindow(hsidewnd, SW_SHOW);
			}

			SetForegroundWindow(hsidewnd);
			SetForegroundWindow(hwindow);
		}
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		return onCtlColorStatic((HDC)wparam, (HWND)lparam);
		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_LBUTTONUP:
		if(mwprsOverButton)
		{
			mwprsOverButton = false;
		}
		break;

	case WM_MOUSEMOVE:
	{
		// Note: rparam can give info about buttons pressed
		mx = ((int)lparam & 0xffff);
		my = ((int)lparam >> 16);

		SendMessage(GetDlgItem(hwindow, 1), WM_SETTEXT, NULL, (LPARAM)(to_string(mx) + " " + to_string(my)).c_str());

		moveWindow(wparam);

		break;
	}

	case WM_CHAR:
		if(toupper((int)wparam) == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		break;

	//case WM_COMMAND:
	case WM_NOTIFY:
		onNotify(wparam, lparam);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}

	return 0;
}

LRESULT CALLBACK buttonWinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	NMHDR nmh;
	switch(msg)
	{
	case WM_MOUSEMOVE:
		if(!mouseOverButton)
		{
			trackMouse(hwnd);
			mouseOverButton = true;
		}
		break;

	case WM_LBUTTONDOWN:
		nmh = makeNMHDR(N_MDOWN, hwnd);
		SendMessage(GetParent(hwnd), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
		mwprsOverButton = true;
		break;
	
	case WM_LBUTTONUP:
		nmh = makeNMHDR(N_MUP, hwnd);
		SendMessage(GetParent(hwnd), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
		mwprsOverButton = false;
		break;
	
	case WM_MOUSEHOVER:
		nmh = makeNMHDR(N_HOVER, hwnd);
		SendMessage(GetParent(hwnd), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
		break;

	case WM_MOUSELEAVE:
		nmh = makeNMHDR(N_LEAVE, hwnd);
		SendMessage(GetParent(hwnd), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
		mouseOverButton = false;
		break;

	default:;
	}

	return CallWindowProc(oldWndProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK graphWinProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;
	HBRUSH backgrnd = CreateSolidBrush(GRAPHFG);
	
	switch(msg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		// Draw background
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, backgrnd);
		DeleteObject(backgrnd);

		// Draw Graph
		drawGraph(hdc);

		EndPaint(hwnd, &ps);
		return 0;
	default:;
	}

	return CallWindowProc(oldGraphWndProc, hwnd, msg, wp, lp);
}

bool summonWindow()
{
	hinstance = GetModuleHandle(NULL);

	WNDCLASSEX wc;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = winProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hinstance;
	wc.hIcon         = LoadIcon(hinstance, MAKEINTRESOURCE(IDB_ICON));
	wc.hIconSm       = LoadIcon(hinstance, MAKEINTRESOURCE(IDB_ICON));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(MAIN_BG);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = wnd_class.c_str();

	if(!RegisterClassEx(&wc))
	{
		return false;
	}

	DWORD ex_style, style;
	ex_style = WS_EX_APPWINDOW | WS_EX_TOPMOST;
	style = WS_POPUP;

	RECT wbox;
	wbox.left = 0;
	wbox.top = 0;
	wbox.right = (long)wndw;
	wbox.bottom = (long)wndh;
	
	AdjustWindowRectEx(&wbox, style, false, ex_style);

	hwindow = CreateWindowEx(
		ex_style,
		wnd_class.c_str(),
		wnd_title.c_str(),
		style,
		init_x, init_y,
		wbox.right - wbox.left,
		wbox.bottom - wbox.top,
		NULL,
		NULL,
		hinstance,
		NULL);

	if(!hwindow)
	{
		return false;
	}

	ShowWindow(hwindow, SW_SHOW);
	SetForegroundWindow(hwindow);
	SetFocus(hwindow);

	return true;
}

bool summonSideWindow()
{
	hinstance = GetModuleHandle(NULL);

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = winProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(BUTTPRS);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = side_wnd_class.c_str();

	if(!RegisterClassEx(&wc))
	{
		return false;
	}

	DWORD ex_style, style;
	ex_style = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	style = WS_POPUP;

	RECT wbox;
	wbox.left = 0;
	wbox.top = 0;
	wbox.right = (long)side_wndw;
	wbox.bottom = (long)side_wndh;

	AdjustWindowRectEx(&wbox, style, false, ex_style);

	hsidewnd = CreateWindowEx(
		ex_style,
		side_wnd_class.c_str(),
		NULL,
		style,
		init_x + wndw, init_y + 330,
		wbox.right - wbox.left,
		wbox.bottom - wbox.top,
		NULL,
		NULL,
		hinstance,
		NULL);

	if(!hsidewnd)
	{
		return false;
	}

	ShowWindow(hsidewnd, SW_HIDE);

	return true;
}

void moveWindow(WPARAM wparam)
{
	if(wparam == MK_LBUTTON && !mouseOverButton && !mwprsOverButton)
	{
		if(!mwprs)
		{
			mxp = mx;
			myp = my;
			mwprs = true;
		}

		RECT rec_main, rec_side;
		
		GetWindowRect(hwindow, &rec_main);
		GetWindowRect(hsidewnd, &rec_side);

		MoveWindow(hsidewnd, rec_side.left + mx - mxp, rec_side.top + my - myp, side_wndw, side_wndh, false);
		MoveWindow(hwindow, rec_main.left + mx - mxp, rec_main.top + my - myp, wndw, wndh, false);
	}
	else
	{
		mwprs = false;
	}
}

void summonChildWindows()
{
	// Main Utilities
	fonts.insert(Pif(24, makeStaticText("99999.99", Win(88, 371, 149, 59), hwindow, ELECTRO, 24, SS_NOTIFY)));
	makeStaticText("99999.99", Win(326, 371, 149, 59), hwindow, GAS, fonts[24], SS_NOTIFY);
	makeStaticText("99999.99", Win(564, 371, 149, 59), hwindow, WATER, fonts[24], SS_NOTIFY);
	// Small Utilities
	fonts.insert(Pif(20, makeStaticText("999.99", Win(238, 473, 87, 42), hwindow, TELE, 20, SS_NOTIFY)));
	makeStaticText("999.99", Win(476, 473, 87, 42), hwindow, INTNET, fonts[20], SS_NOTIFY);
	// Result
	fonts.insert(Pif(36, makeStaticText("99999.99", Win(253, 557, 294, 90), hwindow, RESULT, 36, SS_NOTIFY)));
	// Bottom Info Structure
	fonts.insert(Pif(26, makeStaticText("999", Win(9, 727, 56, 73), hwindow, BLAMOUNT, 26)));
	fonts.insert(Pif(12, makeStaticTextW(L"kW\u22C5h", Win(68, 763, 37, 14), hwindow, BLKWH, 12)));
	bitmaps.insert(Pib(IDB_UP, makeStaticImage(IDB_UP, Win(191, 727, 26, 73), hwindow, BLARROW)));
	makeStaticText("99%", Win(131, 727, 58, 73), hwindow, BLPERC, fonts[26]);
	makeStaticEmpty(Win(0, 727, 232, 73), hwindow, BOTLEFT);
	
	makeStaticText("999", Win(294, 741, 57, 59), hwindow, BMAMOUNT, fonts[26]);
	makeStaticTextW(L"m\u00B3", Win(352, 768, 22, 17), hwindow, BMM3, fonts[12]);
	bitmaps.insert(Pib(IDB_DOWN, makeStaticImage(IDB_DOWN, Win(482, 741, 26, 59), hwindow, BMARROW)));
	makeStaticText("99%", Win(422, 741, 58, 59), hwindow, BMPERC, fonts[26]);
	makeStaticEmpty(Win(270, 741, 260, 59), hwindow, BOTMID);

	makeStaticText("999", Win(588, 727, 52, 73), hwindow, BRAMOUNT, fonts[26]);
	makeStaticTextW(L"m\u00B3", Win(645, 763, 19, 15), hwindow, BRM3, fonts[12]);
	makeStaticImage(bitmaps[IDB_UP], Win(759, 727, 26, 73), hwindow, BRARROW);
	makeStaticText("99%", Win(699, 727, 58, 73), hwindow, BRPERC, fonts[26]);
	makeStaticEmpty(Win(568, 727, 232, 73), hwindow, BOTRIGHT);
	// +++
	bitmaps.insert(Pib(IDB_BIGPLUS, makeStaticImage(IDB_BIGPLUS, Win(261, 380, 40, 40), hwindow, BIGPLUS1)));
	makeStaticImage(bitmaps[IDB_BIGPLUS], Win(499, 380, 40, 40), hwindow, BIGPLUS2);
	bitmaps.insert(Pib(IDB_LTLPLUS, makeStaticImage(IDB_LTLPLUS, Win(384, 477, 34, 34), hwindow, LTLPLUS)));
	// Buttons
	bitmaps.insert(Pib(IDB_DETAILS, makeStaticImage(IDB_DETAILS, Win(4, 467, 30, 27), hwindow, LBUTIMG)));
	fonts.insert(Pif(1, makeStaticText("", Win(0, 430, 38, 101), hwindow, LBUTTON, 1, SS_NOTIFY)));
	bitmaps.insert(Pib(IDB_TARIFFS, makeStaticImage(IDB_TARIFFS, Win(766, 457, 30, 47), hwindow, RBUTIMG)));
	makeStaticText("", Win(762, 430, 38, 101), hwindow, RBUTTON, fonts[1], SS_NOTIFY);
	bitmaps.insert(Pib(IDB_MINBUT, makeStaticImage(IDB_MINBUT, Win(3, 14, 30, 8), hwindow, MINBIMG)));
	makeStaticText("", Win(0, 0, 36, 36), hwindow, MINBUT, fonts[1], SS_NOTIFY);
	bitmaps.insert(Pib(IDB_XBUT, makeStaticImage(IDB_XBUT, Win(767, 3, 30, 30), hwindow, XBUTIMG)));
	makeStaticText("", Win(764, 0, 36, 36), hwindow, XBUTTON, fonts[1], SS_NOTIFY);
	// Icons
	bitmaps.insert(Pib(IDB_ELECTRO, makeStaticImage(IDB_ELECTRO, Win(146, 319, 33, 49), hwindow, ICOELECT)));
	bitmaps.insert(Pib(IDB_GAS, makeStaticImage(IDB_GAS, Win(379, 319, 43, 49), hwindow, ICOGAS)));
	bitmaps.insert(Pib(IDB_WATER, makeStaticImage(IDB_WATER, Win(621, 319, 35, 49), hwindow, ICOWATER)));
	bitmaps.insert(Pib(IDB_PHONE, makeStaticImage(IDB_PHONE, Win(266, 444, 31, 26), hwindow, ICOTELE)));
	bitmaps.insert(Pib(IDB_INET, makeStaticImage(IDB_INET, Win(504, 439, 31, 31), hwindow, ICOINET)));
	// Graph
	makeStaticEmptyTop(Win(112, 60, 576, 251), hwindow, GRAPH);
	bitmaps.insert(Pib(IDB_LEGEND, makeStaticImage(IDB_LEGEND, Win(632, 253, 51, 53), hwindow, GRAPHLG)));
	bitmaps.insert(Pib(IDB_LEGEND_Q, makeStaticImage(IDB_LEGEND_Q, Win(679, 302, 9, 9), hwindow, GRAPHLQI)));
	makeStaticEmpty(Win(107, 55, 586, 261), hwindow, GRAPHB);
	bitmaps.insert(Pib(IDB_LEGEND_QE, makeStaticImage(IDB_LEGEND_QE, Win(679, 302, 9, 9), hwindow, GRAPHLQ, SS_NOTIFY)));
	ShowWindow(GetDlgItem(hwindow, GRAPHLG), SW_HIDE);
	// Side window
	// Edit fields
	fonts.insert(Pif(18, makeEdit("", Win(22, 17, 76, 31), hsidewnd, TARINET, 18)));
	makeEdit("", Win(22, 64, 76, 31), hsidewnd, TARELE1, fonts[18]);
	makeEdit("", Win(22, 111, 76, 31), hsidewnd, TARELE2, fonts[18]);
	makeEdit("", Win(22, 158, 76, 31), hsidewnd, TARGAS, fonts[18]);
	makeEdit("", Win(22, 205, 76, 31), hsidewnd, TARWATER, fonts[18]);
	makeEdit("", Win(22, 252, 76, 31), hsidewnd, TARPHONE, fonts[18]);
	makeEdit("", Win(202, 64, 76, 31), hsidewnd, VALELE1, fonts[18]);
	makeEdit("", Win(202, 111, 76, 31), hsidewnd, VALELE2, fonts[18]);
	makeEdit("", Win(202, 158, 76, 31), hsidewnd, VALGAS, fonts[18]);
	makeEdit("", Win(202, 205, 76, 31), hsidewnd, VALWATER, fonts[18]);
	// Icons
	bitmaps.insert(Pib(IDB_TARIFINET, makeStaticImage(IDB_TARIFINET, Win(135, 18, 29, 29), hsidewnd, TINETIMG)));
	bitmaps.insert(Pib(IDB_TARIFELE1, makeStaticImage(IDB_TARIFELE1, Win(137, 65, 26, 29), hsidewnd, TELE1IMG)));
	bitmaps.insert(Pib(IDB_TARIFELE2, makeStaticImage(IDB_TARIFELE2, Win(136, 112, 28, 29), hsidewnd, TELE2IMG)));
	bitmaps.insert(Pib(IDB_TARIFGAS, makeStaticImage(IDB_TARIFGAS, Win(137, 159, 25, 29), hsidewnd, TGASIMG)));
	bitmaps.insert(Pib(IDB_TARIFWATER, makeStaticImage(IDB_TARIFWATER, Win(140, 206, 20, 29), hsidewnd, TWATERIMG)));
	bitmaps.insert(Pib(IDB_TARIFPHONE, makeStaticImage(IDB_TARIFPHONE, Win(133, 253, 35, 29), hsidewnd, TPHONEIMG)));

	// Subclass buttons
	oldWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwindow, LBUTTON), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, RBUTTON), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, MINBUT), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, XBUTTON), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	// Subclass results
	SetWindowLongPtr(GetDlgItem(hwindow, ELECTRO), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, GAS), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, WATER), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, TELE), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, INTNET), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, RESULT), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);
	// Subclass graph
	oldGraphWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwindow, GRAPH), GWLP_WNDPROC, (LONG_PTR)graphWinProc);
	SetWindowLongPtr(GetDlgItem(hwindow, GRAPHLQ), GWLP_WNDPROC, (LONG_PTR)buttonWinProc);

	// Disable Double Clicks
	WNDCLASS wclass;
	GetClassInfoA(NULL, "STATIC", &wclass);
	LONG_PTR new_style = wclass.style & ~CS_DBLCLKS;
	SetClassLongPtr(GetDlgItem(hwindow, LBUTTON), GCL_STYLE, new_style);

	loadAdditButImg();
}

void mapGraph()
{
	// Win(112, 60, 576, 251)
	const double win_h = 251.0;
	const int dx = 48;

	MinMax mm = db.GetMinMax();
	SumArrays sa = db.GetSumArrays();

	const int init_x = sa.pre_m ? 0 : dx;

	lines_num = sa.n;
	double range = mm.max - mm.min;

	mm.max += range * 0.05;
	mm.min -= range * 0.05;
	range = mm.max - mm.min;

	// Electricity graph
	int x = init_x;
	for(int i = 0; i < sa.n; ++i)
	{
		ele_line[i].x = x;
		x += dx;

		ele_line[i].y = (int)round((mm.max - sa.ele[i])/range * win_h);
	}

	// Gas graph
	x = init_x;
	for(int i = 0; i < sa.n; ++i)
	{
		gas_line[i].x = x;
		x += dx;

		gas_line[i].y = (int)round((mm.max - sa.gas[i])/range * win_h);
	}

	// Water graph
	x = init_x;
	for(int i = 0; i < sa.n; ++i)
	{
		wat_line[i].x = x;
		x += dx;

		wat_line[i].y = (int)round((mm.max - sa.wat[i])/range * win_h);
	}

	InvalidateRect(GetDlgItem(hwindow, GRAPH), NULL, TRUE);
	UpdateWindow(GetDlgItem(hwindow, GRAPH));
	InvalidateRect(GetDlgItem(hwindow, GRAPHLQI), NULL, TRUE);
	UpdateWindow(GetDlgItem(hwindow, GRAPHLQI));
}

void drawGraph(HDC hdc)
{
	bool hdc_created = false;
	if(!hdc)
	{
		hdc_created = true;
		hdc = GetDC(GetDlgItem(hwindow, GRAPH));
	}

	Graphics g(hdc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	Pen ele_p(Color(255, 255, 255), 4);
	Pen gas_p(Color(0, 0, 0), 4);
	Pen wat_p(Color(119, 119, 119), 4);
	ele_p.SetStartCap(LineCapRound);
	ele_p.SetEndCap(LineCapRound);
	ele_p.SetLineJoin(LineJoinRound);
	gas_p.SetStartCap(LineCapRound);
	gas_p.SetEndCap(LineCapRound);
	gas_p.SetLineJoin(LineJoinRound);
	wat_p.SetStartCap(LineCapRound);
	wat_p.SetEndCap(LineCapRound);
	wat_p.SetLineJoin(LineJoinRound);

	g.DrawLines(&ele_p, (Point*)ele_line, lines_num);
	g.DrawLines(&gas_p, (Point*)gas_line, lines_num);
	g.DrawLines(&wat_p, (Point*)wat_line, lines_num);

	if(hdc_created)
	{
		ReleaseDC(GetDlgItem(hwindow, GRAPH), hdc);
	}
}

void onNotify(WPARAM wp, LPARAM lp)
{
	HBITMAP bm = NULL;
	HWND hwnd = ((LPNMHDR)lp)->hwndFrom;
	int id = (int)((LPNMHDR)lp)->idFrom;
	
	switch(((LPNMHDR)lp)->code)
	{
	case N_HOVER:
		switch(id)
		{
		case LBUTTON:
			if(mwprsOverButton)
			{
				SendMessage(GetDlgItem(hwindow, LBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DETAILPRS]);
				lbutcol = BUTTPRS;
				InvalidateRect(GetDlgItem(hwindow, LBUTIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else
			{
				SendMessage(GetDlgItem(hwindow, LBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DETAILHOV]);
			}
			break;
		case RBUTTON:
			if(tarif_on)
			{
				if(mwprsOverButton)
				{
					SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFONP]);
				}
				else
				{
					SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFONH]);
				}
				break;
			}
			
			if(mwprsOverButton)
			{
				SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFPRS]);
				rbutcol = BUTTPRS;
				InvalidateRect(GetDlgItem(hwindow, RBUTIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else
			{
				SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFHOV]);
			}
			break;
		case MINBUT:
			if(mwprsOverButton)
			{
				SendMessage(GetDlgItem(hwindow, MINBIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_MINBUTPRS]);
				mbutcol = BUTTPRS;
				InvalidateRect(GetDlgItem(hwindow, MINBIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else
			{
				SendMessage(GetDlgItem(hwindow, MINBIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_MINBUTHOV]);
			}
			break;
		case XBUTTON:
			if(mwprsOverButton)
			{
				SendMessage(GetDlgItem(hwindow, XBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_XBUTPRS]);
				xbutcol = BUTTPRS;
				InvalidateRect(GetDlgItem(hwindow, XBUTIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else
			{
				SendMessage(GetDlgItem(hwindow, XBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_XBUTHOV]);
			}
			break;
		case ELECTRO:
			ecb = MUTIL_BHOV;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case GAS:
			gcb = MUTIL_BHOV;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case WATER:
			wcb = MUTIL_BHOV;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case TELE:
			pcb = SUTIL_BHOV;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case INTNET:
			icb = SUTIL_BHOV;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case RESULT:
			rcb = RESULT_BH;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case GRAPHLQ:
			ShowWindow(GetDlgItem(hwindow, GRAPHLG), SW_SHOW);
			ShowWindow(GetDlgItem(hwindow, GRAPHLQI), SW_HIDE);
			break;
		default:;
		}
		break;

	case N_LEAVE:
		switch(id)
		{
		case LBUTTON:
			SendMessage(GetDlgItem(hwindow, LBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DETAILS]);
			if(mwprsOverButton)
			{
				lbutcol = BUTTONS;
				InvalidateRect(GetDlgItem(hwindow, LBUTIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		case RBUTTON:
			if(tarif_on)
			{
				SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFPRS]);
				break;
			}
			
			SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFS]);
			if(mwprsOverButton)
			{
				rbutcol = BUTTONS;
				InvalidateRect(GetDlgItem(hwindow, RBUTIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		case MINBUT:
			SendMessage(GetDlgItem(hwindow, MINBIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_MINBUT]);
			if(mwprsOverButton)
			{
				mbutcol = BUTTONS;
				InvalidateRect(GetDlgItem(hwindow, MINBIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		case XBUTTON:
			SendMessage(GetDlgItem(hwindow, XBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_XBUT]);
			if(mwprsOverButton)
			{
				xbutcol = BUTTONS;
				InvalidateRect(GetDlgItem(hwindow, XBUTIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		case ELECTRO:
			ecb = MUTIL_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case GAS:
			gcb = MUTIL_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case WATER:
			wcb = MUTIL_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case TELE:
			pcb = SUTIL_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case INTNET:
			icb = SUTIL_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case RESULT:
			rcb = RESULT_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case GRAPHLQ:
			ShowWindow(GetDlgItem(hwindow, GRAPHLG), SW_HIDE);
			ShowWindow(GetDlgItem(hwindow, GRAPHLQI), SW_SHOW);
			break;
		default:;
		}
		break;

	case N_MDOWN:
		switch(id)
		{
		case LBUTTON:
			SendMessage(GetDlgItem(hwindow, LBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DETAILPRS]);
			lbutcol = BUTTPRS;
			InvalidateRect(GetDlgItem(hwindow, LBUTIMG), NULL, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case RBUTTON:
			if(tarif_on)
			{
				SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFONP]);
				break;
			}

			SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFPRS]);
			rbutcol = BUTTPRS;
			InvalidateRect(GetDlgItem(hwindow, RBUTIMG), NULL, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case MINBUT:
			SendMessage(GetDlgItem(hwindow, MINBIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_MINBUTPRS]);
			mbutcol = BUTTPRS;
			InvalidateRect(GetDlgItem(hwindow, MINBIMG), NULL, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case XBUTTON:
			SendMessage(GetDlgItem(hwindow, XBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_XBUTPRS]);
			xbutcol = BUTTPRS;
			InvalidateRect(GetDlgItem(hwindow, XBUTIMG), NULL, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case ELECTRO:
			ect = RESULT_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case GAS:
			gct = RESULT_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case WATER:
			wct = RESULT_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case TELE:
			pct = RESULT_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case INTNET:
			ict = RESULT_B;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case RESULT:
			rct = BOTINF_PTU;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		default:;
		}
		break;

	case N_MUP:
		switch(id)
		{
		case LBUTTON:
			SendMessage(GetDlgItem(hwindow, LBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DETAILHOV]);
			lbutcol = BUTTONS;
			InvalidateRect(GetDlgItem(hwindow, LBUTIMG), NULL, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case RBUTTON:
			if(!tarif_on)
			{
				SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFONH]);
				tarif_on = true;
				ShowWindow(hsidewnd, SW_SHOW);
				SetForegroundWindow(hwindow);
			}
			else
			{
				// Tariff saving action
				if(!recalcMonth())
				{
					return;
				}

				SendMessage(GetDlgItem(hwindow, RBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_TARIFFHOV]);
				rbutcol = BUTTONS;
				InvalidateRect(GetDlgItem(hwindow, RBUTIMG), NULL, TRUE);
				InvalidateRect(hwnd, NULL, TRUE);
				tarif_on = false;
				ShowWindow(hsidewnd, SW_HIDE);
			}
			break;
		case MINBUT:
			SendMessage(GetDlgItem(hwindow, MINBIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_MINBUTHOV]);
			mbutcol = BUTTONS;
			InvalidateRect(GetDlgItem(hwindow, MINBIMG), NULL, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			// Actual work, not cosmetics
			ShowWindow(hwindow, SW_MINIMIZE);
			ShowWindow(hsidewnd, SW_HIDE);
			break;
		case XBUTTON:
			SendMessage(GetDlgItem(hwindow, XBUTIMG), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_XBUTHOV]);
			xbutcol = BUTTONS;
			InvalidateRect(GetDlgItem(hwindow, XBUTIMG), NULL, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			// Actual work, not cosmetics
			PostQuitMessage(0);
			break;
		case ELECTRO:
			ect = MUTIL_T;
			InvalidateRect(hwnd, NULL, TRUE);
			copyToClipboard(ELECTRO);
			break;
		case GAS:
			gct = MUTIL_T;
			InvalidateRect(hwnd, NULL, TRUE);
			copyToClipboard(GAS);
			break;
		case WATER:
			wct = MUTIL_T;
			InvalidateRect(hwnd, NULL, TRUE);
			copyToClipboard(WATER);
			break;
		case TELE:
			pct = SUTIL_T;
			InvalidateRect(hwnd, NULL, TRUE);
			copyToClipboard(TELE);
			break;
		case INTNET:
			ict = SUTIL_T;
			InvalidateRect(hwnd, NULL, TRUE);
			copyToClipboard(INTNET);
			break;
		case RESULT:
			rct = RESULT_T;
			InvalidateRect(hwnd, NULL, TRUE);
			copyResultToClipboard();
			break;
		default:;
		}
		break;

	default:;
	}
}

bool calcNewMonthUI()
{
	ReadTariff rt;
	if(!(rt = getReadTariff()).state)
	{
		return false;
	}
	
	Month m = getDate();
	m.SetReadings(rt.rd);
	db.SetTariffs(rt.e1, rt.e2, rt.g, rt.w, rt.i, rt.p);

	Sums sums = m.GetSums(db, SUM_NEW);

	displaySums(sums, m.GetDeltas(), m.GetPercents());

	db.AddMonth(m);

	return true;
}

bool recalcMonth()
{
	ReadTariff rt;
	if(!(rt = getReadTariff()).state)
	{
		return false;
	}

	Month& m = db.GetLastMonth();
	m.SetReadings(rt.rd);
	db.SetTariffs(rt.e1, rt.e2, rt.g, rt.w, rt.i, rt.p);

	Sums sums;
	memset(&sums, 0, sizeof(Sums));
	if(db.MonthsCount() > 1)
	{
		sums = m.GetSums(db, SUM_RECALC);
	}

	displaySums(sums, m.GetDeltas(), m.GetPercents());

	mapGraph();

	return true;
}

bool calcNewMonthNet(Readings rd)
{
	Month m;
	Sums sums;
	Word d = db.GetLastMonth().Date();

	if(d) // At least 1 month present in DB
	{
		Byte y = db.GetLastMonth().Year();
		Byte mo = db.GetLastMonth().Mon();

		if(mo < 12)
		{
			++mo;
		}
		else
		{
			mo = 1;
			++y;
		}
		
		d = (Word)mo << 8 | (Word)y;
		m.SetDate(d);
		m.SetReadings(rd);
		sums = m.GetSums(db, SUM_NEW);
	}
	else // Empty DB case
	{
		memset(&m, 0, sizeof(Month));
		m.SetDate(getDate());
		m.SetReadings(rd);
		memset(&sums, 0, sizeof(Sums));
	}

	displaySums(sums, m.GetDeltas(), m.GetPercents());
	db.AddMonth(m);
	setTariffs(rd);

	mapGraph();

	return true;
}

ReadTariff getReadTariff()
{
	ReadTariff rt;
	rt.state = false;
	int e1, e2, g, w;

	const char* winp = "Wrong input";
	const char* int_msg = "Readings must contain only integer numbers (0-9) and not 0!";
	const char* doub_msg = "Tariffs must contain only real numbers (0-9 and\\or one \".\" dot) and not 0!";

	if((e1 = editTxti(VALELE1)) <= 0)
	{
		MessageBoxA(hsidewnd, int_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((e2 = editTxti(VALELE2)) <= 0)
	{
		MessageBoxA(hsidewnd, int_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((g = editTxti(VALGAS)) <= 0)
	{
		MessageBoxA(hsidewnd, int_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((w = editTxti(VALWATER)) <= 0)
	{
		MessageBoxA(hsidewnd, int_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	
	double te1, te2, tg, tw, ti, tp;
	if((te1 = editTxtd(TARELE1)) <= 0.0)
	{
		MessageBoxA(hsidewnd, doub_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((te2 = editTxtd(TARELE2)) <= 0.0)
	{
		MessageBoxA(hsidewnd, doub_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((tg = editTxtd(TARGAS)) <= 0.0)
	{
		MessageBoxA(hsidewnd, doub_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((tw = editTxtd(TARWATER)) <= 0.0)
	{
		MessageBoxA(hsidewnd, doub_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((ti = editTxtd(TARINET)) <= 0.0)
	{
		MessageBoxA(hsidewnd, doub_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}
	if((tp = editTxtd(TARPHONE)) <= 0.0)
	{
		MessageBoxA(hsidewnd, doub_msg, winp, MB_OK | MB_ICONWARNING);
		return rt;
	}

	rt.rd.ele_1 = e1;
	rt.rd.ele_2 = e2;
	rt.rd.gas = g;
	rt.rd.water = w;
	rt.e1 = te1;
	rt.e2 = te2;
	rt.g = tg;
	rt.w = tw;
	rt.i = ti;
	rt.p = tp;
	rt.state = true;

	return rt;
}

void displaySums(const Sums& s, const Deltas& d, Percents p)
{
	SendMessage(GetDlgItem(hwindow, ELECTRO), WM_SETTEXT, NULL, (LPARAM)dToStr(s.electro).c_str());
	SendMessage(GetDlgItem(hwindow, GAS), WM_SETTEXT, NULL, (LPARAM)dToStr(s.gas).c_str());
	SendMessage(GetDlgItem(hwindow, WATER), WM_SETTEXT, NULL, (LPARAM)dToStr(s.water).c_str());
	SendMessage(GetDlgItem(hwindow, TELE), WM_SETTEXT, NULL, (LPARAM)dToStr(s.phone).c_str());
	SendMessage(GetDlgItem(hwindow, INTNET), WM_SETTEXT, NULL, (LPARAM)dToStr(s.inet).c_str());

	SendMessage(GetDlgItem(hwindow, RESULT), WM_SETTEXT, NULL, (LPARAM)dToStr(s.main_sum).c_str());

	// Fill bottom info structure
	// Fill amounts
	if((d.e1 + d.e2) > 999)
	{
		SendMessage(GetDlgItem(hwindow, BLAMOUNT), WM_SETFONT, (WPARAM)fonts[18], TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BLAMOUNT), WM_SETFONT, (WPARAM)fonts[26], TRUE);
	}

	if(d.g > 999)
	{
		SendMessage(GetDlgItem(hwindow, BMAMOUNT), WM_SETFONT, (WPARAM)fonts[18], TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BMAMOUNT), WM_SETFONT, (WPARAM)fonts[26], TRUE);
	}

	if(d.w > 999)
	{
		SendMessage(GetDlgItem(hwindow, BRAMOUNT), WM_SETFONT, (WPARAM)fonts[18], TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BRAMOUNT), WM_SETFONT, (WPARAM)fonts[26], TRUE);
	}

	SendMessage(GetDlgItem(hwindow, BLAMOUNT), WM_SETTEXT, NULL, (LPARAM)to_string(d.e1 + d.e2).c_str());
	SendMessage(GetDlgItem(hwindow, BMAMOUNT), WM_SETTEXT, NULL, (LPARAM)to_string(d.g).c_str());
	SendMessage(GetDlgItem(hwindow, BRAMOUNT), WM_SETTEXT, NULL, (LPARAM)to_string(d.w).c_str());

	// Fill percents
	if(p.ele > 0)
	{
		SendMessage(GetDlgItem(hwindow, BLARROW), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_UP]);
		lperccol = BOTINF_PTU;
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BLARROW), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DOWN]);
		lperccol = BOTINF_PTD;
		p.ele *= -1;
	}

	if(p.gas > 0)
	{
		SendMessage(GetDlgItem(hwindow, BMARROW), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_UP]);
		mperccol = BOTINF_PTU;
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BMARROW), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DOWN]);
		mperccol = BOTINF_PTD;
		p.gas *= -1;
	}

	if(p.water > 0)
	{
		SendMessage(GetDlgItem(hwindow, BRARROW), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_UP]);
		rperccol = BOTINF_PTU;
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BRARROW), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmaps[IDB_DOWN]);
		rperccol = BOTINF_PTD;
		p.water *= -1;
	}

	if(p.ele > 99)
	{
		SendMessage(GetDlgItem(hwindow, BLPERC), WM_SETFONT, (WPARAM)fonts[18], TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BLPERC), WM_SETFONT, (WPARAM)fonts[26], TRUE);
	}

	if(p.gas > 99)
	{
		SendMessage(GetDlgItem(hwindow, BMPERC), WM_SETFONT, (WPARAM)fonts[18], TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BMPERC), WM_SETFONT, (WPARAM)fonts[26], TRUE);
	}

	if(p.water > 99)
	{
		SendMessage(GetDlgItem(hwindow, BRPERC), WM_SETFONT, (WPARAM)fonts[18], TRUE);
	}
	else
	{
		SendMessage(GetDlgItem(hwindow, BRPERC), WM_SETFONT, (WPARAM)fonts[26], TRUE);
	}

	SendMessage(GetDlgItem(hwindow, BLPERC), WM_SETTEXT, NULL, (LPARAM)(to_string(p.ele) + '%').c_str());
	SendMessage(GetDlgItem(hwindow, BMPERC), WM_SETTEXT, NULL, (LPARAM)(to_string(p.gas) + '%').c_str());
	SendMessage(GetDlgItem(hwindow, BRPERC), WM_SETTEXT, NULL, (LPARAM)(to_string(p.water) + '%').c_str());
}

void setTariffs(const Readings& r)
{
	SendMessage(GetDlgItem(hsidewnd, TARINET), WM_SETTEXT, NULL, (LPARAM)dToStrFull(db.TariffInet()).c_str());
	SendMessage(GetDlgItem(hsidewnd, TARELE1), WM_SETTEXT, NULL, (LPARAM)dToStrFull(db.TariffElectro().less_min).c_str());
	SendMessage(GetDlgItem(hsidewnd, TARELE2), WM_SETTEXT, NULL, (LPARAM)dToStrFull(db.TariffElectro().more_min).c_str());
	SendMessage(GetDlgItem(hsidewnd, TARGAS), WM_SETTEXT, NULL, (LPARAM)dToStrFull(db.TariffGas()).c_str());
	SendMessage(GetDlgItem(hsidewnd, TARWATER), WM_SETTEXT, NULL, (LPARAM)dToStrFull(db.TariffWater()).c_str());
	SendMessage(GetDlgItem(hsidewnd, TARPHONE), WM_SETTEXT, NULL, (LPARAM)dToStrFull(db.TariffPhone()).c_str());

	SendMessage(GetDlgItem(hsidewnd, VALELE1), WM_SETTEXT, NULL, (LPARAM)to_string(r.ele_1).c_str());
	SendMessage(GetDlgItem(hsidewnd, VALELE2), WM_SETTEXT, NULL, (LPARAM)to_string(r.ele_2).c_str());
	SendMessage(GetDlgItem(hsidewnd, VALGAS), WM_SETTEXT, NULL, (LPARAM)to_string(r.gas).c_str());
	SendMessage(GetDlgItem(hsidewnd, VALWATER), WM_SETTEXT, NULL, (LPARAM)to_string(r.water).c_str());
}

LRESULT onCtlColorStatic(HDC hdc, HWND hcont)
{
	int txt = 0, bg = 0;
	int cntr_id = GetDlgCtrlID(hcont);

	switch(cntr_id)
	{
	case ELECTRO:
		txt = ect;
		bg = ecb;
		break;
	case GAS:
		txt = gct;
		bg = gcb;
		break;
	case WATER:
		txt = wct;
		bg = wcb;
		break;

	case TELE:
		txt = pct;
		bg = pcb;
		break;
	case INTNET:
		txt = ict;
		bg = icb;
		break;

	case RESULT:
		txt = rct;
		bg = rcb;
		break;

	case BOTLEFT:
	case BOTMID:
	case BOTRIGHT:
		bg = BOTINF_B;
		break;

	case BLPERC:
		txt = lperccol;
		bg = BOTINF_B;
		break;

	case BRPERC:
		txt = rperccol;
		bg = BOTINF_B;
		break;

	case BMPERC:
		txt = mperccol;
		bg = BOTINF_B;
		break;

	case BLAMOUNT:
	case BMAMOUNT:
	case BRAMOUNT:
		txt = BOTINF_T;
		bg = BOTINF_B;
		break;
	case BLKWH:
	case BMM3:
	case BRM3:
		txt = BOTINF_ST;
		bg = BOTINF_B;
		break;

	case LBUTTON:
		bg = lbutcol;
		break;
	case RBUTTON:
		bg = rbutcol;
		break;
	case XBUTTON:
		bg = xbutcol;
		break;
	case MINBUT:
		bg = mbutcol;
		break;

	case GRAPH:
		bg = GRAPHFG;
		break;

	case GRAPHB:
		bg = GRAPHBG;
		break;

	case TARINET:
	case TARELE1:
	case TARELE2:
	case TARGAS:
	case TARWATER:
	case TARPHONE:
	case VALELE1:
	case VALELE2:
	case VALGAS:
	case VALWATER:
		txt = RESULT_B;
		bg = MAIN_BG;
		break;

	default:
		return 0;
	}

	SetTextColor(hdc, txt);
	SetBkColor(hdc, bg);

	MapIbrush::iterator it = brushes.find(bg);
	if(it == brushes.end())
	{
		pair<MapIbrush::iterator, bool> res = brushes.insert(Pibrush(bg, CreateSolidBrush(bg)));
		return (LRESULT)res.first->second;
	}
	else
	{
		return (LRESULT)it->second;
	}
}

void copyToClipboard(int menu)
{
	char buff[100];
	GetDlgItemText(hwindow, menu, buff, 100);
	const int l = (int)strlen(buff) + 1;
	HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, l);
	memcpy(GlobalLock(hmem), buff, l);
	GlobalUnlock(hmem);
	OpenClipboard(hwindow);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hmem);
	CloseClipboard();
}

void copyResultToClipboard()
{
	char buff[100];
	string out;
	GetDlgItemText(hwindow, ELECTRO, buff, 100);
	out += "(E)" + string(buff) + " + ";
	GetDlgItemText(hwindow, GAS, buff, 100);
	out += "(G)" + string(buff) + " + ";
	GetDlgItemText(hwindow, WATER, buff, 100);
	out += "(W)" + string(buff) + " + ";
	GetDlgItemText(hwindow, TELE, buff, 100);
	out += "(T)" + string(buff) + " + ";
	GetDlgItemText(hwindow, INTNET, buff, 100);
	out += "(I)" + string(buff);
	GetDlgItemText(hwindow, RESULT, buff, 100);
	out += " = " + string(buff);

	const int l = (int)strlen(out.c_str()) + 1;
	HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, l);
	memcpy(GlobalLock(hmem), out.c_str(), l);
	GlobalUnlock(hmem);
	OpenClipboard(hwindow);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hmem);
	CloseClipboard();
}

void loadAdditButImg()
{
	bitmaps.insert(Pib(IDB_DETAILHOV, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_DETAILHOV))));
	bitmaps.insert(Pib(IDB_DETAILPRS, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_DETAILPRS))));
	bitmaps.insert(Pib(IDB_TARIFFHOV, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_TARIFFHOV))));
	bitmaps.insert(Pib(IDB_TARIFFPRS, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_TARIFFPRS))));
	bitmaps.insert(Pib(IDB_MINBUTHOV, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_MINBUTHOV))));
	bitmaps.insert(Pib(IDB_MINBUTPRS, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_MINBUTPRS))));
	bitmaps.insert(Pib(IDB_XBUTHOV, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_XBUTHOV))));
	bitmaps.insert(Pib(IDB_XBUTPRS, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_XBUTPRS))));
	bitmaps.insert(Pib(IDB_TARIFFONH, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_TARIFFONH))));
	bitmaps.insert(Pib(IDB_TARIFFONP, LoadBitmap(hinstance, MAKEINTRESOURCE(IDB_TARIFFONP))));
}

int editTxti(int menu)
{
	char buff[100];
	GetDlgItemText(hsidewnd, menu, buff, 100);
	string tmp(buff);

	if(tmp.empty())
	{
		return 0;
	}

	for(int i = 0; i < tmp.size(); ++i)
	{
		char c = tmp[i];
		if(c < '0' || c > '9')
		{
			return -1;
		}
	}

	return stoi(tmp);
}

double editTxtd(int menu)
{
	char buff[100];
	GetDlgItemText(hsidewnd, menu, buff, 100);
	string tmp(buff);

	if(tmp.empty())
	{
		return 0.0;
	}

	bool dot_found = false;
	for(int i = 0; i < tmp.size(); ++i)
	{
		char c = tmp[i];
		if(c != '.' && (c < '0' || c > '9'))
		{
			return -1.0;
		}

		if(c == '.' && !dot_found)
		{
			dot_found = true;
		}
		else if(c == '.' && dot_found)
		{
			return -1.0;
		}
	}

	return stod(tmp);
}

void cleanUp()
{
	for(MapIb::iterator it = bitmaps.begin(); it != bitmaps.end(); ++it)
	{
		DeleteObject(it->second);
	}

	for(MapIf::iterator it = fonts.begin(); it != fonts.end(); ++it)
	{
		DeleteObject(it->second);
	}

	for(MapIbrush::iterator it = brushes.begin(); it != brushes.end(); ++it)
	{
		DeleteObject(it->second);
	}
}