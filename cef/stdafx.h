// stdafx.h: 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 项目特定的包含文件
//

#pragma once

#include "targetver.h"

// C runtime header
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// base header
#include "base/base.h"

// duilib
#include "duilib/UIlib.h"

// ui components
#include "ui_components/ui_components.h"
#include "ui_components/ui_cef_control.h"

#include "tool.h"
#include "util.h"

const std::string versionNum = "16";

#define WM_SHOWTASK						WM_USER+101 
#define WM_ONCLOSE						WM_USER+102
#define WM_ONSHOW						WM_USER+103
#define WM_ONMIN						WM_USER+104
#define WM_ONSHOWNOTICE					WM_USER+105
#define WM_ONHIDENOTICE					WM_USER+106
#define WM_WSRECONNECT					WM_USER+107
#define WM_CLEARCACHE					WM_USER+108
#define WM_SETTOOLBTNURL				WM_USER+109
#define WM_CEFWINDOWCLOSE				WM_USER+110
#define WM_CEFWINDOWOPEN				WM_USER+111
#define WM_OPENEXISTCEFWINDOW			WM_USER+112
#define WM_RESULTWINDOWOPEN				WM_USER+113
#define WM_RESULTWINDOWCLOSE			WM_USER+114
#define WM_OPENCEFWINDOWFROMRESULT		WM_USER+115
#define WM_MODIFYPASSWORD				WM_USER+116
#define WM_STARTRECORDAUDIO				WM_USER+117
#define WM_ENDRECORDAUDIO				WM_USER+118
#define WM_SENDRWMESSAGE				WM_USER+119
#define WM_USERONLINESTATE				WM_USER+120
#define WM_OPENRWCLIENT					WM_USER+121
#define WM_CLOSEQAWINDOW				WM_USER+122
#define WM_SENDQAMESSAGE				WM_USER+123
#define WM_OPENQAWINDOW					WM_USER+124
#define WM_SHOWTOASTWINDOW				WM_USER+125
#define WM_USERQUIT						WM_USER+126
#define WM_ONCLOSENOTICE				WM_USER+127