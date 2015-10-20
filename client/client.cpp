/**
 * chat client
 */
#include <stdio.h>
#include <tchar.h>
#include <stdio.h>
#include <windows.h>

#include "SockClient.h"
#include "resource.h"

static SockClient g_sock;

LRESULT CALLBACK MyDlgProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
    switch( msg )
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch( LOWORD(wp) )
            {
                case IDC_SEND:
				{
					char bufName[ 256 ];
					char bufMessage[ 256 ];
					char buf[ 512 + 10 ];
					memset( buf, 0, sizeof(buf) );
					GetDlgItemTextA( hWnd, IDC_NAME, bufName, sizeof(bufName) );
					GetDlgItemTextA( hWnd, IDC_MESSAGE, bufMessage, sizeof(bufMessage)  );
					SetDlgItemTextA( hWnd, IDC_MESSAGE, "" );
					sprintf( buf, "(%s) %s", bufName, bufMessage );
					g_sock.Send( buf );
                    return TRUE;
				}

                case IDCANCEL:
                    return TRUE;
            }
			return TRUE;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			PostQuitMessage( 0 );
			return TRUE;
    }
    return FALSE;
}

static void _sub()
{
	HWND hDlg = CreateDialog( GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)MyDlgProc);

	for(;;)
	{
		MSG msg;
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE))
		{
			if( msg.message == WM_QUIT)
				break;

			if (hDlg && IsDialogMessage(hDlg , &msg))
			{
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}

		g_sock.Main();
		std::string buf = g_sock.Recv();
		if( buf.size() > 0 )
			fwrite( buf.c_str(), buf.size(), 1, stdout );
	}

	if (hDlg) DestroyWindow(hDlg);
}

int main(int argc, char *argv[])
{
	char *target /*= ADDR*/;

	if (argc != 2)
	{
	 //printf("Usage : %s dest\n", argv[0]);
	 //return 1;
		target = "localhost";
	}
	else
		target = argv[1];


	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
	{
		printf("WSAStartup failed\n");
		return 1;
	}


	if( !g_sock.Start( target ) )
	{
		printf("g_sock.Start failed\n");
		return 1;
	}

	_sub();


	g_sock.Finalize();

	WSACleanup();

	return 0;
}

