//Client Terminal v1.01
//Written by Andrew Sturges
//April 2015

#ifndef _CLIENT_TERMINAL_H
#define _CLIENT_TERMINAL_H

CMainWindow clientWin;
CEditBox dispBox;
CLabel statusBox;
CLabel statusLabel;
CButton connectBtn;
CButton disconnectBtn;
CEditBox hostEdit;
CLabel hostLabel;
CEditBox portEdit;
CLabel portLabel;
CEditBox dataEdit;
CLabel dataLabel;
CButton sendBtn;
CButton clearBtn;
CSocket* clientSocketPtr = NULL;

COLORREF dispBkColor;
COLORREF dispTextColor;
HBRUSH dispBkColorBrush;

bool ClientSetup(void);
void ClientCleanup(void);
int WINAPI WinMain (HINSTANCE, HINSTANCE, LPSTR, int);

LRESULT ClientWin_OnClose(CWindow*, const CWinEvent&);
LRESULT ClientWin_OnSize(CWindow*, const CWinEvent&);
LRESULT ClientWin_OnCtlColorStatic(CWindow*, const CWinEvent&);
LRESULT ConnectBtn_OnClick(CWindow*, const CWinEvent&);
LRESULT DisconnectBtn_OnClick(CWindow*, const CWinEvent&);
LRESULT DataEdit_OnKeyPress(CWindow*, const CWinEvent&);
LRESULT SendBtn_OnClick(CWindow*, const CWinEvent&);
LRESULT ClearBtn_OnClick(CWindow*, const CWinEvent&);

void Socket_OnConnect(CSocket*, const CSocketEvent&);
void Socket_OnDisconnect(CSocket*, const CSocketEvent&);
void Socket_OnDataArrival(CSocket*, const CSocketEvent&);
void Socket_OnError(CSocket*, const CSocketEvent&);

void UpdateUI(void);
void SendData(void);

#endif
