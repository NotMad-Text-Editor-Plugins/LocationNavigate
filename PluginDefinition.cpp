//this file is part of notepad++
//Copyright (C) 2011 AustinYoung<pattazl@gmail.com>
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

#include "PluginDefinition.h"
#include "menuCmdID.h"

//
// put the headers you need here
//
#include <stdlib.h>
#include <time.h>
#include <shlwapi.h>
#include "LNhistoryDlg.h"


LocationNavigateDlg _LNhistory;

#ifdef UNICODE 
	#define generic_itoa _itow
#else
	#define generic_itoa itoa
#endif

FuncItem funcItem[nbFunc];
bool menuState[nbFunc];
HWND hToolbar;
int IconID[nbFunc];
//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
HANDLE				g_hModule;
toolbarIcons		g_TBPrevious{0,0,0x666,0,IDI_ICON_PREV,IDI_ICON_PREV_ACT,IDI_ICON_PREV_OFF,IDB_BITMAP1};
toolbarIcons		g_TBNext{0,0,0x666,0,IDI_ICON_NEXT,IDI_ICON_NEXT_ACT,IDI_ICON_NEXT_OFF,IDB_BITMAP2};
toolbarIcons		g_TBPreviousChg{0,0,0x666,0,IDI_ICON_PREV1,IDI_ICON_PREV1_ACT,IDI_ICON_PREV1_OFF,IDB_BITMAP3};
toolbarIcons		g_TBNextChg{0,0,0x666,0,IDI_ICON_NEXT1,IDI_ICON_NEXT1_ACT,IDI_ICON_NEXT1_OFF,IDB_BITMAP4};

TCHAR iniFilePath[MAX_PATH];
//bool SaveRecording = false;
//#define DOCKABLE_LNH_INDEX 4

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
	// Initialize dialog
	_LNhistory.init((HINSTANCE)hModule, NULL);
	g_hModule = hModule;

	//InitializeCriticalSection(&criCounter); 
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	//DeleteCriticalSection(&criCounter);
	TCHAR str[500]={0};	
	wsprintf(str,TEXT("%d"),MaxOffset);
	::WritePrivateProfileString(sectionName, strMaxOffset,str , iniFilePath);
	wsprintf(str,TEXT("%d"),MaxList);
	::WritePrivateProfileString(sectionName, strMaxList, str, iniFilePath);
	wsprintf(str,TEXT("%d"),AutoClean?1:0);
	::WritePrivateProfileString(sectionName, strAutoClean, str, iniFilePath);
	wsprintf(str,TEXT("%d"),AlwaysRecord?1:0);
	::WritePrivateProfileString(sectionName, strAlwaysRecord, str, iniFilePath);
	wsprintf(str,TEXT("%d"),SaveRecord?1:0);
	::WritePrivateProfileString(sectionName, strSaveRecord, str, iniFilePath);
	wsprintf(str,TEXT("%d"),InCurr?1:0);
	::WritePrivateProfileString(sectionName, strInCurr, str, iniFilePath);
	wsprintf(str,TEXT("%d"),skipClosed?1:0);
	::WritePrivateProfileString(sectionName, strSkipClosed, str, iniFilePath);
	wsprintf(str,TEXT("%d"),bAutoRecord?1:0);
	::WritePrivateProfileString(sectionName, strAutoRecord, str, iniFilePath);
	
	wsprintf(str,TEXT("%d"),NeedMark?1:0);
	::WritePrivateProfileString(sectionName, strNeedMark, str, iniFilePath);
	wsprintf(str,TEXT("%d"),ByBookMark);
	::WritePrivateProfileString(sectionName, strByBookMark, str, iniFilePath);
	wsprintf(str,TEXT("%d"),MarkColor);
	::WritePrivateProfileString(sectionName, strMarkColor, str, iniFilePath);
	wsprintf(str,TEXT("%d"),SaveColor);
	::WritePrivateProfileString(sectionName, strSaveColor, str, iniFilePath);

}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	//
	// Firstly we get the parameters from your plugin config file (if any)
	//

	// get path of plugin configuration
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniFilePath);

	// if config path doesn't exist, we create it
	if (PathFileExists(iniFilePath) == FALSE)
	{
		::CreateDirectory(iniFilePath, NULL);
	}

	// make your plugin config file full file path name
	PathAppend(iniFilePath, configFileName);

	// get the parameter value from plugin config
	MaxOffset = ::GetPrivateProfileInt(sectionName, strMaxOffset, 100, iniFilePath);
	MaxList = ::GetPrivateProfileInt(sectionName, strMaxList, 50, iniFilePath) ;
	AutoClean = (::GetPrivateProfileInt(sectionName, strAutoClean, 0, iniFilePath)== 1) ;
	AlwaysRecord = (::GetPrivateProfileInt(sectionName, strAlwaysRecord, 0, iniFilePath)== 1) ;
	SaveRecord   = (::GetPrivateProfileInt(sectionName, strSaveRecord, 0, iniFilePath)== 1) ;
	InCurr   = (::GetPrivateProfileInt(sectionName, strInCurr, 0, iniFilePath)== 1) ;
	skipClosed   = (::GetPrivateProfileInt(sectionName, strSkipClosed, 0, iniFilePath)== 1) ;
	bAutoRecord   = (::GetPrivateProfileInt(sectionName, strAutoRecord, 1, iniFilePath)== 1) ;
	NeedMark = (::GetPrivateProfileInt(sectionName, strNeedMark, 1, iniFilePath)== 1) ;
	ByBookMark = (MarkType)::GetPrivateProfileInt(sectionName, strByBookMark, 0, iniFilePath);
	MarkColor = ::GetPrivateProfileInt(sectionName, strMarkColor, DefaultColor, iniFilePath);
	SaveColor = ::GetPrivateProfileInt(sectionName, strSaveColor, DefaultSaveColor, iniFilePath);
    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
	#define VK_OEM_MINUS      0xBD
	ShortcutKey *PreviousKey = new ShortcutKey;
	PreviousKey->_isAlt = false;
	PreviousKey->_isCtrl = true;
	PreviousKey->_isShift = false;
	PreviousKey->_key = VK_OEM_MINUS; 

	ShortcutKey *NextKey = new ShortcutKey;
	NextKey->_isAlt = false;
	NextKey->_isCtrl = true;
	NextKey->_isShift = true;
	NextKey->_key = VK_OEM_MINUS; 

	ShortcutKey *PreChgKey = new ShortcutKey;
	PreChgKey->_isAlt = true;
	PreChgKey->_isCtrl = true;
	PreChgKey->_isShift = false;
	PreChgKey->_key = 0x5A ; //VK_Z

	ShortcutKey *NextChgKey = new ShortcutKey;
	NextChgKey->_isAlt = true;
	NextChgKey->_isCtrl = true;
	NextChgKey->_isShift = false;
	NextChgKey->_key = 0x59;  //VK_Y


	ShortcutKey *optionsKey = new ShortcutKey;
	optionsKey->_isAlt = true;
	optionsKey->_isCtrl = true;
	optionsKey->_isShift = true;
	optionsKey->_key = VK_OEM_MINUS;

	ShortcutKey *AutoKey = new ShortcutKey;
	AutoKey->_isAlt = true;
	AutoKey->_isCtrl = true;
	AutoKey->_isShift = false;
	AutoKey->_key = VK_F9;

	ShortcutKey *ManualKey = new ShortcutKey;
	ManualKey->_isAlt = false;
	ManualKey->_isCtrl = false;
	ManualKey->_isShift = false;
	ManualKey->_key = VK_F9;

	ShortcutKey *ClearRecordsKey = new ShortcutKey;
	ClearRecordsKey->_isAlt = true;
	ClearRecordsKey->_isCtrl = true;
	ClearRecordsKey->_isShift = true;
	ClearRecordsKey->_key = VK_F9;

	ShortcutKey *incurrKey = new ShortcutKey;
	incurrKey->_isAlt = true;
	incurrKey->_isCtrl = false;
	incurrKey->_isShift = false;
	incurrKey->_key = VK_OEM_MINUS;

	ShortcutKey *markKey = new ShortcutKey;
	markKey->_isAlt = true;
	markKey->_isCtrl = true;
	markKey->_isShift = false;
	markKey->_key = 0x4D; // VK_Y

    setCommand(menuPrevious, TEXT("Previous Location"), PreviousLocation, PreviousKey, false);
    setCommand(menuNext, TEXT("Next Location"), NextLocation, NextKey, false);
	setCommand(menuChgPrevious, TEXT("Previous Changed"), PreviousChangedLocation, PreChgKey, false);
	setCommand(menuChgNext, TEXT("Next Changed"), NextChangedLocation, NextChgKey, false);
	setCommand(menuOption, TEXT("Show List and Option"), LocationNavigateHistoryDlg, optionsKey, false);
	
	setCommand(menuSeparator0, TEXT("-SEPARATOR-"),NULL, NULL, false);
	setCommand(menuAutoRecord, TEXT("Auto Record"), AutoRecord, AutoKey, false);
	setCommand(menuManualRecord, TEXT("Record"), ManualRecord, ManualKey, false);
	setCommand(menuClearRecords, TEXT("Clear All Records"), ClearAllRecords, ClearRecordsKey, false);
	

	setCommand(menuInCurr, TEXT("In Current File"), NavigateInCurr, incurrKey, false);
	setCommand(menuNeedMark, TEXT("Mark Changed Line"), MarkChange, markKey, false);


	setCommand(menuSeparator1, TEXT("-SEPARATOR-"),NULL, NULL, false);

	setCommand(menuSkipClosed, TEXT("Skip closed File"), SkipClosed, NULL, false);
	setCommand(menuClearOnClose, TEXT("Auto clear when close"), MarkChange, markKey, false);


	setCommand(menuSeparator2, TEXT("-SEPARATOR-"),NULL, NULL, false);
	setCommand(menuCheckUpdate, TEXT("Check for update"), checkUpdate, NULL, false);
	setCommand(menuAbout, TEXT("About Location Navigate"), ShowAbout, NULL, false);

	//int items[]{menuAutoRecord};
	//for(auto idx:items) funcItem[idx].flag=1;

}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[menuPrevious]._pShKey;
	delete funcItem[menuNext]._pShKey;
	delete funcItem[menuChgPrevious]._pShKey;
	delete funcItem[menuChgNext]._pShKey;
	delete funcItem[menuOption]._pShKey;
	delete funcItem[menuInCurr]._pShKey;
	delete funcItem[menuNeedMark]._pShKey;

	if (SaveRecord)
	{
		//SaveRecording = true;
		TCHAR iniContent[RecordConentMax]={0};
		// 如果要保存记录，那么将 locationlist 保存
		for (int i=0;i<LocationSave.size();i++)
		{
			TCHAR tmp[300]={0};
			wsprintf(tmp,TEXT("%d<%d>%s|"),LocationSave[i].changed?1:0,LocationSave[i].position,LocationSave[i].FilePath);
			lstrcat(iniContent,tmp);
		}
		::WritePrivateProfileString(sectionName, strRecordContent, iniContent, iniFilePath);
	}else
	{
		::WritePrivateProfileString(sectionName, strRecordContent, NULL, iniFilePath);
	}
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void PreviousLocation()
{
	if (LocationPos > 0)
	{
		if ( InCurr )
		{
			int pos = LocationPos;
			while ( --pos >= 0 )
			{
				if ( lstrcmp(LocationList[pos].FilePath,currFile) == 0 )
				{
					// 当前位置的文件名和当前文件一样,找位置不一样的
					if( LocationList[pos].position != LocationList[LocationPos].position)						{
						LocationPos = pos;
						break;
					}
				}
			}
			SetPosByIndex(0);
		} else {
			SetPosByIndex(-1);
		}
		_LNhistory.refreshDlg();
	}
}
void NextLocation()
{
	if (LocationPos < LocationList.size()-1)
	{
		// 需要查找当前文件的下一个
		if ( InCurr )
		{
			int pos = LocationPos;
			while ( ++pos <= LocationList.size()-1)
			{
				if ( lstrcmp(LocationList[pos].FilePath,currFile) == 0 )
				{
					// 当前位置的文件名和当前文件一样,找位置不一样的
					if( LocationList[pos].position != LocationList[LocationPos].position)						{
						LocationPos = pos;
						break;
					};
				}
			}
			SetPosByIndex(0);
		} else {
			SetPosByIndex(1);
		}
		_LNhistory.refreshDlg();
	}
}
void PreviousChangedLocation()
{
	int tmpPos = LocationPos;
	int len =  LocationList.size()-1;
	if (tmpPos > len )
	{
		tmpPos = len;
	}
	while ( tmpPos >0)
	{
		// 需要往前走一个
		int pos = --tmpPos;
		if ( LocationList[pos].changed )
		{
			// 需要判断文件是否一致
			bool samefile = true;
			if ( InCurr )
			{
				samefile = (lstrcmp(LocationList[pos].FilePath,currFile) == 0);
				if( samefile )
				{
					//if ( lstrcmp(LocationList[LocationPos].FilePath,currFile)==0 )
					// 当前位置的文件名和当前文件一样,找位置不一样的
					samefile = ( LocationList[pos].position != LocationList[LocationPos].position);
				}
			}
			if ( samefile )
			{
				//LocationPos = pos;
				if(!SetPosByIndex(pos-LocationPos, false)) {
					continue;
				}
				_LNhistory.refreshDlg();
				break;
			}
		}
	}
}
void NextChangedLocation()
{
	int tmpPos = LocationPos;
	int len =  LocationList.size()-1;
	if (tmpPos > len )
	{
		tmpPos = len;
	}
	while ( tmpPos < len)
	{
		// 需要往后走一个
		int pos = ++tmpPos;
		if ( LocationList[pos].changed )
		{
			bool samefile = true;
			if ( InCurr )
			{
				samefile = (lstrcmp(LocationList[pos].FilePath,currFile) == 0);
				if( samefile )
				{
					// 当前位置的文件名和当前文件一样,找位置不一样的
					samefile = ( LocationList[pos].position != LocationList[LocationPos].position);
				}
			}
			if ( samefile )
			{
				//LocationPos = pos;
				if(!SetPosByIndex(pos-LocationPos, false)) {
					continue;
				}
				_LNhistory.refreshDlg();
				break;
			}
		}
	}
}
void AutoRecord()
{
	//
	bAutoRecord = !bAutoRecord;
	// 刷新菜单
	::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[menuAutoRecord]._cmdID, MF_BYCOMMAND | (bAutoRecord?MF_CHECKED:MF_UNCHECKED));
	::EnableMenuItem(::GetMenu(nppData._nppHandle),
		funcItem[menuManualRecord]._cmdID,MF_BYCOMMAND|(bAutoRecord?MF_GRAYED:MF_ENABLED ));

}
void ManualRecord()
{
	//
	AddList(true);
}
void ClearAllRecords()
{
	ClearLocationList();
	_LNhistory.refreshDlg();
}
void SkipClosed()
{
	skipClosed = !skipClosed;
	::CheckMenuItem(::GetMenu(nppData._nppHandle),
		funcItem[menuSkipClosed]._cmdID, MF_BYCOMMAND | (skipClosed?MF_CHECKED:MF_UNCHECKED));
}

void NavigateInCurr()
{
	InCurr = !InCurr;
	// 刷新菜单
	::CheckMenuItem(::GetMenu(nppData._nppHandle),
		funcItem[menuInCurr]._cmdID, MF_BYCOMMAND | (InCurr?MF_CHECKED:MF_UNCHECKED));
	// 刷新配置界面
	_LNhistory.refreshDlg();
	if ( _LNhistory.isCreated() )
	{
		::SendMessage( _LNhistory._hInCurr, BM_SETCHECK ,(LPARAM)(InCurr?1:0),0);
	}
}
void MarkChange()
{
	NeedMark = !NeedMark;
	// 刷新菜单
	::CheckMenuItem(::GetMenu(nppData._nppHandle), 
		funcItem[menuNeedMark]._cmdID, MF_BYCOMMAND | (NeedMark?MF_CHECKED:MF_UNCHECKED));
	// 刷新配置界面
	if ( _LNhistory.isCreated() )
	{
		::SendMessage( _LNhistory._hMark, BM_SETCHECK ,(LPARAM)(NeedMark?1:0),0);
	}
}
//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
	if (index >= nbFunc)
		return false;

	if (!pFunc)
		return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}
void LocationNavigateHistoryDlg()
{
	_LNhistory.setParent( nppData._nppHandle );
	tTbData data = {0};

	if ( !_LNhistory.isCreated() )
	{
		_LNhistory.create( &data );
		// define the default docking behaviour
		data.uMask          = DWS_DF_CONT_RIGHT | DWS_ICONTAB;
		data.pszModuleName = _LNhistory.getPluginFileName();
		// the dlgDlg should be the index of funcItem where the current function pointer is
		data.dlgID = menuOption;
		data.hIconTab       = ( HICON )::LoadImage( _LNhistory.getHinst(),
			MAKEINTRESOURCE( IDI_ICON1 ), IMAGE_ICON, 0, 0,
			LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT );
		::SendMessage( nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0,
			( LPARAM )&data );
	}

	UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
		funcItem[menuOption]._cmdID, MF_BYCOMMAND );

	if ( state & MF_CHECKED )
		_LNhistory.display( false );
	else
		_LNhistory.display();

	::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
		funcItem[menuOption]._cmdID, !( state & MF_CHECKED ) );
}

void ShowAbout()
{
	::MessageBox(nppData._nppHandle, TEXT(" You can use Ctrl+ - jump to previous cursor position \n You can use Ctrl+Shift+ - jump to next cursor position \n You can use Ctrl+Alt+ Z jump to previous changed position \n You can use Ctrl+Alt+ Y jump to next changed position \n 'Auto clear when close'- Will remove the file's record when file closed.\n 'Always record'- Will always record the position even after you jumped.\n 'Save record when App exit'- Record data when application exit and it will be loaded in next run \n 'In Curr'- If checked, navigate only in current file\n 'Mark'- If checked, modified line will be marked by bookmark or color\n 'Mark Color/Save Color'- Available if not select mark with bookmark, you could mark with different symbol.  \n\n Version: 0.4.7.7   Author: Austin Young<pattazl@gmail.com>"), TEXT("About Location Navigate"), MB_OK);
}
void checkUpdate()
{
	ShellExecute(NULL, TEXT("open"),TEXT("http://sourceforge.net/projects/locationnav/files"), NULL, NULL, SW_SHOW);
}