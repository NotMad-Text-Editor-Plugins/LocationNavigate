//this file is part of notepad++
//Copyright (C)2012 Austin Young ( pattazl@gmail.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "SelfCtrl.h"
static BYTE XORMask[128] =
{
  0xff, 0xff, 0xff, 0xff,
  0xf9, 0xff, 0xff, 0xff,
  0xf0, 0xff, 0xff, 0xff,
  0xf0, 0xff, 0xff, 0xff,
  0xf0, 0xff, 0xff, 0xff,
  0xf0, 0xff, 0xff, 0xff,
  0xf0, 0x24, 0xff, 0xff,
  0xf0, 0x00, 0x7f, 0xff,
  0xc0, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x7f, 0xff,
  0xc0, 0x00, 0x7f, 0xff,
  0xe0, 0x00, 0x7f, 0xff,
  0xf0, 0x00, 0xff, 0xff,
  0xf0, 0x00, 0xff, 0xff,
  0xf0, 0x00, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
};

/* AND mask for hand cursor */
/* Generated by HexEdit */
static BYTE ANDMask[128] =
{
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00,
  0x06, 0xdb, 0x00, 0x00,
  0x06, 0xdb, 0x00, 0x00,
  0x36, 0xdb, 0x00, 0x00,
  0x36, 0xdb, 0x00, 0x00,
  0x37, 0xff, 0x00, 0x00,
  0x3f, 0xff, 0x00, 0x00,
  0x3f, 0xff, 0x00, 0x00,
  0x1f, 0xff, 0x00, 0x00,
  0x0f, 0xff, 0x00, 0x00,
  0x07, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
};


void SelfCtrl::create(HWND itemHandle,long MarkIndex, long color)
{
	COLORREF bkColor;
	bkColor = color;
	SelfID = MarkIndex;
	// turn on notify style
    ::SetWindowLong(itemHandle, GWL_STYLE, ::GetWindowLong(itemHandle, GWL_STYLE) | SS_NOTIFY);

	// set the URL text (not the display text)

	// set the hyperlink colour
    _bkColor = bkColor;

	// set the visited colour

	// subclass the static control
    _oldproc = (WNDPROC)::SetWindowLong(itemHandle, GWLP_WNDPROC, (LONG)SelfCtrlProc);

	// associate the URL structure with the static control
    ::SetWindowLong(itemHandle, GWLP_USERDATA, (LONG)this);
}
void SelfCtrl::SetTextBkColor(HWND hwnd)
{
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	DWORD dwDTStyle = DT_SINGLELINE;

	//Test if centered horizontally or vertically
	if(dwStyle & SS_CENTER)	     dwDTStyle |= DT_CENTER;
	if(dwStyle & SS_RIGHT)		 dwDTStyle |= DT_RIGHT;
	if(dwStyle & SS_CENTERIMAGE) dwDTStyle |= DT_VCENTER;

	RECT		rect;
	::GetClientRect(hwnd, &rect);

	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(hwnd, &ps);

	// ::SetTextColor(hdc, _bkColor);
	::SetBkColor  (hdc, _bkColor);//::GetSysColor(COLOR_3DFACE)

	// Get the default GUI font
	HFONT hf = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	::SelectObject(hdc, hf);

	// Draw the text!
	TCHAR szWinText[MAX_PATH];
	::GetWindowText(hwnd, szWinText, sizeof szWinText);
	::DrawText(hdc, szWinText, -1, &rect, dwDTStyle);

	//::SelectObject(hdc, hOld);

	::EndPaint(hwnd, &ps);
}
LRESULT SelfCtrl::runProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
	    // Free up the structure we allocated
	    case WM_NCDESTROY:
		    //HeapFree(GetProcessHeap(), 0, url);
		    break;

	    // Paint the static control using our custom
	    // colours, and with an underline text style
	    case WM_PAINT:
        {
		   SetTextBkColor(hwnd);
		    return 0;
        }
	    // Provide a hand cursor when the mouse moves over us
	    //case WM_SETCURSOR:
        case WM_MOUSEMOVE:
        {
            if (_hCursor == 0)
                _hCursor = ::CreateCursor(::GetModuleHandle(0), 5, 2, 32, 32, XORMask, ANDMask);

            SetCursor(_hCursor);
            return TRUE;
        }

	    case WM_LBUTTONDOWN:
		    _clicking = true;
		    break;

	    case WM_LBUTTONUP:
		    if(_clicking)
		    {
			    _clicking = false;
                ::InvalidateRect(hwnd, 0, 0);
                ::UpdateWindow(hwnd);

			    // Open a color
				CHOOSECOLOR   cc;                                   //   common   dialog   box   structure  
				static   COLORREF   acrCustClr[16];   //   array   of   custom   colors  
				HBRUSH   hbrush;                                     //   brush   handle
				static   DWORD   rgbCurrent;                 //   initial   color   selection

				//   Initialize   CHOOSECOLOR  
				ZeroMemory(&cc,   sizeof(cc));
				cc.lStructSize   =   sizeof(cc);
				cc.hwndOwner   = hwnd ;		// NULL; 
				cc.lpCustColors   =   (LPDWORD)   acrCustClr;
				cc.rgbResult   =   _bkColor;
				cc.Flags   =   CC_FULLOPEN   |   CC_RGBINIT;

				if   (ChooseColor(&cc)==TRUE)   {
					_bkColor =cc.rgbResult;
					::InvalidateRect(hwnd, 0, 0);
					::UpdateWindow(hwnd);
					if (SelfID == _MARK_INDEX)
					{
						MarkColor = _bkColor;
						::SendMessage(curScintilla,SCI_MARKERSETBACK,_MARK_INDEX,_bkColor);// Ĭ����ɫ
					}else if (SelfID == _SAVE_INDEX)
					{
						SaveColor = _bkColor;
						::SendMessage(curScintilla,SCI_MARKERSETBACK,_SAVE_INDEX,_bkColor);// ������ɫ
					}
					
				}
		    }
		    break;

	    // A standard static control returns HTTRANSPARENT here, which
	    // prevents us from receiving any mouse messages. So, return
	    // HTCLIENT instead.
	    case WM_NCHITTEST:
		    return HTCLIENT;
    }
    return ::CallWindowProc(_oldproc, hwnd, Message, wParam, lParam);
}
