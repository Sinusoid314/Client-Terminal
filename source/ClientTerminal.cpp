#include <cstdio>
#include "..\..\[Libraries]\String Extension\stringExt.h"
#include "..\..\[Libraries]\WinSocket\Socket.h"
#include "..\..\[Libraries]\WinGUI\WinGUI.h"
#include "ClientTerminal.h"

using namespace std;


bool ClientSetup(void)
//Set up program resources
{
    WNDCLASSEX wndClassInfo;
    HINSTANCE hThisInstance = (HINSTANCE) GetModuleHandle(NULL);
    int wndWidth = int(GetSystemMetrics(SM_CXSCREEN) / 1.8);
    int wndHeight = int(GetSystemMetrics(SM_CYSCREEN) / 1.5);
    int wndLeft = (GetSystemMetrics(SM_CXSCREEN) - wndWidth) / 2;
    int wndTop = (GetSystemMetrics(SM_CYSCREEN) - wndHeight) / 2;
    
    //Setup socket support
    if(!SocketSetup())
        return false;
    
    //Setup GUI support
    if(!WinGUISetup())
        return false;
    
    //Create socket
    clientSocketPtr = new CSocket;
    
    //Set socket events
    clientSocketPtr->AddEvent(FD_CONNECT, Socket_OnConnect);
    clientSocketPtr->AddEvent(FD_CLOSE, Socket_OnDisconnect);
    clientSocketPtr->AddEvent(FD_READ, Socket_OnDataArrival);
    clientSocketPtr->AddEvent(FD_ERROR, Socket_OnError);
    
    //Create window and controls
    clientWin.Create("Client Terminal", wndLeft, wndTop, wndWidth, wndHeight, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN);
    hostLabel.Create(&clientWin, "Host:", 15, 15, 35, 20, WS_VISIBLE | WS_CHILD);
    hostEdit.Create(&clientWin, "", 50, 15, 140, 20, WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL|WS_TABSTOP, WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT);
    portLabel.Create(&clientWin, "Port:", 200, 15, 35, 20, WS_VISIBLE|WS_CHILD);
    portEdit.Create(&clientWin, "", 235, 15, 70, 20, WS_VISIBLE|WS_CHILD|WS_TABSTOP, WS_EX_CONTROLPARENT);
    connectBtn.Create(&clientWin, "Connect", 325, 10, 85, 30, WS_VISIBLE|WS_CHILD);
    disconnectBtn.Create(&clientWin, "Disconnect", 420, 10, 85, 30, WS_VISIBLE|WS_CHILD|WS_DISABLED);
    statusLabel.Create(&clientWin, "Status:", 525, 15, 50, 20, WS_VISIBLE|WS_CHILD);
    statusBox.Create(&clientWin, "Disconnected", 575, 15, 130, 20, WS_VISIBLE|WS_CHILD, WS_EX_STATICEDGE);
    dispBox.Create(&clientWin, "", 0, 50, 0, 0, 
                                    WS_VISIBLE|WS_CHILD|WS_BORDER|
                                    WS_VSCROLL|WS_HSCROLL|ES_READONLY|
                                    ES_MULTILINE|ES_WANTRETURN|
                                    ES_AUTOHSCROLL|ES_AUTOVSCROLL);
    dataLabel.Create(&clientWin, "Data:", 0, 0, 35, 20, WS_VISIBLE|WS_CHILD);
    dataEdit.Create(&clientWin, "", 0, 0, 300, 20, WS_VISIBLE|WS_CHILD|WS_DISABLED|WS_TABSTOP, WS_EX_CONTROLPARENT);
    sendBtn.Create(&clientWin, "Send", 0, 0, 85, 30, WS_VISIBLE|WS_CHILD|WS_DISABLED);
    
    //Set window events
    clientWin.AddEvent(WM_CLOSE, ClientWin_OnClose, WINEVENT_MESSAGE);
    clientWin.AddEvent(WM_SIZE, ClientWin_OnSize, WINEVENT_MESSAGE);
    clientWin.AddEvent(WM_CTLCOLORSTATIC, ClientWin_OnCtlColorStatic, WINEVENT_MESSAGE);
    connectBtn.AddEvent(BN_CLICKED, ConnectBtn_OnClick, WINEVENT_COMMAND);
    disconnectBtn.AddEvent(BN_CLICKED, DisconnectBtn_OnClick, WINEVENT_COMMAND);
    dataEdit.AddEvent(WM_CHAR, DataEdit_OnKeyPress, WINEVENT_MESSAGE);
    sendBtn.AddEvent(BN_CLICKED, SendBtn_OnClick, WINEVENT_COMMAND);
    
    //Set display colors
    dispBkColor = RGB(0,0,90);
    dispTextColor = RGB(255,255,255);
    dispBkColorBrush = CreateSolidBrush(dispBkColor);
    
    return true;
}

void ClientCleanup(void)
//Clean up program resources
{
    //Close the main window
    clientWin.Destroy();
    
    //Close socket
    delete clientSocketPtr;
    
    //Cleanup GUI support
    WinGUICleanup();
    
    //Cleanup socket support
    SocketCleanup();
    
    //Cleanup display color brush
    DeleteObject(dispBkColorBrush);
}

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//Program starting point
{
    WPARAM retVal;
    
    if(!ClientSetup())
        return 0;
    
    //Display the main window
    clientWin.Show();
    
    //Run message loop
    retVal = InputWinEvents();
    
    ClientCleanup();
    
    return retVal;
}

LRESULT ClientWin_OnClose(CWindow* winPtr, const CWinEvent& eventObj)
//End the program
{
    PostQuitMessage(0);
    return 0;
}

LRESULT ClientWin_OnSize(CWindow* winPtr, const CWinEvent& eventObj)
//Resize controls with the main window
{
    int newWidth = LOWORD(eventObj.lParam);
    int newHeight = HIWORD(eventObj.lParam);
    
    dispBox.SetSize(newWidth, newHeight - 100);
    dataLabel.SetPos(15, newHeight - 35);
    dataEdit.SetPos(50, newHeight - 35);
    sendBtn.SetPos(370, newHeight - 40);
    
    return 0;
}

LRESULT ClientWin_OnCtlColorStatic(CWindow* winPtr, const CWinEvent& eventObj)
//
{
    HDC hdc = (HDC) eventObj.wParam;
    HWND hwnd = (HWND) eventObj.lParam;
    
    if(hwnd == dispBox.winHandle)
    {
    	SetBkColor(hdc, dispBkColor);
	    SetTextColor(hdc, dispTextColor);
        return (LRESULT)dispBkColorBrush;
    }
    else
    {
    	return DefWindowProc(clientWin.winHandle, WM_CTLCOLORSTATIC, eventObj.wParam, eventObj.lParam);
    }
}

LRESULT ConnectBtn_OnClick(CWindow* winPtr, const CWinEvent& eventObj)
//Connect to the given host
{
	string hostStr;
    int portNum;
    string portStr;
    
    //Get the host address
    hostStr = hostEdit.GetText();
    
    //Get the port number
    portStr = portEdit.GetText();
    portNum = StrToNum(portStr);
    
    //Connect to remote host
    if(!(clientSocketPtr->Connect(hostStr.c_str(), portNum)))
    {
	    dispBox.AppendText("Failed to connect. \r\n\r\n");
        return 0;
	}
    
    UpdateUI();
    return 0;
}

LRESULT DisconnectBtn_OnClick(CWindow* winPtr, const CWinEvent& eventObj)
//End the current connection
{
    
    if(!(clientSocketPtr->Disconnect()))
    {
	    dispBox.AppendText("Failed to disconnect. \r\n\r\n");
        return 0;
	}
    
    UpdateUI();
    return 0;
}

LRESULT DataEdit_OnKeyPress(CWindow* winPtr, const CWinEvent& eventObj)
//
{
    if(eventObj.wParam == 13)
    {
        SendData();
    }
    
    return 0;
}

LRESULT SendBtn_OnClick(CWindow* winPtr, const CWinEvent& eventObj)
//
{
    SendData();
}

void Socket_OnConnect(CSocket* socketPtr, const CSocketEvent& eventObj)
//Runs when a connection is established
{
    UpdateUI();
}

void Socket_OnDisconnect(CSocket* socketPtr, const CSocketEvent& eventObj)
//Runs when the remote socket closes the connection
{   
    UpdateUI();
}

void Socket_OnDataArrival(CSocket* socketPtr, const CSocketEvent& eventObj)
//Runs when data arrives on the socket
{
    string dataStr;
    
    //Retrieve incoming data from socketPtr
    if(!(socketPtr->GetData(dataStr)))
    {
        dispBox.AppendText("Failed to retrieve data. \r\n\r\n");
        return;
    }
    
    //Print data
    dispBox.AppendText(dataStr.c_str());
    dispBox.AppendText("\r\n");
}

void Socket_OnError(CSocket* socketPtr, const CSocketEvent& eventObj)
//Something went wrong with the socket
{
    dispBox.AppendText("Socket error: '");
    dispBox.AppendText((socketPtr->socketErrorStr).c_str());
    dispBox.AppendText("'. \r\n\r\n");
    
    UpdateUI();
}

void UpdateUI(void)
//Update the user interface controls
{
    //Update connection info controls
    if(clientSocketPtr->socketState == SOCKET_STATE_DISCONNECTED)
    {
        connectBtn.Enable();
        disconnectBtn.Disable();
        hostEdit.Enable();
        portEdit.Enable();
    }
    else
    {
	    connectBtn.Disable();
        disconnectBtn.Enable();
        hostEdit.Disable();
        portEdit.Disable();
	}
    
    //Update data-sending controls
    if(clientSocketPtr->socketState == SOCKET_STATE_CONNECTED)
    {
	    dataEdit.Enable();
        sendBtn.Enable();
	}
    else
    {
	    dataEdit.Disable();
        sendBtn.Disable();
	}
    
    //Display the current socket state
    dispBox.AppendText("Socket ");
    dispBox.AppendText(clientSocketPtr->GetStateStr());
    dispBox.AppendText("\r\n\r\n");
    statusBox.SetText(clientSocketPtr->GetStateStr());
}

void SendData(void)
//Send data from the data edit box to the remote host
{
    string dataStr;
    
    //Get data to send
    dataStr = dataEdit.GetText();
    dataStr += "\r\n";
    
    //Print data to display
    dispBox.AppendText(">>");
    dispBox.AppendText(dataStr.c_str());
    dispBox.AppendText("\r\n");
    
    //Clear dataEdit
    dataEdit.SetText("");
    
    //Send data to remote host
    if(!(clientSocketPtr->SendData(dataStr)))
    {
        dispBox.AppendText("Failed to send data. \r\n\r\n");
    }
}

