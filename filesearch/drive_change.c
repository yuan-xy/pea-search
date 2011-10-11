#include "drive_change.h"
#include <dbt.h>
#include <stdio.h>

const WCHAR winClass[] = L"DirveChangeNotifyWindow";
const WCHAR winTitle[] = L"DirveChangeNotifyWindowTitle";

static HANDLE m_thread=NULL;

static pDriveChangeListener listener;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DEVICECHANGE:
		{
			if ((wParam == DBT_DEVICEARRIVAL) || (wParam == DBT_DEVICEREMOVECOMPLETE))
			{
				DEV_BROADCAST_HDR* header = (DEV_BROADCAST_HDR*)(lParam);
				if (header->dbch_devicetype == DBT_DEVTYP_VOLUME)
				{
					DEV_BROADCAST_VOLUME* devNot = (DEV_BROADCAST_VOLUME*)(lParam);
					BOOL add = 0;
					int i=0,mask=1;
					if (wParam == DBT_DEVICEARRIVAL) add=1;
					for(;i<26;i++){
						if(devNot->dbcv_unitmask&mask){
							listener(i,add);
						}
						mask<<=1;
					}
				}
			}
			if(DBT_DEVICEQUERYREMOVE==wParam){
				//TODO: 截获DBT_DEVICEQUERYREMOVE事件
				printf("before device remove.\n");
			}
		}
		break;
	default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
}

void NotifyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(wcex));

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= winClass;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);
}

DWORD WINAPI DriveChangeMonitorProc(PVOID pParam){
	MSG msg;
	int ret;
	HWND hWnd;
	pDriveChangeListener notify = (pDriveChangeListener) pParam;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	NotifyRegisterClass(hInstance);

	hWnd = CreateWindow(winClass, winTitle, WS_ICONIC, 0, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, SW_HIDE);
	/*
	HDEVNOTIFY hDevNotify = RegisterDeviceNotification( hWnd, 
	&notificationFilter,
	DEVICE_NOTIFY_WINDOW_HANDLE 
	);
	*/

loop:

	WaitMessage();

	// update windows
	while(PeekMessage(&msg,NULL,0,0,0)) 
	{
		ret = (int)GetMessage(&msg,0,0,0);
		if (ret == -1) goto exit;
		if (!ret) goto exit;

		// let windows handle it.
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}			

	goto loop;

exit:

	return 0;
}




BOOL StartDriveChangeMonitorThread( pDriveChangeListener notify ){
	if(m_thread!=NULL) return 0;
	m_thread=CreateThread(NULL,0,DriveChangeMonitorProc,notify,0,0);
	if(m_thread==NULL) return 0;
	listener=notify;
	return 1;
}

BOOL StopDriveChangeMonitorThread(void){
	return TerminateThread(m_thread,0);
}
