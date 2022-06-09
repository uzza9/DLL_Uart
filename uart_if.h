#ifndef     UART_IF_H
#define     UART_IF_H

// #include <windows.h>
/*
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#include <fileapi.h>
*/

#include    "cmn_cfg.h"


/* ************************************************************ */
/* USER CODE BEGIN                                                */
/* ************************************************************ */ 
// #define BUFFERSIZE 512

/* variables */



/* function prototypes */
bool        init_uart();
bool        config_port();
void        PrintCommState();

bool        uart_sendEx(t_u8 *txBuff, t_u16 txLen);
bool        uart_send(t_u8 *txBuff, t_u16 txLen);

t_u16       uart_receive(Pkt_Info &rx_Pkt);
// t_u16 uart_receive(Pkt_Info &rx_Pkt);

t_u8        uart_receive_abyte(t_u16 rxDataNum);

// unsigned char * uart_rx_singleByte();

bool        close_uart();

// void run_uart(bool flagTst);

VOID CALLBACK FileIOCompletionRoutine(
    DWORD   dwErrorCode,
    DWORD   dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
);

/* ************************************************************ */
/* USER CODE END                                                        */
/* ************************************************************ */ 

#endif      // end UART_MESSAGES_H