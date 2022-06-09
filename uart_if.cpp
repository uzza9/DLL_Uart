/* --------------------------------------------------------------------------- */
// Name:         uart_if
// Purpose:      handle UART interface
// Revision:     rev_1.0
// Author:       C Jeon
// Created:      05/15/2021
// last revision:06/07/2021
/* ---------------------------------------------------------------------------*/

#include    "protocol_msg.h"
#include    "uart_if.h"

HANDLE hCom; 
DCB dcb;
BOOL fSuccess;
DWORD g_BytesTransferred;

bool close_uart()
{
    CloseHandle(hCom);
    return TRUE; // return bool as return type of __stdcall < _cdecl => out of _cdelc
}

bool init_uart()
{
    hCom = CreateFile("\\\\.\\COM3",                // pcCommPort, name of the write
                       GENERIC_WRITE|GENERIC_READ,  // open for writing/reading
                       FILE_SHARE_READ,             // share i.e. mutual access
                       0,                           // default security
                       OPEN_EXISTING,               // existing file only
                       FILE_ATTRIBUTE_NORMAL,       // normal file (sync), not overlapped (async)
                       0);                          // no attr. template, hTemplate must be NULL for comm devices 

    if (hCom == INVALID_HANDLE_VALUE) 
    {
        return FALSE;   // return bool as return type of __stdcall < _cdecl => out of _cdelc
    }
    return TRUE;        // return bool as return type of __stdcall < _cdecl => out of _cdelc
}

bool config_port()
{
    //  Initialize the DCB structure.
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    //  Fill in some DCB values and set the com state: 
    dcb.BaudRate = CBR_115200;      //  baud rate
    // dcb.BaudRate = 363636UL;     //  baud rate
    dcb.ByteSize = 8;               //  data size, xmit and rcv
    dcb.Parity   = NOPARITY;        //  parity bit
    dcb.StopBits = ONESTOPBIT;      //  stop bit

    fSuccess = SetCommState(hCom, &dcb);

    if (!fSuccess) 
    {
        //  Handle the error.
        printf ("SetCommState failed with error %d.\n", GetLastError());
        return FALSE; // return bool as return type of __stdcall < _cdecl => out of _cdelc
    }

    //  Get the comm config again.
    fSuccess = GetCommState(hCom, &dcb);

    if (!fSuccess) 
    {
        //  Handle the error.
        printf ("GetCommState failed with error %d.\n", GetLastError());
        return FALSE; // return bool as return type of __stdcall < _cdecl => out of _cdelc
    }

    PrintCommState();       //  Output to console

    return TRUE; // return bool as return type of __stdcall < _cdecl => out of _cdelc
}

void PrintCommState()
{
    //  Print some of the DCB structure values
    printf("@@@ uart_if::PrintCommState: BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n",
              dcb.BaudRate, 
              dcb.ByteSize, 
              dcb.Parity,
              dcb.StopBits);
}

bool __cdecl uart_sendEx(t_u8 *txBuff, t_u16 txLen)
{
    OVERLAPPED osWrite = {0};
    DWORD dwBytesToWrite = (DWORD)txLen;
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;

    bErrorFlag = WriteFile( 
                    hCom,               // open file handle
                    txBuff,             // start of data to write
                    dwBytesToWrite,     // number of bytes to write
                    &dwBytesWritten,    // number of bytes that were written
                    &osWrite);          // overlapped structure with SleepEx()

    if (FALSE == bErrorFlag)
    {
        printf("@@@ uart_if::uart_send: Terminal failure: Unable to write to file. ====> ignore for a while !!!\n");
        // OK when FILE_ATTRIBUTE_NORMAL, FALSE when FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED
    }
    else
    {
        if (dwBytesWritten != dwBytesToWrite)
        {
            // This is an error because a synchronous write that results in
            // success (WriteFile returns TRUE) should write all data as
            // requested. This would not necessarily be the case for
            // asynchronous writes.
            printf("@@@ uart_if::uart_send: Error >>> dwBytesWritten != dwBytesToWrite\n");
        }
        else
        {
            // printf("@@@ uart_if::uart_send: Wrote %d bytes to %s successfully.\n", dwBytesWritten, "COM PORT");
        }
    }
    ///////////////////////////////////////////////////////
    // mSec, Time critical and while txing than rxing
    SleepEx(1, TRUE);               
    ///////////////////////////////////////////////////////
    return bErrorFlag;
}

bool __cdecl uart_send(t_u8 *txBuff, t_u16 txLen)
{
    OVERLAPPED osWrite = {0};
    DWORD dwBytesToWrite = (DWORD)txLen;
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;

    bErrorFlag = WriteFile( 
                    hCom,           // open file handle
                    txBuff,      // start of data to write
                    dwBytesToWrite,  // number of bytes to write
                    &dwBytesWritten, // number of bytes that were written
                    &osWrite);           //NULL);            // no overlapped structure

    if (FALSE == bErrorFlag)
    {
        printf("@@@ uart_if::uart_send: Terminal failure: Unable to write to file. ====> ignore for a while !!!\n");
        // OK when FILE_ATTRIBUTE_NORMAL, FALSE when FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED
    }
    else
    {
        if (dwBytesWritten != dwBytesToWrite)
        {
            // This is an error because a synchronous write all data as requested. 
            // This would not necessarily be the case for asynchronous writes.
            printf("@@@ uart_if::uart_send: Error >>> dwBytesWritten != dwBytesToWrite\n");
        }
        else
        {
            // printf("@@@ uart_if::uart_send: Wrote %d bytes to %s successfully.\n", dwBytesWritten, "COM PORT");
        }
    }
    return bErrorFlag;
}

VOID CALLBACK FileIOCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped )
 {
    g_BytesTransferred = dwNumberOfBytesTransfered;
    printf("@@@ uart_if::FileIOCompletionRoutine: Read %d bytes  >>>>>>>>>>>>>>>>>>>>\n", g_BytesTransferred);
 }

t_u16 __cdecl uart_receive(Pkt_Info &rx_Pkt)
{
    DWORD  dwBytesRead = 0;
    BOOL fWaitingOnRead = FALSE;
    t_u8 rxBuff[BUFFER_SIZE+1] = {0};
    DWORD numBytesRead = 0;

    OVERLAPPED ol = {0};
    
    COMMTIMEOUTS timeouts = { 0, //read interval timeout for non blocking mode. 0 = not used
                              0, // read multiplier
                              0, // read constant (milliseconds)           // 15 for 128 bytes at 115200 bps = 8.8ms, 5 for 128 B at 363636 bps
                              0, // Write multiplier
                              0  // Write Constant
                            };
    SetCommTimeouts(hCom, &timeouts);

    // Read one character less than the buffer size to save room for the terminating NULL character. 
    rx_Pkt.data.reserve(rx_Pkt.len + 1);

    while (numBytesRead == 0)
    {
        // OK when FILE_ATTRIBUTE_NORMAL, FALSE when FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED
        t_u16 err = ReadFile(hCom, rxBuff, rx_Pkt.len, &numBytesRead, NULL);
        // if ReadFile return true, mean that io operation already completed. check with GetOverlappedResult()

        if (!err && GetLastError() != ERROR_IO_PENDING)     // read not delayed?, ERROR_IO_PENDING(0x3E5)
        {
            // Error in communications; report it.
            printf("@@@ uart_if::uart_receive: ReadFile failed with error %d (6:ERROR_INVALID_HANDLE)\n", GetLastError());

            // err = GetOverlappedResult(hCom, &ol, &numBytesRead, FALSE);
            // Retrieves the results of an overlapped operation

            // printf("@@@ uart_if::uart_receive: Return of GetOverlappedResult() = %d, GetLastError= %d\n", err, GetLastError());
            // GetLastError() = 996: ERROR_IO_INCOMPLETE (Overlapped I/O event is not in a signaled state.)
            // Overlapped operation is still in progress
        }
        else
        {
            WaitForSingleObject(&(ol.hEvent), 0);
            // read completed immediately
        }
        dwBytesRead += numBytesRead;

        if (dwBytesRead > 0 && dwBytesRead <= BUFFER_SIZE)
        {
            rxBuff[dwBytesRead + 1]='\0'; // NULL character
            rx_Pkt.data.insert(rx_Pkt.data.end(), rxBuff, rxBuff + dwBytesRead);
            rxBuff[BUFFER_SIZE + 1] = {0};
        }
    }
    return dwBytesRead;
}

t_u8 __cdecl uart_receive_abyte(t_u16 rxDataNum)
{
    t_u8 rxBuff;
    DWORD numBytesRead = 0;

    COMMTIMEOUTS timeouts = { 0, //interval timeout. 0 = not used
                              0, // read multiplier
                              1, // read constant (milliseconds)
                              0, // Write multiplier
                              0  // Write Constant
                            };
    SetCommTimeouts(hCom, &timeouts);

    if(FALSE == ReadFile(hCom, &rxBuff, rxDataNum, &numBytesRead, NULL))
    {
        printf("@@@ uart_if::uart_receive_abyte: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
        CloseHandle(hCom);
        // return 0;
        // return nullptr;
    }
    return rxBuff;
}