#ifndef     CMN_CFG_H
#define     CMN_CFG_H

#ifdef _WIN32
#include    <Windows.h>
#else
#include    <unistd.h>
#endif

#include    <fstream>
#include    <iostream>
#include    <cstdlib>
#include    <cstring>

#include    <chrono>
#include    <thread>
#include    <queue>

#include    <vector>
#include    <array>


#if defined(__cplusplus)
extern "C" {
#endif

/* ************************************************************ */
/* gloval constant                                              */
/* ************************************************************ */ 
// system (platform)

#define     POINTER_UNIT_BYTES          4       // not necessary

// target
#define     BL_VER_0                    0
#define     BL_VER_1                    1
#define     BL_VER_2                    2


#define     DESC_BYTES                  128

#define     PKT_LEN_BYTES               2
#define     PKT_ADDR_BYTES              4    
#define     BUFFER_SIZE                 512
#define     CRC16_BYTES                 2
#define     MAX_READ_DATA_SIZE          512             // 128 for R1, 512 for R2

#define     HAND_SHAKE_NUM              1

#define     DELIMITER                   '\0'

// data structure
#define     MAX_RETURN_INFO             5
#define     INFO_HEADER                 0
#define     INFO_PAYLOAD_1              1
#define     INFO_PAYLOAD_2              2
#define     INFO_TAIL                   4


/* ************************************************************ */
/* global variables                                             */
/* ************************************************************ */
// extern globalVar    refVar



/* ************************************************************ */
/* type definition                                              */
/* ************************************************************ */
typedef unsigned long long  t_u64;
typedef unsigned long int   t_u32;
typedef unsigned short int  t_u16;
typedef unsigned char       t_u8;

typedef signed long long    t_s64;
typedef signed long int     t_s32;
typedef signed short int    t_s16;
typedef signed char         t_s8;

/* ************************************************************ */
/* data structure for each messages                             */
/* ************************************************************ */

struct Uart_Cfg
{
    HANDLE  hCom; 

    DCB     dcb;
    BOOL    fSuccess;

    DWORD   g_BytesTransferred = 0;
};

// Addr_Info of Bootloader version 1
struct Addr_InfoR1
{
    t_u16   dataByteCnt;

    t_u8    blVer[4];
    t_u8    flashBaseAddr[4];
    t_u8    flashEndAddr[4];
    t_u8    blSNAddr[4];
    t_u8    appStartAddr[4];
    t_u8    e2StartAddr[4];
    t_u8    e2EndAddr[4];

    // template<t_u8 numId, t_u8 codeVal> 
    // template <typename T, size_t numId, size_t codeVal>
    // using serialNum = std::array<std::array<std::array<T,  codeVal>, numId>;
    // t_u8    serialNum**;
    // std::vector<t_u8>    serialNum[2];
    t_u8*   serialNum[2][4];

    t_u8    cntlerTxtIdAddr[4];

        #define     SIZE_R1_CNTLER_ID           128
    t_u8    cntlerTxtIdBytes[SIZE_R1_CNTLER_ID]; 

    t_u16   buffSize;
    t_u8    lastWrittenAddr[4];
};

// Addr_Info of Bootloader version 2
struct Extra_MemRange
{
    t_u8    startAddr[4] = {0};
    t_u8    endAddr[4] = {0};
    bool    flagUsed;
};

struct Addr_InfoR2
{
    t_u16   dataByteCnt;
    t_u8    blVer[4];
    t_u8    flashBaseAddr[4];
    t_u8    flashEndAddr[4];
    t_u8    wrOnceStartAddr[4];
    t_u8    wrOnceEndAddr[4];
    t_u8    appStartAddr[4];
    t_u8    appEndAddr[4]; 
    t_u8    e2StartAddr[4];
    t_u8    e2EndAddr[4];
    t_u8    pgBlSNAddr[4];
    t_u8    pgSNByteCnt;
    t_u8    pgCntlerTxtIdAddr[4];
    t_u8    pgLabelByteCnt[2];
    t_u8    pgCpuId[4];
    t_u8    mfCpuPkgIdAddr[4];
    t_u8    mfCpuPkgIDBytes;
    t_u8    mfCpuSNAddrUniqId[4];
    t_u8    mfCpuSNBytes;
    t_u8    mfCpuFlashSize[4];
    t_u8    cpuFlashOptAddr[4];
    t_u8    cpuFlashOptEndAddr[4];
    t_u8    blCapa1[4];
    t_u8    blCapa2[4];
    t_u8    bitsPerMemAddr;
    t_u8    cpuEndianBig;
    t_u8    qtyAddMemInfoRange;

        #define     CNT_MAX_EXTRA_MEM_RANGE     2
        #define     SIZE_MAX_BUILD_DATE         40
        #define     SIZE_R2_CNTLER_ID           128

    Extra_MemRange    extra_MemRange[CNT_MAX_EXTRA_MEM_RANGE];

    t_u8    blBuildDate[SIZE_MAX_BUILD_DATE];

    t_u8    serialNum[8];
    t_u8    cntlerTxtIdBytes[SIZE_R2_CNTLER_ID];

    bool    flagEngAccessEnabled;

    // reserved for future usage
    t_u16   buffSize;
    t_u8    lastWrittenAddr[4];
};

struct Tgt_Info
{
    t_u16   blVer = 0;
    t_u16   sn1 = 0;
    t_u16   sn2 = 0;
   
    t_u8    desc[DESC_BYTES + 1];
};

struct Addr_Range
{
    t_u32   sAddr;
    t_u32   eAddr;
};

struct Pkt_Info
{
    t_u16               len = 0;  
    std::vector<t_u8>   data;
};

struct Rtn_Info
{
    bool                rslt = FALSE;
    bool                status = FALSE;
    bool                pwrCycle = FALSE;

    Pkt_Info            pkt_Info;
};


/* duplicated. same in Uart_Message.h
struct Protocol_Info
{
    t_u16   lenCmd;
    t_u8    *rqstCmd;
    t_u16   lenAck;
    t_u8    *respAck;       
};
*/

struct Cmd_Info
{
    t_u16               cmdByteNum;
    t_u16               ackByteNum;
    std::vector<t_u8>   cmd;
    std::vector<t_u8>   ack;
    t_u16               rxByteNum;
};

// Addr_InfoC if necessary e.g. infoR_lower is not subset of InfoR_upper
// typedef struct _Addr_InfoC


/* ************************************************************ */
/* USER CODE BEGIN                                                */
/* ************************************************************ */ 





/* ************************************************************ */
/* USER CODE END                                                        */
/* ************************************************************ */ 

#if defined(__cplusplus)
}
#endif      // end (__cplusplus)

#endif      // end CMN_CFG_H