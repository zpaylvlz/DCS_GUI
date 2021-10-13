#include <Windows.h>
#include <tchar.h>
#include <string>
#include <cstdlib>
#include <CommCtrl.h>
#include <uxtheme.h>
#include <winuser.h>
#include <atlstr.h>
#include <iostream>
#include <fstream>
#include <shlobj.h>

//Set controls to morden styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//WM_COMMAND Event
#define FILE_OPEN_EVENT 111
#define FILE_EXIT_EVENT 112
#define HELP_ABOUT_TAG 113
#define BTN_CONFIRM 114
#define RBTN_SWITCH_ITER 115
#define RBTN_SWITCH_TIME 116
#define COMBOBOX_SELECT_KPI 117
#define PRGS_CANCEL 118
#define PRGS_PREV_STEP 119
#define PRGS_DONE 120
#define ERROR_CONFIRM 121
#define SEND_ERROR_MSG 122
#define SELECT_FOLDER 123
#define OPEN_ERROR_WINDOW 124
#define PGSB_Update 689//this is a command for testing

// Handle of MainWindow controls
HMENU hMenu;

HWND MainWindow;
HWND hEdit_Database_IP, hEdit_Database_Port, hEdit_Database_Account, hEdit_Database_Password,
hEdit_Datatable_Name, hEdit_Setup_Time, hEdit_Process_Time, hEdit_Postop_Time, hEdit_Search_Param,
hEdit_Time_Gap, hEdit_Search_Depth, hCombox_KPI, hRBTN_ITER, hRBTN_TIME, hStatic_Time_Gap,
hStatic_Search_Param, hGBX1;

// Handle of ShowProgress controls
HWND PreprocessProgressBar, RunTimeProgressBar, hEdit_FileName, hEdit_FilePath,
hBtn_Cancel, hBtn_Prev, hBtn_Done;

HWND Error_Window;
HWND hEdit_Error_Message;

std::string DatabaseIP = "", DataTableName = "", DatabasePort = "", DatabaseAccount = "",
DatabasePassword = "", KPI = "", FileName = "", FilePath = "";
int Setup_Time = 0, Process_Time = 0, Postop_Time = 0, Search_Param = 0, Optimize_Time_Gap = 0;
bool isSearchByIterarion = false, isErrorWindowOpened = false, isProgressWindowOpened = false;
int iProgressPosition = 0;

static HBRUSH DefaultBKBrush = CreateSolidBrush(RGB(255, 255, 255));

void Addmenu(HWND);
void AddControls(HWND);
void AddProgressControl(HWND);
void AddErrorControl(HWND);

void displayProgress(HWND);
void displayError(HWND);

void WinProgress(HINSTANCE);//Initialize of ShowProgress Window
void SubWinError(HINSTANCE);// Initialize of ShowError Window

LRESULT CALLBACK ProgressProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ErrorProc(HWND, UINT, WPARAM, LPARAM);

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char*)lpData;
		std::cout << "path: " << tmp << std::endl;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::string BrowseFolder()
{
	TCHAR path[MAX_PATH];

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = ("Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)_T("C:\\");

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc* imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return "C:\\";
}


std::string Get_Edit_Text_Value(HWND Current_hWnd) {
	wchar_t GetControlValue[4096] = { 0 };
	GetWindowTextW(Current_hWnd, GetControlValue, 4096);
	char Convert_Buffer[4096];
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, GetControlValue, -1, Convert_Buffer, 4096, &DefChar, NULL);
	std::string SetByValue(Convert_Buffer);
	return SetByValue;
}

std::string Get_ComboBox_Text_Value(HWND Current_hWnd) {
	wchar_t GetControlValue[4096] = { 0 };
	int KPI_Index = SendMessage(Current_hWnd, CB_GETCURSEL, 0, 0);
	SendMessage(hCombox_KPI, CB_GETLBTEXT, KPI_Index, (LPARAM)GetControlValue);
	char Convert_Buffer[4096] = { 0 };
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, GetControlValue, -1, Convert_Buffer, 4096, &DefChar, NULL);
	std::string SetByValue(Convert_Buffer);
	return SetByValue;
}

int Get_Edit_Int_Value(HWND Current_hWnd) {
	wchar_t GetControlValue[10] = { 0 };
	GetWindowTextW(Current_hWnd, GetControlValue, 10);
	// to get edit control's input text, the instance must be declare as global, or fetch by GetDlgItem function
	int SetByValue = 0;
	SetByValue = wcstol(GetControlValue, 0, 10);
	return SetByValue;
}

//Callback function for main window
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
		static HINSTANCE hInstance;
	case WM_COMMAND:
		switch (wp) {
		case FILE_OPEN_EVENT: {
		}
							break;
		case FILE_EXIT_EVENT:
			DestroyWindow(hWnd);
			break;
		case HELP_ABOUT_TAG:
			MessageBox(NULL, _T("Liver Broken Co. Ltd."), _T("DCS_Ver2.0"), MB_OK);
			break;
		case BTN_CONFIRM: {

			DatabaseIP = Get_Edit_Text_Value(hEdit_Database_IP);
			DatabasePort = Get_Edit_Text_Value(hEdit_Database_Port);
			DatabaseAccount = Get_Edit_Text_Value(hEdit_Database_Account);
			DatabasePassword = Get_Edit_Text_Value(hEdit_Database_Password);
			DataTableName = Get_Edit_Text_Value(hEdit_Datatable_Name);
			Setup_Time = Get_Edit_Int_Value(hEdit_Setup_Time);
			Process_Time = Get_Edit_Int_Value(hEdit_Process_Time);
			Postop_Time = Get_Edit_Int_Value(hEdit_Postop_Time);
			Search_Param = Get_Edit_Int_Value(hEdit_Search_Param);
			Optimize_Time_Gap = Get_Edit_Int_Value(hEdit_Time_Gap);
			KPI = Get_ComboBox_Text_Value(hCombox_KPI);
			FileName = Get_Edit_Text_Value(hEdit_FileName);
			FilePath = Get_Edit_Text_Value(hEdit_FilePath);

			//testing section for showing all sub window, set it to the right position
			if (!isProgressWindowOpened) {
				isProgressWindowOpened = true;
				displayProgress(hWnd);
			}

			/*std::ofstream myfile;
			myfile.open("example.txt");
			myfile << DatabaseIP << std::endl;
			myfile << DatabasePort << std::endl;
			myfile << DatabaseAccount << std::endl;
			myfile << DatabasePassword << std::endl;
			myfile << DataTableName << std::endl;
			myfile << Setup_Time << std::endl;
			myfile << Process_Time << std::endl;
			myfile << Postop_Time << std::endl;
			myfile << Search_Param << std::endl;
			myfile << Optimize_Time_Gap << std::endl;
			myfile << KPI << std::endl;
			myfile << FileName << std::endl;
			myfile << FilePath << std::endl;
			myfile.close();*/
			break;
		}
		case RBTN_SWITCH_ITER: {
			SendMessage(hRBTN_ITER, BM_SETCHECK, BST_CHECKED, NULL);
			SendMessage(hRBTN_TIME, BM_SETCHECK, BST_UNCHECKED, NULL);
			SetWindowTextW(hStatic_Search_Param, (L"Iteration"));
			SetWindowTextW(hStatic_Time_Gap, (L"個Iteration"));
			int index = GetWindowTextLength(hEdit_Search_Param);
			SetFocus(hEdit_Search_Param); // set focus
			SendMessageA(hEdit_Search_Param, EM_SETSEL, (WPARAM)0, (LPARAM)index); // set selection - end of text
			SendMessageA(hEdit_Search_Param, EM_REPLACESEL, 0, (LPARAM)(_T("120")));
			isSearchByIterarion = true;
			break;
		}
		case RBTN_SWITCH_TIME: {
			SendMessage(hRBTN_TIME, BM_SETCHECK, BST_CHECKED, NULL);
			SendMessage(hRBTN_ITER, BM_SETCHECK, BST_UNCHECKED, NULL);
			SetWindowTextW(hStatic_Search_Param, (L"分鐘"));
			SetWindowTextW(hStatic_Time_Gap, (L"分鐘"));
			int index = GetWindowTextLength(hEdit_Search_Param);
			SetFocus(hEdit_Search_Param); // set focus
			SendMessageA(hEdit_Search_Param, EM_SETSEL, (WPARAM)0, (LPARAM)index); // set selection - end of text
			SendMessageA(hEdit_Search_Param, EM_REPLACESEL, 0, (LPARAM)(_T("40")));
			isSearchByIterarion = true;
			isSearchByIterarion = false;
			break;
		}
		case SELECT_FOLDER: {
			std::string Selected_filepath = BrowseFolder();
			int index = GetWindowTextLength(hEdit_FilePath);
			SetFocus(hEdit_FilePath); // set focus
			SendMessageA(hEdit_FilePath, EM_SETSEL, (WPARAM)0, (LPARAM)index); // set selection - end of text
			SendMessageA(hEdit_FilePath, EM_REPLACESEL, 0, (LPARAM)Selected_filepath.c_str());
			break;
		}
		case OPEN_ERROR_WINDOW:
			if (!isErrorWindowOpened) {
				isErrorWindowOpened = true;
				displayError(hWnd);
			}
			break;
		case PGSB_Update:
			//iProgressPosition += 10;
			//SendMessage(PreprocessProgressBar, PBM_SETPOS, iProgressPosition, 0);
			//SendMessage(RunTimeProgressBar	, PBM_SETPOS, iProgressPosition, 0);
			//ShowWindow(PreprocessProgressBar, SW_HIDE);

			std::string buffer = "append this!\r\n";
			int index = GetWindowTextLength(hEdit_Error_Message);
			SetFocus(hEdit_Error_Message); // set focus
			SendMessageA(hEdit_Error_Message, EM_SETSEL, (WPARAM)index, (LPARAM)index); // set selection - end of text
			SendMessageA(hEdit_Error_Message, EM_REPLACESEL, 0, (LPARAM)buffer.c_str());
			break;
		}
		break;
	case WM_CREATE: {
		Addmenu(hWnd);
		AddControls(hWnd);
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT     ps;
		HDC             hdc;
		HBRUSH BRSH = CreateSolidBrush(RGB(0, 128, 0));
		hdc = BeginPaint(hWnd, &ps);
		HGDIOBJ FillContent = SelectObject(hdc, CreateSolidBrush(RGB(144, 192, 168)));
		HGDIOBJ FillBorder = SelectObject(hdc, CreatePen(PS_INSIDEFRAME, 1, RGB(255, 255, 255)));
		RoundRect(hdc, 10, 8, 510, 220, 8, 8);
		RoundRect(hdc, 530, 8, 1030, 220, 8, 8);
		RoundRect(hdc, 10, 228, 510, 440, 8, 8);
		RoundRect(hdc, 530, 228, 1030, 390, 8, 8);
		SelectObject(hdc, FillContent);
		SelectObject(hdc, FillBorder);
		//SetRect(&secondbackground, 3, 3, 297, 495);
		//FrameRect(hdc, &secondbackground, BRSH);
		
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CTLCOLORSTATIC: {
		HDC hdcStatic = (HDC)wp; // or obtain the static handle in some other way
		
		if (lp == (LPARAM)hEdit_FilePath) {
			SetBkMode((HDC)wp, OPAQUE);
			SetTextColor(hdcStatic, RGB(0, 255, 0)); // text color
			SetBkColor(hdcStatic, RGB(0, 0, 0));
			return (INT_PTR)CreateSolidBrush(RGB(0, 0, 0));
		}
		else
		{
			SetBkMode((HDC)wp, OPAQUE);
			SetTextColor(hdcStatic, RGB(0, 0, 0)); // text color
			SetBkColor(hdcStatic, RGB(144, 192, 168));
			return (INT_PTR)CreateSolidBrush(RGB(144, 192, 168));
		}
		break;
	}
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
	//HINSTANCE: Program entry point
	WNDCLASSW WinC = { 0 };
	HBRUSH WinBkBrush = CreateSolidBrush(RGB(255, 255, 255));
	WinC.hbrBackground = WinBkBrush;
	WinC.hCursor = LoadCursor(NULL, IDC_ARROW);
	WinC.hInstance = hInst;
	WinC.lpszClassName = L"WindowClass";
	WinC.lpfnWndProc = WindowProcedure;// set call back function

	if (!RegisterClassW(&WinC)) {
		return -1;//Create window fail
	}

	WinProgress(hInst);
	SubWinError(hInst);
	//
	MainWindow = CreateWindowW(L"WindowClass", L"Window", (WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) | WS_VISIBLE, 0, 0, 1080, 520
		, NULL, NULL, NULL, NULL);
	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, NULL, NULL)) {
		if (IsDialogMessage(MainWindow, &msg)) {

		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}
	return 0;
}
void Addmenu(HWND hWnd) {

	hMenu = CreateMenu();
	HMENU hFileMenu = CreateMenu();
	HMENU hHelpMenu = CreateMenu();

	AppendMenu(hFileMenu, MF_POPUP, FILE_OPEN_EVENT, _T("Open File"));
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, FILE_EXIT_EVENT, _T("Exit"));

	AppendMenu(hHelpMenu, MF_STRING, HELP_ABOUT_TAG, _T("About"));

	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, _T("File"));
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, _T("Help"));

	SetMenu(hWnd, hMenu);
}
void AddControls(HWND hWnd) {
	LPCWSTR KPIItem[] = { L"最小化最晚完工時間", L"最大化三天內完成工序數", L"最小化實際與最短加工距離",L"最大化完工時間與交期距離" };
	//Section 1: Database Setting
	hGBX1 = CreateWindowW(L"Button", L"資料輸入", WS_VISIBLE | WS_CHILD | BS_GROUPBOX , 10, 10, 500, 210, hWnd,
		NULL, NULL, NULL);
	CreateWindowW(L"static", L"資料庫IP:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		20, 50, 70, 30, hWnd, NULL, NULL, NULL);
	hEdit_Database_IP = CreateWindowW(L"Edit", L"140.114.54.25", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		110, 50, 120, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"資料庫Port:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		250, 50, 80, 30, hWnd, NULL, NULL, NULL);
	hEdit_Database_Port = CreateWindowW(L"Edit", L"1433", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		350, 50, 50, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"資料庫帳號:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		20, 110, 90, 30, hWnd, NULL, NULL, NULL);
	hEdit_Database_Account = CreateWindowW(L"Edit", L"sa", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		110, 110, 100, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"資料庫密碼:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		250, 110, 90, 30, hWnd, NULL, NULL, NULL);
	hEdit_Database_Password = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL | ES_PASSWORD,
		350, 110, 100, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"資料表名稱:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		20, 170, 90, 30, hWnd, NULL, NULL, NULL);
	hEdit_Datatable_Name = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		110, 170, 200, 25, hWnd, NULL, NULL, NULL);

	//Section 2: Basic Parameter Setting
	CreateWindowW(L"Button", L"基礎參數設定", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 10, 230, 500, 210, hWnd,
		NULL, NULL, NULL);
	CreateWindowW(L"static", L"Setup Time:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		20, 260, 100, 30, hWnd, NULL, NULL, NULL);
	hEdit_Setup_Time = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL | ES_NUMBER,
		130, 260, 100, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"分鐘", WS_VISIBLE | WS_CHILD | SS_LEFT,
		240, 260, 35, 30, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"Process Time:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		20, 300, 100, 30, hWnd, NULL, NULL, NULL);
	hEdit_Process_Time = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL | ES_NUMBER,
		130, 300, 100, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"分鐘", WS_VISIBLE | WS_CHILD | SS_LEFT,
		240, 300, 35, 30, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"Postop Time:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		20, 340, 100, 30, hWnd, NULL, NULL, NULL);
	hEdit_Postop_Time = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL | ES_NUMBER,
		130, 340, 100, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"分鐘", WS_VISIBLE | WS_CHILD | SS_LEFT,
		240, 340, 35, 30, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"搜尋深度:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		20, 380, 100, 30, hWnd, NULL, NULL, NULL);
	hEdit_Search_Depth = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL | ES_NUMBER,
		130, 380, 100, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"機台數", WS_VISIBLE | WS_CHILD | SS_LEFT,
		240, 380, 50, 30, hWnd, NULL, NULL, NULL);

	//Section 3: Goal & End condition
	CreateWindowW(L"Button", L"目標、終止條件", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		530, 10, 500, 210, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"KPI:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		550, 50, 40, 30, hWnd, NULL, NULL, NULL);
	hCombox_KPI = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWN | CBS_HASSTRINGS | CBS_OEMCONVERT | WS_VISIBLE | WS_CHILD | WS_BORDER,
		700, 50, 220, 300, hWnd, NULL, NULL, NULL);	
	CreateWindowW(L"static", L"單位:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		550, 90, 40, 30, hWnd, NULL, NULL, NULL);
	hRBTN_ITER = CreateWindowW(L"Button", L"Iteration", WS_VISIBLE | WS_CHILD | WS_GROUP | BS_AUTORADIOBUTTON,
		680, 85, 70, 30, hWnd, (HMENU)RBTN_SWITCH_ITER, NULL, NULL);
	hRBTN_TIME = CreateWindowW(L"Button", L"分鐘", WS_VISIBLE | WS_CHILD | WS_GROUP | BS_AUTORADIOBUTTON,
		610, 85, 50, 30, hWnd, (HMENU)RBTN_SWITCH_TIME, NULL, NULL);
	CreateWindowW(L"static", L"搜尋選項:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		550, 130, 70, 30, hWnd, NULL, NULL, NULL);
	hEdit_Search_Param = CreateWindowW(L"Edit", L"40", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL | ES_NUMBER,
		700, 130, 100, 25, hWnd, NULL, NULL, NULL);
	hStatic_Search_Param = CreateWindowW(L"static", L"分鐘", WS_VISIBLE | WS_CHILD | SS_LEFT,
		810, 130, 70, 30, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"產出優化解間隔:    每", WS_VISIBLE | WS_CHILD | SS_LEFT,
		550, 170, 145, 30, hWnd, NULL, NULL, NULL);
	hEdit_Time_Gap = CreateWindowW(L"Edit", L"5", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL | ES_NUMBER,
		700, 170, 100, 25, hWnd, NULL, NULL, NULL);
	hStatic_Time_Gap = CreateWindowW(L"static", L"分鐘", WS_VISIBLE | WS_CHILD | SS_LEFT,
		810, 170, 70, 30, hWnd, NULL, NULL, NULL);

		
	SendMessageW(hCombox_KPI, CB_ADDSTRING, 0, (LPARAM)KPIItem[0]);
	SendMessageW(hCombox_KPI, CB_ADDSTRING, 0, (LPARAM)KPIItem[1]);
	SendMessageW(hCombox_KPI, CB_ADDSTRING, 0, (LPARAM)KPIItem[2]);
	SendMessageW(hCombox_KPI, CB_ADDSTRING, 0, (LPARAM)KPIItem[3]);
	SendMessage(hCombox_KPI, CB_SETCURSEL, (WPARAM)0, 0);
	SendMessage(hRBTN_TIME, BM_SETCHECK, BST_CHECKED, NULL);

	//Section 4: Export Setting
	CreateWindowW(L"Button", L"匯出設定", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		530, 230, 500, 160, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"檔名:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		550, 260, 40, 30, hWnd, NULL, NULL, NULL);
	hEdit_FileName = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_AUTOVSCROLL,
		610, 260, 100, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"路徑:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		550, 310, 40, 30, hWnd, NULL, NULL, NULL);
	hEdit_FilePath = CreateWindowW(L"Edit", L"C:\\", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | ES_READONLY | ES_AUTOHSCROLL,
		610, 310, 300, 25, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"Button", L"…", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 910, 310, 20, 25, hWnd,
		(HMENU)SELECT_FOLDER, NULL, NULL);

	CreateWindowW(L"Button", L"Confirm", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 960, 400, 70, 30, hWnd,
		(HMENU)BTN_CONFIRM, NULL, NULL);

	/*CreateWindowW(L"Button", L"SendError", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 550, 400, 100, 30, hWnd,
		(HMENU)PGSB_Update, NULL, NULL);*/
}
void AddProgressControl(HWND hWnd) {
	CreateWindowW(L"static", L"前處理進度:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		10, 20, 100, 30, hWnd, NULL, NULL, NULL);
	PreprocessProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 130, 20, 300, 30,
		hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"優化進度:", WS_VISIBLE | WS_CHILD | SS_LEFT,
		10, 80, 100, 30, hWnd, NULL, NULL, NULL);
	RunTimeProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 130, 80, 300, 30,
		hWnd, NULL, NULL, NULL);

	hBtn_Cancel = CreateWindowW(L"Button", L"Cancel", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 150, 480, 30, hWnd,
		(HMENU)PRGS_CANCEL, NULL, NULL);

	hBtn_Prev = CreateWindowW(L"Button", L"Previous step", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 150, 180, 30, hWnd,
		(HMENU)PRGS_PREV_STEP, NULL, NULL);

	hBtn_Done = CreateWindowW(L"Button", L"Done", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 180, 150, 300, 30, hWnd,
		(HMENU)PRGS_DONE, NULL, NULL);
}
void AddErrorControl(HWND hWnd) {
	CreateWindowW(L"Button", L"錯誤訊息", WS_VISIBLE | WS_CHILD | WS_BORDER | BS_GROUPBOX,
		0, 0, 480, 250, hWnd, NULL, NULL, NULL);
	hEdit_Error_Message = CreateWindowW(L"Edit", L"Origin", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		0, 20, 480, 130, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"Button", L"Confirm", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 150, 480, 30, hWnd,
		(HMENU)ERROR_CONFIRM, NULL, NULL);
}

LRESULT CALLBACK ProgressProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_COMMAND:
		switch (wp) {
		case PRGS_CANCEL:
			//DestroyWindow(hWnd);
			ShowWindow(hBtn_Cancel, SW_HIDE);
			ShowWindow(hBtn_Prev, SW_SHOW);
			ShowWindow(hBtn_Done, SW_SHOW);
			//TODO: set cancel process function and generate a temp file for last iteration
			break;
		case PRGS_PREV_STEP:
			//DestroyWindow(hWnd);
			SendMessage(MainWindow, WM_COMMAND, OPEN_ERROR_WINDOW, 0);
			SendMessage(Error_Window, WM_COMMAND, SEND_ERROR_MSG, 0);
			
			ShowWindow(hBtn_Cancel, SW_SHOW);
			ShowWindow(hBtn_Prev, SW_HIDE);
			ShowWindow(hBtn_Done, SW_HIDE);
			//TODO: reset mainwindow function or close program
			break;
		case PRGS_DONE:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_CREATE:
		AddProgressControl(hWnd);
		ShowWindow(hBtn_Prev, SW_HIDE);
		ShowWindow(hBtn_Done, SW_HIDE);
		break;
	case WM_DESTROY:
		isProgressWindowOpened = false;
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
}

void WinProgress(HINSTANCE hInst) {
	WNDCLASSW Progress = { 0 };
	HBRUSH WinBkBrush = CreateSolidBrush(RGB(255, 255, 255));
	Progress.hbrBackground = WinBkBrush;
	Progress.hCursor = LoadCursor(NULL, IDC_ARROW);
	Progress.hInstance = hInst;
	Progress.lpszClassName = L"ShowProgress";
	Progress.lpfnWndProc = ProgressProc;
	RegisterClassW(&Progress);
}

void displayProgress(HWND hWnd) {
	CreateWindowW(L"ShowProgress", L"Progress", (WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) | WS_VISIBLE, 0, 0, 500, 220
		, NULL, NULL, NULL, NULL);
}

LRESULT CALLBACK ErrorProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_COMMAND:
		switch (wp) {
		case ERROR_CONFIRM:
			DestroyWindow(hWnd);
			break;
		case SEND_ERROR_MSG:
			std::string buffer = "Error:(\r\n";
			int index = GetWindowTextLength(hEdit_Error_Message);
			SetFocus(hEdit_Error_Message); // set focus
			SendMessageA(hEdit_Error_Message, EM_SETSEL, (WPARAM)index, (LPARAM)index); // set selection - end of text
			SendMessageA(hEdit_Error_Message, EM_REPLACESEL, 0, (LPARAM)buffer.c_str());
			break;
		}
		break;
	case WM_CREATE:
		AddErrorControl(hWnd);
		break;
	case WM_DESTROY:
		isErrorWindowOpened = false;
		DestroyWindow(hWnd);
		break;
	case WM_CTLCOLORSTATIC: {

		HDC hdcStatic = (HDC)wp; // or obtain the static handle in some other way
		SetBkMode((HDC)wp, OPAQUE);
		SetTextColor(hdcStatic, RGB(0, 0, 0)); // text color
		SetBkColor(hdcStatic, RGB(255, 255, 255));

		return (LRESULT)GetStockObject(NULL_BRUSH);
		break;
	}
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
}

void SubWinError(HINSTANCE hInst) {
	WNDCLASSW WinErr = { 0 };
	HBRUSH WinBkBrush = CreateSolidBrush(RGB(255, 255, 255));
	WinErr.hbrBackground = WinBkBrush;
	WinErr.hCursor = LoadCursor(NULL, IDC_ARROW);
	WinErr.hInstance = hInst;
	WinErr.lpszClassName = L"ShowError";
	WinErr.lpfnWndProc = ErrorProc;
	RegisterClassW(&WinErr);
}

void displayError(HWND hWnd) {
	Error_Window = CreateWindowW(L"ShowError", L"ErrorMessage", (WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) | WS_VISIBLE, 0, 0, 500, 220
		, NULL, NULL, NULL, NULL);
}