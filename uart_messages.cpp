/* --------------------------------------------------------------------------- */
// Name:         uart_messages
// Purpose:      handle UART messages
// Revision:     rev_1.0
// Author:       C Jeon
// Created:      05/15/2021
// last revision:06/07/2021
/* ---------------------------------------------------------------------------*/

#include "uart_messages.h"

// CMN_DEF_DBG__TAG(App)
// #define D_d(__fmt__)        D_dbgp(App, (8, "EXPORT] "), __fmt__)


extern HANDLE hCom; 

extern DCB dcb;
extern BOOL fSuccess;

extern DWORD g_BytesTransferred;

bool Uart_Messages::fBlReady = FALSE;

///////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////
void Uart_Messages::initialize()
{
    clear_addr_infoR1();
    clear_addr_infoR2();
    mapping_to_addr_infoC();   
}

void Uart_Messages::clear_addr_infoR1()
{
    t_u16   i;

    m_addr_InfoR1.dataByteCnt           = 0;
    m_addr_InfoR1.blVer[4]              = {0};
    m_addr_InfoR1.flashBaseAddr[4]      = {0};
    m_addr_InfoR1.flashEndAddr[4]       = {0};
    m_addr_InfoR1.blSNAddr[4]           = {0};
    m_addr_InfoR1.appStartAddr[4]       = {0};
    m_addr_InfoR1.e2StartAddr[4]        = {0};
    m_addr_InfoR1.e2EndAddr[4]          = {0};
    m_addr_InfoR1.serialNum[0][4]       = {0};
    m_addr_InfoR1.serialNum[1][4]       = {0};
    m_addr_InfoR1.cntlerTxtIdAddr[4]    = {0};

    for(i = 0; i < SIZE_R1_CNTLER_ID; i++)
    {
        m_addr_InfoR1.cntlerTxtIdBytes[i] = 0;
    }
}

void Uart_Messages::clear_addr_infoR2()
{
    m_addr_InfoR2.dataByteCnt           = 0;
    m_addr_InfoR2.blVer[4]              = {0};
    m_addr_InfoR2.flashBaseAddr[4]      = {0};
    m_addr_InfoR2.flashEndAddr[4]       = {0};
    m_addr_InfoR2.wrOnceStartAddr[4]    = {0};
    m_addr_InfoR2.wrOnceEndAddr[4]      = {0};
    m_addr_InfoR2.appStartAddr[4]       = {0};
    m_addr_InfoR2.appEndAddr[4]         = {0};
    m_addr_InfoR2.e2StartAddr[4]        = {0};
    m_addr_InfoR2.e2EndAddr[4]          = {0};
    m_addr_InfoR2.pgBlSNAddr[4]         = {0};
    m_addr_InfoR2.pgSNByteCnt           = 0;
    m_addr_InfoR2.pgCntlerTxtIdAddr[4]  = {0};
    m_addr_InfoR2.pgLabelByteCnt[2]     = {0};
    m_addr_InfoR2.pgCpuId[4]            = {0};
    m_addr_InfoR2.mfCpuPkgIdAddr[4]     = {0};
    m_addr_InfoR2.mfCpuPkgIDBytes       = 0;
    m_addr_InfoR2.mfCpuSNAddrUniqId[4]  = {0};
    m_addr_InfoR2.mfCpuSNBytes          = 0;
    m_addr_InfoR2.mfCpuFlashSize[4]     = {0};
    m_addr_InfoR2.cpuFlashOptAddr[4]    = {0};
    m_addr_InfoR2.cpuFlashOptEndAddr[4] = {0};
    m_addr_InfoR2.blCapa1[4]            = {0};
    m_addr_InfoR2.blCapa2[4]            = {0};
    m_addr_InfoR2.bitsPerMemAddr        = 0;
    m_addr_InfoR2.cpuEndianBig          = 0;
    m_addr_InfoR2.qtyAddMemInfoRange    = 0;

    m_addr_InfoR2.serialNum[8]   = {0};

    memset(&m_addr_InfoR2.extra_MemRange, 0xff, sizeof(m_addr_InfoR2.extra_MemRange));
    memset(&m_addr_InfoR2.blBuildDate, 0xff,sizeof(m_addr_InfoR2.blBuildDate));
    memset(&m_addr_InfoR2.cntlerTxtIdBytes, 0xff, sizeof(m_addr_InfoR2.cntlerTxtIdBytes));       

}

void Uart_Messages::mapping_to_addr_infoC()
{
    if(blVer == BL_VER_1)
    {
        m_addr_InfoC.dataByteCnt = m_addr_InfoR1.dataByteCnt;
        memcpy(m_addr_InfoC.blVer, m_addr_InfoR1.blVer, 4);
        memcpy(m_addr_InfoC.flashBaseAddr, m_addr_InfoR1.flashBaseAddr, 4);
        memcpy(m_addr_InfoC.flashEndAddr, m_addr_InfoR1.flashEndAddr, 4);
        memcpy(m_addr_InfoC.wrOnceStartAddr, m_addr_InfoR1.blSNAddr, 4);
        memcpy(m_addr_InfoC.appStartAddr, m_addr_InfoR1.appStartAddr, 4);
        memcpy(m_addr_InfoC.e2StartAddr, m_addr_InfoR1.e2StartAddr, 4);
        memcpy(m_addr_InfoC.e2EndAddr, m_addr_InfoR1.e2EndAddr, 4);
        memcpy((t_u8*)(m_addr_InfoC.serialNum), m_addr_InfoR1.serialNum[0], 4);
        memcpy((t_u8*)(m_addr_InfoC.serialNum+4), m_addr_InfoR1.serialNum[1], 4);

        m_addr_InfoC.buffSize = m_addr_InfoR1.buffSize;
        memcpy(m_addr_InfoC.lastWrittenAddr, m_addr_InfoR1.lastWrittenAddr, 4);

        memcpy(&m_addr_InfoC.cntlerTxtIdBytes, &m_addr_InfoR1.cntlerTxtIdBytes, sizeof(m_addr_InfoR1.cntlerTxtIdBytes));
    }
    else if(blVer == BL_VER_2)
    {
        memcpy(&m_addr_InfoC, &m_addr_InfoR2, sizeof(m_addr_InfoR2));
    }
}

/*
void Uart_Messages::check_timeout()
{

}
*/

bool Uart_Messages::parse_addr_infoR1(Pkt_Info &rxPkt_Info)
{
    bool    fRslt;
    t_u8    data[rxPkt_Info.len + 1];                     // rxPkt_Info.len = len(2 bytes) + data(168 bytes) + crc16(2 bytes)
    
    memset(data, '\0', rxPkt_Info.len + 1);
    memcpy(data, &rxPkt_Info.data[0], rxPkt_Info.len);       // data will be swapped when reverse_bytes(...)
    // printf("@@@ uart_Messages::parse_addr_infoR1: buff data[0]= %d, data[1]= %d\n", data[0], data[1]);


    m_addr_InfoR1.dataByteCnt = Inline_Lib::convert_binary_int(data[0], data[1]);
    // printf("@@@ uart_Messages::parse_addr_infoR1: dataByteCnt = %d\n", m_addr_InfoR1.dataByteCnt);

    memcpy(m_addr_InfoR1.blVer, Inline_Lib::reverse_bytes((t_u8*)(data + 2), 4), 4);
    blVer = Inline_Lib::convert_binary_int4(m_addr_InfoR1.blVer, 4);

    // printf("@@@ uart_Messages::parse_addr_infoR1: blVer = %d\n", m_addr_InfoR1.blVer[0]);
    memcpy(m_addr_InfoR1.flashBaseAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 6), 4), 4);
    memcpy(m_addr_InfoR1.flashEndAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 10), 4), 4);
    memcpy(m_addr_InfoR1.blSNAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 14), 4), 4);
    memcpy(m_addr_InfoR1.appStartAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 18), 4), 4);
    memcpy(m_addr_InfoR1.e2StartAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 22), 4), 4);
    memcpy(m_addr_InfoR1.e2EndAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 26), 4), 4);
    memcpy(m_addr_InfoR1.serialNum[0], Inline_Lib::reverse_bytes((t_u8*)(data + 30), 4), 4);
    memcpy(m_addr_InfoR1.serialNum[1], Inline_Lib::reverse_bytes((t_u8*)(data + 34), 4), 4);
    memcpy(m_addr_InfoR1.cntlerTxtIdAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 38), 4), 4);

    memcpy(m_addr_InfoR1.cntlerTxtIdBytes, (t_u8*)(data + 42), SIZE_R1_CNTLER_ID);

    t_u16   calCrc = Inline_Lib::generate_crc16(&rxPkt_Info.data[0], rxPkt_Info.len - 2);        // len + data for generate CRC16 i.e. except rxed CRC16 (2 bytes)
    t_u16   rxCrc = Inline_Lib::convert_binary_int(rxPkt_Info.data.at(rxPkt_Info.len - 2), rxPkt_Info.data.at(rxPkt_Info.len - 1));   
    printf("@@@ uart_Messages::parse_addr_infoR1: calCrc = %d, rxCrc= %d\n", calCrc, rxCrc);

    if (rxCrc == calCrc)
        fRslt = TRUE;
    else
    {
        fRslt = FALSE;
        printf("@@@ uart_Messages::parse_addr_infoR1: Error for CRC calCrc = %d, rxCrc= %d\n", calCrc, rxCrc);
    }

    return fRslt;
}

bool Uart_Messages::parse_addr_infoR2(Pkt_Info &rxPkt_Info)
{
    bool    fRslt;

    t_u8    data[rxPkt_Info.len + CRC16_BYTES + 1];                   // rxPkt_Info.len = 168 i.e. only data except len(2 bytes) and crc16 (2 bytes)
    memset(data, '\0', rxPkt_Info.len + CRC16_BYTES + 1);
    memcpy(data, &rxPkt_Info.data[0], rxPkt_Info.len + CRC16_BYTES);      // data swapped when reverse_bytes(...)

    m_addr_InfoR2.dataByteCnt = Inline_Lib::convert_binary_int(data[0], data[1]);
    // printf("@@@ uart_Messages::parse_addr_infoR2: dataByteCnt = %d\n", m_addr_InfoR2.dataByteCnt);

    memcpy(m_addr_InfoR2.blVer, Inline_Lib::reverse_bytes((t_u8*)(data + 2), 4), 4);
    memcpy(m_addr_InfoR2.flashBaseAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 6), 4), 4);
    memcpy(m_addr_InfoR2.flashEndAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 10), 4), 4);
    memcpy(m_addr_InfoR2.wrOnceStartAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 14), 4), 4);
    memcpy(m_addr_InfoR2.wrOnceEndAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 18), 4), 4);
    memcpy(m_addr_InfoR2.appStartAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 22), 4), 4);
    memcpy(m_addr_InfoR2.appEndAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 26), 4), 4);
    memcpy(m_addr_InfoR2.e2StartAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 30), 4), 4);
    memcpy(m_addr_InfoR2.e2EndAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 34), 4), 4);
    m_addr_InfoR2.pgSNByteCnt = data[38];
    memcpy(m_addr_InfoR2.pgCntlerTxtIdAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 39), 4), 4);
    memcpy(m_addr_InfoR2.pgLabelByteCnt, Inline_Lib::reverse_bytes((t_u8*)(data + 43), 2), 2);
    memcpy(m_addr_InfoR2.pgCpuId, Inline_Lib::reverse_bytes((t_u8*)(data + 45), 4), 4);
    memcpy(m_addr_InfoR2.mfCpuPkgIdAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 49), 4), 4);
    m_addr_InfoR2.mfCpuPkgIDBytes = data[53];
    memcpy(m_addr_InfoR2.mfCpuSNAddrUniqId, Inline_Lib::reverse_bytes((t_u8*)(data + 54), 4), 4);
    m_addr_InfoR2.mfCpuSNBytes = data[58];
    memcpy(m_addr_InfoR2.mfCpuFlashSize, Inline_Lib::reverse_bytes((t_u8*)(data + 59), 4), 4);
    memcpy(m_addr_InfoR2.cpuFlashOptAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 63), 4), 4);
    memcpy(m_addr_InfoR2.cpuFlashOptEndAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 67), 4), 4);
    memcpy(m_addr_InfoR2.blCapa1, Inline_Lib::reverse_bytes((t_u8*)(data + 71), 4), 4);
    memcpy(m_addr_InfoR2.blCapa2, Inline_Lib::reverse_bytes((t_u8*)(data + 75), 4), 4);   
    m_addr_InfoR2.bitsPerMemAddr = data[79];
    m_addr_InfoR2.cpuEndianBig = data[80];
    m_addr_InfoR2.qtyAddMemInfoRange = data[81];

    if (m_addr_InfoR2.qtyAddMemInfoRange > 0)
    {
        memcpy(m_addr_InfoR2.extra_MemRange[0].startAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 82), 4), 4);    
        memcpy(m_addr_InfoR2.extra_MemRange[0].endAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 86), 4), 4);    
        memcpy(m_addr_InfoR2.extra_MemRange[1].startAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 90), 4), 4);    
        memcpy(m_addr_InfoR2.extra_MemRange[1].endAddr, Inline_Lib::reverse_bytes((t_u8*)(data + 94), 4), 4);    

        memcpy(m_addr_InfoR2.blBuildDate, (t_u8*)(&data[98]), SIZE_MAX_BUILD_DATE);    
        memcpy(m_addr_InfoR2.serialNum, Inline_Lib::reverse_bytes((t_u8*)(data + 138), 8), 8);    
        memcpy(m_addr_InfoR2.cntlerTxtIdBytes, (t_u8*)(data + 146), SIZE_R2_CNTLER_ID);    
    }
    else
    {
        memcpy(m_addr_InfoR2.blBuildDate, (t_u8*)(data + 82), SIZE_MAX_BUILD_DATE);
        memcpy(m_addr_InfoR2.serialNum, Inline_Lib::reverse_bytes((t_u8*)(data + 122), 8), 8);    
        memcpy(m_addr_InfoR2.cntlerTxtIdBytes, (t_u8*)(data + 130), SIZE_R2_CNTLER_ID);            
    }

    t_u16   calCrc = Inline_Lib::generate_crc16(&rxPkt_Info.data[0], rxPkt_Info.len - 2);        // len + data for generate CRC16 i.e. except rxed CRC16 (2 bytes)
    t_u16   rxCrc = Inline_Lib::convert_binary_int(rxPkt_Info.data.at(rxPkt_Info.len - 2), rxPkt_Info.data.at(rxPkt_Info.len - 1));   

    printf("@@@ uart_Messages::parse_addr_infoR1: calCrc = %d, rxCrc= %d\n", calCrc, rxCrc);

    if (rxCrc == calCrc)
        fRslt = TRUE;
    else
    {
        fRslt = FALSE;
        printf("@@@ uart_Messages::parse_addr_infoR1: Error for CRC calCrc = %d, rxCrc= %d\n", calCrc, rxCrc);
    }

    return fRslt;
}

bool Uart_Messages::parse_last_writtenAddr(t_u8 *data)
{
    memcpy(m_addr_InfoR1.lastWrittenAddr, (t_u8*)(data), 4);;
    memcpy(m_addr_InfoR2.lastWrittenAddr, (t_u8*)(data), 4);;
    // for (int i = 0; i < 4; i++)
    //    printf("@@@ uart_Messages::parse_last_writtenAddr: addr[%d] = %d\n", i, m_addr_InfoR1.lastWrittenAddr[i]);

    return TRUE;
}

bool Uart_Messages::parse_buff_size(t_u8 *data)
{
    m_addr_InfoR1.buffSize = ((t_u16)data[0] << 8) + (t_u16)data[1];
    m_addr_InfoR2.buffSize = m_addr_InfoR1.buffSize;
    // m_addr_InfoR2.buffSize = ((t_u16)data[0] << 8) + (t_u16)data[1];

    // printf("@@@ uart_Messages::parse_buff_size: buffSize = %d\n", m_addr_InfoR1.buffSize);
    return TRUE;
}

void Uart_Messages::convert_int_bytes_R2(t_u32 intVal, t_u16 num, t_u8 (&rtn)[4])
{
    t_u16   i;
    t_u8    temp[4];
    for (i = 0; i < num; i++)
    {
        temp[num - 1 - i] = (intVal >> (i * 8));
        printf("@@@ Parse_Messages::Uart_Messages:: convert_int_bytes_R2: temp[%d]= %d\n", i, temp);
    }
    // memcpy(rtn, temp, num);
    for (i = 0; i < num; i++)
        rtn[i] = temp[i];
}  

bool Uart_Messages::rx_uart_pkt(t_u16 dataLen, Pkt_Info &rxPkt_Info)
{
    bool    fRslt_T = TRUE;
    t_u8    tempBuff[dataLen + 1];
    // std::vector<t_u8>  tempBuff(dataLen + 1);

    t_u16   totalRxedBytes = 0;        
    rxPkt_Info.len = dataLen;
    while (totalRxedBytes < dataLen)
    {
        fRslt_T &= uart_receive(rxPkt_Info);
        memcpy(&tempBuff[totalRxedBytes], &rxPkt_Info.data[0], rxPkt_Info.len);
        totalRxedBytes += rxPkt_Info.len; 
    }
    
    memset(&rxPkt_Info.data[0], '\0', dataLen + 1);
    memcpy(&rxPkt_Info.data[0], tempBuff, totalRxedBytes);

    return fRslt_T;
}

bool Uart_Messages::handle_single_rqst_resp(t_u8 cmd, t_u8 ack)
{
    bool    rslt;
    t_u8    rxByte;

    // printf("@@@ uart_Messages::handle_single_rqst_resp: cmd = %d,    resp = %d\n", cmd, ack);
    // timestamp_start("@@@ Uart_Messages::handle_single_rqst_resp: Current Time at starting CMD/ACK >>> ");
    try
    {
        rslt = uart_send(&cmd, 1);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    if (rslt)
    {
        while(rxByte != ack)       // condition with timeout and UART status
        {
            try
            {
                rxByte = uart_receive_abyte(1);
                // timestamp_start("@@@ Uart_Messages::handle_single_rqst_resp: Current Time at completing CMD/ACK >>> ");
                // timestamp_elapsed("@@@ Uart_Messages::handle_single_rqst_resp: Elapsede time for CMD/ACK >>> ");   

                if (rxByte == ack) rslt = TRUE;
            }
            catch(...)
            {
                rslt = FALSE;
                std::cout << "@@@ uart_Messages::handle_single_rqst_resp: default exception" << std::endl;
            }
        }
    }
    return rslt;
}

bool Uart_Messages::handle_single_rqst_resp(t_u8 *cmd, t_u8 *ack)
{
    bool    rslt;
    t_u8    rxByte;

    try
    {
        rslt = uart_send(cmd, 1);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    if (rslt)
    {
        while(rxByte != *ack)       // condition with timeout and UART status
        {
            try
            {
                rxByte = uart_receive_abyte(1);
                if (rxByte == *ack) rslt = TRUE;
            }
            catch(...)
            {
                rslt = FALSE;
                std::cout << "@@@ uart_Messages::handle_single_rqst_resp: default exception" << std::endl;
            }
        }
    }
    return rslt;
}

bool Uart_Messages::handle_base_rqst_resp(Cmd_Info &cmd_Info)
{
    bool    fRslt_T = TRUE; 

    for (int i = 0; i < cmd_Info.cmdByteNum; i++)
    {
        // printf("@@@ uart_Messages::handle_base_rqst_resp: Cmd_Info >>> cmd_Info.cmd.at(%d)= %d, cmd_Info.ack.at(%d)= %d\n", i, cmd_Info.cmd.at(i), i, cmd_Info.ack.at(i));
        fRslt_T &= handle_single_rqst_resp(cmd_Info.cmd.at(i), cmd_Info.ack.at(i));
    }
    return fRslt_T;
}

bool Uart_Messages::handle_base_rqst_resp(t_u8 *cmd, t_u8 *ack, t_u16 cmdLen)
{
    bool    fRslt_T = TRUE; 

    for (int i = 0; i < cmdLen; i++)
    {
        fRslt_T &= handle_single_rqst_resp(&(cmd[i]), &(ack[i]));
    }
    return fRslt_T;
}

/////////////////////////////////////////////////////////////////

void Uart_Messages::t_read_memory_rxPkt(Pkt_Info &rxPkt_Info)
{
    // When using std::thread to call the member function, 
    // the second argument should be an instance of the class (or the pointer to it).

    // since t_read_memory_respPkt not static
    std::thread tRMR(this->handle_read_memory_rxPkt, this, std::ref(rxPkt_Info));

    // if t_read_memory_respPkt static
    // have to create an instance of the class to pass to threads constructor
    // can share one object for both calls or give each thread it's own object
    // std::thread t(&Uart_Message::handle_read_memory_respPkt, Uart_Messages{}, std::ref(rxPkt_Info));

    // To use lambda
    // std::thread ([&](){ Uart_Messages{}.handle_read_memory_respPkt(rxPkt_Info); })

    // uart_send() then check the last rx prior to join()
    tRMR.join();
}

// void Uart_Messages::t_read_memory_txPkt(Addr_Range &addr_Range)
void Uart_Messages::t_read_memory_txPkt(t_u32 sAddr, t_u32 eAddr)
{
}

DWORD WINAPI Uart_Messages::t_read_memory_rxPkt_R0(Pkt_Info &rxPkt_Info)
{
    printf("@@@ uart_Messages::t_read_memory_rxPkt: start >>> \n");
    bool    fLoopDone = FALSE;
    t_u16   rxByteNum = 0;
    t_u16   totalByteNum = 0;
    t_u16   len = rxPkt_Info.len;                      // rx crc16 from target   

    while (not fLoopDone)
    {   
        rxByteNum = uart_receive(rxPkt_Info);
        // printf("@@@ uart_Messages::handle_read_memory_pkt: rxByteNum= %d, rxPkt_Info.data.size= %d\n", rxByteNum, rxPkt_Info.data.size());

        // dataLen = Inline_Lib::convert_binary_int(rxPkt_Info.data[0], rxPkt_Info.data[1]);
        totalByteNum += rxByteNum;

        if (totalByteNum == len)
        {
            fLoopDone = TRUE;

            // fRxBufFull = TRUE;          // called within static function

            // fRslt = TRUE;
            // printf("@@@ uart_Messages::handle_read_memory_pkt: rxPkt_Info.data[0]= %d, rxPkt_Info.data[1]=%d\n", rxPkt_Info.data.at(0), rxPkt_Info.data.at(1));
        }
    }    

    printf("@@@ uart_Messages::t_read_memory_rXkt: end >>> \n");

    return 1;    
}

DWORD WINAPI Uart_Messages::t_connect_pkt(void* proto_Info)
{
    // printf("@@@ uart_Messages::t_connect_pkt: start >>> fBlReady= %d>>>>>>>>>>>>>>>>>>>>\n", fBlReady);
    bool    fRslt = FALSE;
    bool    fTmp = FALSE;
    t_u8    rxBuff;
    t_u8    rxByte[2] = {0};

    Protocol_Info* rx_Info = (Protocol_Info*)proto_Info;

    while(not fBlReady)
    {
        rxBuff = uart_receive_abyte(1);
        // printf("@@@ uart_Messages::t_connect_pkt: rx ack[0] = %d\n", rx_Info->respAck[0]);

        if (rxBuff == rx_Info->respAck[0])
        {
            fBlReady = TRUE;
        }
        SleepEx(10, TRUE);

    }
    printf("@@@ uart_Messages::t_handle_connect_pkt: end >>> fBlReady= %d>>>>>>>>>>>>>>>>>>>>\n", fBlReady);

    return 1;
}

bool Uart_Messages::t_rqst_bootload_connecting(Rtn_Info &rtn_Info)
{
    bool    fRslt = FALSE;
    bool    fRslt_T = TRUE;
    t_u8    *rxBuff;
    t_u8    rxByte[2] = {0};

    t_u8    rqstCmd[2] = {CONNECT1, CONNECT2};
    t_u8    respAck[2] = {CONNECT1_RE, CONNECT2_RE};

    std::queue<int>  rsltQ;

    Protocol_Info    proto_Info;

    proto_Info.lenCmd = 2;

    proto_Info.rqstCmd = (t_u8*)malloc(sizeof(rqstCmd)); 
    proto_Info.respAck = (t_u8*)malloc(sizeof(respAck)); 

    memcpy(proto_Info.rqstCmd, rqstCmd, sizeof(rqstCmd));
    memcpy(proto_Info.respAck, respAck, sizeof(respAck));

    HANDLE tHndlConnMsg = CreateThread(NULL, 0, t_connect_pkt, &proto_Info, 0, NULL);

    // printf("@@@ uart_Messages::rqst_bootload_connecting: fBlReady= %d\n", fBlReady);
    while(not fBlReady)         // set at t_connect_pkt() once ack received
    {
        fRslt = uart_send(rqstCmd, 1);
        SleepEx(10, TRUE);
    }

    SleepEx(10, TRUE);

    if (fRslt)
    {
        fRslt_T &= handle_single_rqst_resp(rqstCmd[1], respAck[1]);

        if (fRslt_T)
        {
        }    
    }
    
    CloseHandle(tHndlConnMsg);

    printf("@@@ uart_Messages::rqst_bootload_connecting: completed successfully !!!\n");
    rtn_Info.status = TRUE;
    rtn_Info.pwrCycle = TRUE;

    free(proto_Info.rqstCmd);
    free(proto_Info.respAck);

    return fRslt;
}

//////////////////////////////////////////////////////////////////
void Uart_Messages::handle_read_memory_chunk_data(std::vector<t_u8> &chunkData, Pkt_Info &rxPkt_Info, t_u32 dataSize)
{
    // printf("@@@ uart_Messages::handle_read_memory_chunk_data: start >>> \n");
    totalRxBytes = dataSize;
    // fRxCmpltd = FALSE;
    while (not fRxCmpltd)
    {
        // fRxBufFull = get_flag_rx_full();
        if (fRxBufFull)
        {  
            chunkData.insert(chunkData.end(), rxPkt_Info.data.begin(), rxPkt_Info.data.end());
            fCDataUpdated = TRUE;
            fRxBufFull = FALSE;
            // printf("@@@ uart_Messages::handle_read_memory_chunk_data: size of rxPkt_Info.data= %d  BLOCK_DATA >>> BLOCK DATA >>> BLOCK DATA \n", rxPkt_Info.data.size());
           
            // rxPkt_Info.data.clear();             // clear after mutex::unlock
            // rxPkt_Info.len = uart_Msg.get_rx_pkt_len();
            // printf("@@@ main::read_data_packet: call set_flag_rx_full(FALSE) !!!\n");
            // set_flag_rx_full(FALSE);       // for mutex::unlock
        }

        if (totalRxBytes <= chunkData.size())
        {
            // printf("@@@ uart_Messages::handle_read_memory_chunk_data: fRxBufFull Cnt = %d, RxBytes to complete = %lu, chunkData.size()= %d\n", i, totalRxBytes, chunkData.size());
            fRxCmpltd = TRUE;
        }
    }

    std::cout << "uart_Messages::handle_read_memory_chunk_data accumlated chunkData size = " << chunkData.size() << std::endl; 
    chunkData.clear();
}


bool Uart_Messages::handle_read_memory_txPkt(t_u32 sAddr, t_u32 eAddr, Pkt_Info &rxPkt_Info)
{
    // printf("@@@ uart_Messages::handle_read_memory_txPkt: start >>> \n");

    bool    fRslt_T = TRUE;
    bool    fRslt = FALSE;
    bool    fLoopDone = FALSE;

    struct  Cmd_Info    cmd_Info;

    cmd_Info.cmdByteNum = 1;
    cmd_Info.ackByteNum = 1;
    cmd_Info.rxByteNum = (t_u16)MAX_READ_DATA_SIZE;         // payload bytes to be rxed = 0

    cmd_Info.cmd.push_back(READ_MEMORY_1);
    cmd_Info.ack.push_back(READ_MEMORY_1_RE);

    fTxCmpltd = FALSE;

    t_u16 quotient = (eAddr - sAddr + 1) / MAX_READ_DATA_SIZE;
    t_u16 remainder = (eAddr - sAddr + 1) % MAX_READ_DATA_SIZE;

    std::vector<t_u8> pktLen;
    pktLen.reserve(PKT_LEN_BYTES);          // 2 bytes

    std::vector<t_u8> addr;
    addr.reserve(4);                        // 4 bytes    

    std::vector<t_u8> pkt;
    pkt.reserve(pktLen.size() + addr.size());

    // printf("@@@ uart_Messages::handle_read_memory_txPkt: quotient= %d, remainder= %d >>>>>>>>>>>>>>>>>>\n", quotient, remainder);

    pktLen = Inline_Lib::convert_int_bytes(MAX_READ_DATA_SIZE, 2);
    t_u32   curAddr = sAddr;
    while (quotient > 0)
    {
        addr = Inline_Lib::convert_int_bytes(curAddr, 4);        
        pkt.insert(pkt.end(), pktLen.begin(), pktLen.end());
        pkt.insert(pkt.end(), addr.begin(), addr.end());

        fRslt_T &= handle_single_rqst_resp(cmd_Info.cmd.at(0), cmd_Info.ack.at(0));

        fRslt = uart_send(pkt.data(), pkt.size());
        // fRslt_T &= uart_send(reinterpret_cast<t_u8*> (pkt.data()), pkt.size());
        curAddr += MAX_READ_DATA_SIZE;
        quotient--;

        // to avoid txBuff(pkt) maintain until tx completed
        // Sleep(10);              // min. 0.5ms to maintain tx buff for 6 bytes at 115200 bps
        addr.clear();
        pkt.clear();

        // printf("@@@ uart_Messages::handle_read_memory_txPkt: quotient= %d\n", quotient);
        rxPkt_Info.len = MAX_READ_DATA_SIZE;
        handle_read_memory_rxPkt(rxPkt_Info);
       
    }
    if (remainder > 0)
    {
        pktLen.clear();
        pktLen = Inline_Lib::convert_int_bytes(remainder, 2);      
        addr = Inline_Lib::convert_int_bytes(curAddr, 4);
        pkt.insert(pkt.end(), pktLen.begin(), pktLen.end());
        pkt.insert(pkt.end(), addr.begin(), addr.end());

        fRslt_T &= handle_single_rqst_resp(cmd_Info.cmd.at(0), cmd_Info.ack.at(0));
        fRslt_T &= uart_send(pkt.data(), pkt.size());   

        rxPkt_Info.len = remainder;
        handle_read_memory_rxPkt(rxPkt_Info);       
    }

    fTxCmpltd = TRUE;

    // printf("@@@ uart_Messages::handle_read_memory_txPkt: end >>> \n");
    return fRslt_T;
}

bool Uart_Messages::handle_read_memory_rxPkt(Pkt_Info &rxPkt_Info)
{
    // printf("@@@ uart_Messages::handle_read_memory_rxPkt: start >>> \n");
    bool    fLoopDone = FALSE;
    t_u16   rxByteNum = 0;
    t_u16   rxByteCnt = 0;              
    t_u16   lenData = rxPkt_Info.len;      //  seperate for CRC16, rx Pkt including the CRC16, but not verified i.e. discard the CRC16
    t_u16   lenCrc16;

    Pkt_Info crc16_Info;
    crc16_Info.len = CRC16_BYTES;            

    fRxCmpltd = FALSE;
    fRxBufFull = FALSE;        

    while (rxPkt_Info.len > rxByteCnt)
    {
        // rxPkt_Info.len += CRC16_BYTES;
        rxByteNum = uart_receive(rxPkt_Info);
        lenCrc16 = uart_receive(crc16_Info);     // discard since not to be used
        // printf("@@@ uart_Messages::handle_read_memory_rxPkt: lenCrc16= %d\n", lenCrc16);

        rxByteCnt += rxByteNum;
        // totalByteNum += (rxByteNum - CRC16_BYTES);
    }
    // fRxCmpltd = TRUE;
    fRxBufFull = TRUE;          // TRUE to update chunkData
    while (!fCDataUpdated)
    {

    }
    rxPkt_Info.data.clear();
    fCDataUpdated = FALSE;   
    // totalRxBytes += rxByteCnt;
    // printf("@@@ uart_Messages::t_read_memory_rxPkt: end >>> \n");

    return 1;

}

// common tx and rx
bool Uart_Messages::handle_read_memory_pkt(Cmd_Info &cmd_Info, t_u16 len, std::vector<t_u8> &addr, Pkt_Info &rxPkt_Info, std::queue<bool> &rsltQ)
{
    bool    fRslt_T = TRUE;
    bool    fRslt = FALSE;
    bool    fLoopDone = FALSE;

    std::vector<t_u8> pktLen;
    pktLen.reserve(PKT_LEN_BYTES);
    pktLen = Inline_Lib::convert_int_bytes(len, 2);

    std::vector<t_u8> pkt;
    pkt.reserve(pktLen.size() + addr.size());

    pkt.insert(pkt.end(), pktLen.begin(), pktLen.end());
    pkt.insert(pkt.end(), addr.begin(), addr.end());

    printf("@@@ uart_Messages::handle_read_memory_pkt: len= %d, pktLen.size= %d,  pkt.size= %d\n", len, pktLen.size(), pkt.size());

    while (not fRslt)
    {
        fRslt = handle_single_rqst_resp(cmd_Info.cmd.at(0), cmd_Info.ack.at(0));

        if (fRslt)
        {
            // rsltQ.push(fRslt);
        }    
    }

    fRslt = FALSE;
    int i = 1;

    while((not fRslt) and (i > 0))
    {
        fRslt = uart_send(reinterpret_cast<t_u8*> (pkt.data()), pkt.size());
        // Sleep(1);      
        i --;
    } 

    t_u16 rxByteNum = 0;
    t_u16 totalByteNum = 0;
    rxPkt_Info.len = len;                      // rx crc16 from target    
    while (not fLoopDone)
    {   
        rxByteNum = uart_receive(rxPkt_Info);
        // printf("@@@ uart_Messages::handle_read_memory_pkt: rxByteNum= %d, array[512+1:rxPkt_Info.data.size]= %d\n", rxByteNum, rxPkt_Info.len);

        // dataLen = Inline_Lib::convert_binary_int(rxPkt_Info.data[0], rxPkt_Info.data[1]);
        totalByteNum += rxByteNum;

        if (totalByteNum == len)
        {
            fLoopDone = TRUE;
            // fRslt = TRUE;
            // printf("@@@ uart_Messages::handle_read_memory_pkt: rxPkt_Info.data[0]= %d, rxPkt_Info.data[1]=%d\n", rxPkt_Info.data.at(0), rxPkt_Info.data.at(1));
        }
    }    

    rsltQ.push(fRslt_T);    
    return fRslt_T;
}

bool Uart_Messages::handle_write_memory_pkt(Cmd_Info &cmd_Info, std::vector<t_u8> &pktBuff, std::vector<t_u8> &crc16, std::queue<bool> &rsltQ)
{
    bool    fRslt_T = TRUE;
    bool    fRslt = FALSE;
    bool    fLoopDone = FALSE;
    
    std::vector<t_u8> pkt;

    // printf("@@@ uart_Messages::handle_write_memory_pkt: pktBuff= %d,  crc16=%d\n", pktBuff.size(), crc16.size());
    pkt.reserve(pktBuff.size() + crc16.size());

    pkt.insert(pkt.end(), pktBuff.begin(), pktBuff.end());
    pkt.insert(pkt.end(), crc16.begin(), crc16.end());

    while (not fRslt)
    {
        fRslt = handle_single_rqst_resp(cmd_Info.cmd.at(0), cmd_Info.ack.at(0));

        if (fRslt)
        {
            // rsltQ.push(fRslt);
        }    
    }
    
    fRslt = FALSE;
    int i = 1;

    while((not fRslt) and (i > 0))
    {
        fRslt = uart_send(reinterpret_cast<t_u8*> (pkt.data()), pkt.size());
        // Sleep(1);      
        i --;
    }
        
    t_u16 rxByteNum = 0;
    t_u16 totalByteNum = 0;
    Pkt_Info rxPkt_Info;
    rxPkt_Info.len = CRC16_BYTES;                      // rx crc16 from target
    while (not fLoopDone)
    {   
        rxByteNum = uart_receive(rxPkt_Info);

        // dataLen = Inline_Lib::convert_binary_int(rxPkt_Info.data[0], rxPkt_Info.data[1]);
        totalByteNum += rxByteNum;


        fLoopDone = TRUE;
        /*
        if (totalByteNum == CRC16_BYTES)
        {
            fLoopDone = TRUE;
            // fRslt = TRUE;
            // printf("@@@ uart_Messages::handle_write_memory_pkt: rxPkt_Info.data[0]= %d, rxPkt_Info.data[1]=%d\n", rxPkt_Info.data[0], rxPkt_Info.data[1]);
        }
        */
    }    

    // crc16.reserve(CRC16_BYTES);
    for (int i = 0; i < CRC16_BYTES; i++)
    {
        // printf("@@@ uart_Messages::handle_write_memory_pkt: crc16.at(%d)= %d, rxPkt_Info.data[%d]=%d\n", i, crc16.at(i), i, rxPkt_Info.data[i]);

        if(crc16.at(i) == rxPkt_Info.data[i])
            fRslt_T &= TRUE;
        else
            fRslt_T = FALSE;
    }
    
    rsltQ.push(fRslt_T);    
    return fRslt_T;
}

t_u16 Uart_Messages::handle_changeable_rxLen_pkt(Cmd_Info &cmd_Info, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info)
{
    bool    fRslt_T = TRUE;
    bool    fLoopDone = FALSE;
    t_u16   totalByteNum = 0;

    fRslt_T &= handle_base_rqst_resp(cmd_Info);

    if (fRslt_T)
    {
        t_u16   dataLen = 0;
        t_u16   rxByteNum = 0;

        rxPkt_Info.len = cmd_Info.rxByteNum;  
        // rxPkt_Info.len += CRC16_BYTES;                      // rxPkt_Info.len = rxedData[0..1] + CRC16_BYTES e.g. 168 + 2
        // rxPkt_Info.len = BUFFER_SIZE;                      // max with BUFFERSIZE or timeout

        while (not fLoopDone)
        {   
            rxByteNum = uart_receive(rxPkt_Info);
            // printf("@@@ uart_Messages::handle_changeable_rxLen_pkt: rx data[0]= %d, data[1]=%d ??????????????????\n", rxPkt_Info.data.at(0), rxPkt_Info.data.at(1));

            dataLen = Inline_Lib::convert_binary_int(rxPkt_Info.data.at(0), rxPkt_Info.data.at(1));
            totalByteNum += rxByteNum;
            printf("@@@ uart_Messages::handle_changeable_rxLen_pkt: totalByteNum= %d, dataLen=%d ??????????????????\n", totalByteNum, dataLen);
            if (totalByteNum >= dataLen)
                fLoopDone = TRUE;
        }
    }
    else
        fRslt_T = 0;

    rsltQ.push(fRslt_T);
    return totalByteNum;
}

bool Uart_Messages::handle_changeable_rxLen_pkt(t_u8 *rqstCmd, t_u8 *respAck, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info)
{
    bool    fRslt_T = TRUE; 

    t_u8    cmdLen = sizeof(rqstCmd)/POINTER_UNIT_BYTES;

    t_u16   dataLen = 2;           // first 2 bytes of the rx payload to get length
    t_u16   rxBytes = 0;

    fRslt_T = handle_base_rqst_resp(rqstCmd, respAck, cmdLen);

    rxBytes = rx_uart_pkt(dataLen, rxPkt_Info);    

    dataLen = Inline_Lib::convert_binary_int(rxPkt_Info.data.at(0), rxPkt_Info.data.at(1));

    printf("@@@ uart_Messages::handle_changeable_rxLen_pkt: dataLen=%d\n", dataLen);

    dataLen += CRC16_BYTES;                      // dataLen = rxedData[0..1] + CRC16_BYTES e.g. 168 + 2
    rxPkt_Info.len = dataLen; 
    rxBytes = rx_uart_pkt(dataLen, rxPkt_Info);    
    // printf("@@@ uart_Messages::handle_changeable_rxLen_pkt: rxPkt_Info.data[0]=%d\n", rxPkt_Info.data[0]);
    // Inline_Lib::convert_charAry_strAry(rxPkt_Info.data, 2);

    rsltQ.push(fRslt_T);
    return fRslt_T;
}

bool Uart_Messages::handle_fixed_rxLen_pkt(Cmd_Info &cmd_Info, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info)
{
    bool    fRslt_T = TRUE; 
    bool    fLoopDone = FALSE;

    fRslt_T &= handle_base_rqst_resp(cmd_Info);

    t_u16 dataLen = 0;
    t_u16 rxByteNum = 0;
    t_u16 totalByteNum = 0;
    rxPkt_Info.len = cmd_Info.rxByteNum;                      // max with BUFFERSIZE or timeout

    if (rxPkt_Info.len > 0)
    {
        while (not fLoopDone)
        {   
            rxByteNum = uart_receive(rxPkt_Info);
            totalByteNum += rxByteNum;
            // printf("@@@ uart_Messages::handle_changeable_rxLen_pkt: totalByteNum= %d, dataLen=%d\n", totalByteNum, dataLen);
            if (totalByteNum >= rxPkt_Info.len)
                fLoopDone = TRUE;
        }
    }
    else
        fLoopDone = TRUE;

    rsltQ.push(fRslt_T);

    return fLoopDone;
}

bool Uart_Messages::handle_fixed_rxLen_pkt(t_u8 *rqstCmd, t_u8 *respAck, t_u16 dataLen, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info)
{
    bool    fRslt_T = TRUE; 
    t_u16   rxBytes = 0;

    fRslt_T = handle_base_rqst_resp(rqstCmd, respAck, 2);

    if (dataLen > 0)
    {   
        rxBytes = rx_uart_pkt(dataLen, rxPkt_Info);    
    }
    printf("@@@ uart_Messages::handle_fixed_rxLen_pkt: rxPkt_Info.data[0]=%d\n", rxPkt_Info.data.at(0));
    // Inline_Lib::convert_charAry_strAry(rxPkt_Info.data, dataLen);

    rsltQ.push(fRslt_T);
    return fRslt_T;
}

bool Uart_Messages::handle_connect_pkt(std::vector<t_u8> &cmd, std::vector<t_u8> &ack)
{
    bool    fRslt_T = TRUE;
    t_u8    rxByte;

    this->fBlReady = FALSE;
    while(not this->fBlReady)
    {
        uart_sendEx(cmd.data(), 1);

        rxByte = uart_receive_abyte(1);            // critical time delay in the function()
        // printf("@@@ uart_Messages::handle_connect_pkt: rxByte=%d, fRslt_T(TRUE) = %d\n", rxByte, fRslt_T);

        if (rxByte == ack.at(0))
        {
            this->fBlReady = TRUE;
        }
    }

    if (this->fBlReady)
    {
        fRslt_T &= handle_single_rqst_resp(cmd.at(1), ack.at(1));

        if (fRslt_T)
        {
            // rsltQ.push(fRslt);
        }    
    }
    // printf("@@@ uart_Messages::handle_connect_pkt: rxByte=%d, fRslt_T(TRUE) = %d\n", rxByte, fRslt_T);

    return fRslt_T;
}

///////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////

bool Uart_Messages::rqst_read_memory()
{
    bool    fRslt = FALSE;

    // struct  Cmd_Info    cmd_Info;
    struct  Cmd_Info    cmd_Info;

    cmd_Info.cmdByteNum = 1;
    cmd_Info.ackByteNum = 1;
    cmd_Info.rxByteNum = (t_u16)MAX_READ_DATA_SIZE;         // payload bytes to be rxed = 0

    cmd_Info.cmd.push_back(READ_MEMORY_1);
    cmd_Info.ack.push_back(READ_MEMORY_1_RE);

    while (not fRslt)
    {
        fRslt = handle_single_rqst_resp(cmd_Info.cmd.at(0), cmd_Info.ack.at(0));

        if (fRslt)
        {
            // rsltQ.push(fRslt);
        }    
    }

    // printf("@@@ uart_Messages::rqst_read_memory: respMask at rtn_Info.rxPkt_Info.data.at(0) = %d\n", rtn_Info.rxPkt_Info.data.at(0));

    return fRslt;
}

bool Uart_Messages::rqst_write_memory(Rtn_Info &rtn_Info, std::vector<t_u8> &pktBuff)
{
    bool    fRslt_T = TRUE;
    bool    fChk = FALSE;
    t_u16   rxByteNum = 0;
    t_u16   calCrc16 = 0;
    std::vector<t_u8> crc16;
    crc16.reserve(CRC16_BYTES);

    struct  Cmd_Info    cmd_Info;

    cmd_Info.cmdByteNum = 1;
    cmd_Info.ackByteNum = 1;
    cmd_Info.rxByteNum = 0;         // payload bytes to be rxed = 0

    calCrc16 = Inline_Lib::generate_crc16(reinterpret_cast<t_u8*> (pktBuff.data()), pktBuff.size());        // cast from <byte> to <t_u8>
    crc16 = Inline_Lib::convert_int_bytes(calCrc16, 2);

    cmd_Info.cmd.push_back(WRITE_MEMORY_1);
    cmd_Info.ack.push_back(WRITE_MEMORY_1_RE);

    rtn_Info.status = FALSE;
    rtn_Info.rslt = FALSE;

    std::queue<bool>  rsltQ;    
    rtn_Info = {};          // initialize with 0 

    t_u16 i = 1;
    
    while ((not fChk) and (i > 0))     // replace to use timeout
    {
        i--;
        fRslt_T &= handle_write_memory_pkt(cmd_Info, pktBuff, crc16, rsltQ);
       
        fChk = rsltQ.front();
        rsltQ.pop();
    }

    // printf("@@@ uart_Messages::rqst_tgt_addr_info: respMask at rtn_Info.rxPkt_Info.data[1] = %d\n", rtn_Info.rxPkt_Info.data[1]);

    rtn_Info.status = TRUE;
    rtn_Info.rslt = TRUE;

    return fRslt_T;
}

bool Uart_Messages::rqst_erase_flash_memory(Rtn_Info &rtn_Info, t_u8 rqstMask, t_u8 respMask)
{
    bool    fRslt = TRUE;
    bool    fChk = FALSE;
    t_u16   rxByteNum = 0;

    struct  Cmd_Info    cmd_Info;
    cmd_Info.cmdByteNum = 3;
    cmd_Info.ackByteNum = 3;
    cmd_Info.rxByteNum = 0;         // payload bytes to be rxed = 0

    cmd_Info.cmd = {ERASE_FLASH_1, rqstMask, ERASE_FLASH_2};
    cmd_Info.ack = {ERASE_FLASH_1_RE, respMask, ERASE_FLASH_SUCCESS_RE};

    rtn_Info.status = FALSE;
    rtn_Info.rslt = FALSE;

    std::queue<bool>  rsltQ;
    rtn_Info = {};          // initialize with 0 

    t_u16   i = 5;
    while ((not fChk) and (i != 0))     // replace to use timeout
    {
        i--;
        fRslt = handle_fixed_rxLen_pkt(cmd_Info, rsltQ, rtn_Info.pkt_Info);

        fChk = rsltQ.front();
        rsltQ.pop();
    }

    rtn_Info.status = TRUE;
    rtn_Info.rslt = TRUE;

    return fRslt;
}

bool Uart_Messages::rqst_last_non_blank_flash_addr(Rtn_Info &rtn_Info)
{
    bool    fRslt_T = TRUE;
    bool    fChk = FALSE;
    t_u16   rxByteNum = 0;

    struct  Cmd_Info    cmd_Info;
    cmd_Info.cmdByteNum = 2;
    cmd_Info.ackByteNum = 2;
    cmd_Info.rxByteNum = PKT_ADDR_BYTES;         // addr info, payload bytes to be rxed = 4

    cmd_Info.cmd = {LAST_USED_FLASH, LAST_USED_FLASH_2};
    cmd_Info.ack = {LAST_USED_FLASH_RE, LAST_USED_FLASH_2_RE};

    rtn_Info.status = FALSE;
    rtn_Info.rslt = FALSE;

    std::queue<bool>  rsltQ;
    rtn_Info = {};          // initialize with 0 

    t_u16   i = 5;
    while ((not fChk) and (i != 0))     // replace to use timeout
    {
        i--;
        fRslt_T &= handle_fixed_rxLen_pkt(cmd_Info, rsltQ, rtn_Info.pkt_Info);

        fChk = rsltQ.front();
        rsltQ.pop();
    }

    printf("@@@ uart_Messages::rqst_tgt_addr_info: rtn_Info.rxPkt_Info.len = %d\n", rtn_Info.pkt_Info.len);

    t_u8    chksum = 0;
    for (int i = 0; i < cmd_Info.rxByteNum; i++)
    {
        chksum += rtn_Info.pkt_Info.data.at(i);
    }

    chksum &= 0xff;
    
    t_u8    len = 1;

    fRslt_T &= handle_base_rqst_resp(&chksum, &chksum, len);

    fRslt_T &= parse_last_writtenAddr(&(rtn_Info.pkt_Info.data[0]));

    rtn_Info.status = TRUE;
    rtn_Info.rslt = TRUE;

    return fRslt_T;
}

bool Uart_Messages::rqst_tgt_addr_info(Rtn_Info &rtn_Info, t_u32 blVer)
{
    bool    fRslt_T = TRUE;
    bool    fChk = FALSE;
    t_u16   rxByteNum = 0;

    struct  Cmd_Info    cmd_Info;
    cmd_Info.cmdByteNum = 2;
    cmd_Info.ackByteNum = 2;
    if (blVer == BL_VER_1)
        cmd_Info.rxByteNum = 168 + 4;           // 2(len) + 168(payload) + 2(crc16)
    else if (blVer == BL_VER_2)
        cmd_Info.rxByteNum = 260 + 4;           // 2(len) + 264(payload) + 2(crc16)

    cmd_Info.cmd = {ADDRESS_INFO_REQ, ADDRESS_INFO_REQ_2};
    cmd_Info.ack = {ADDRESS_INFO_REQ_RE, ADDRESS_INFO_REQ_2_RE};

    rtn_Info.status = FALSE;
    rtn_Info.rslt = FALSE;

    std::queue<bool>  rsltQ;
    rtn_Info = {};          // initialize with 0 

    t_u16   i = 5;
    while ((not fChk) and (i != 0))     // replace to use timeout
    {
        i--;
        rxByteNum += handle_changeable_rxLen_pkt(cmd_Info, rsltQ, rtn_Info.pkt_Info);

        fChk = rsltQ.front();
        rsltQ.pop();
    } 
    printf("@@@ uart_Messages::rqst_tgt_addr_info: >>>>>>>>>>>>>>>>>>>>>>\n");

    rtn_Info.pkt_Info.len = rxByteNum;

    if (blVer == BL_VER_1)
        fRslt_T &= parse_addr_infoR1(rtn_Info.pkt_Info);       // rxPkt_Info.len = 172 bytes i.e. len (2 bytes, val= 168) + data (168 bytes) + crc16 (2 bytes)
    else if (blVer == BL_VER_2)
        fRslt_T &= parse_addr_infoR2(rtn_Info.pkt_Info);

    rtn_Info.status = TRUE;
    rtn_Info.rslt = TRUE;

    return fRslt_T;
}

bool Uart_Messages::rqst_buffer_size(Rtn_Info &rtn_Info)
{
    bool    fRslt_T = TRUE; 
    bool    fChk = FALSE;

    struct  Cmd_Info    cmd_Info;
    cmd_Info.cmdByteNum = 2;
    cmd_Info.ackByteNum = 2;
    cmd_Info.rxByteNum = PKT_LEN_BYTES;         // buffer size, payload bytes to be rxed = 2
    cmd_Info.cmd = {BUFFER_CHK_1, BUFFER_CHK_2};
    cmd_Info.ack = {BUFFER_CHK_1_RE, BUFFER_CHK_2_RE};

    rtn_Info.status = FALSE;
    rtn_Info.rslt = FALSE;

    std::queue<bool>  rsltQ;
    rtn_Info = {};          // initialize with 0 

    t_u16   i = 5;
    while ((not fChk) and (i != 0))     // replace to use timeout
    {
        i--;
        fRslt_T &= handle_fixed_rxLen_pkt(cmd_Info, rsltQ, rtn_Info.pkt_Info);
        fChk = rsltQ.front();
        rsltQ.pop();
    }

    rtn_Info.pkt_Info.len = PKT_LEN_BYTES;
    printf("@@@ uart_Messages::rqst_buffer_size: rtn_Info.rxPkt_Info.data = %d\n", rtn_Info.pkt_Info.data.at(1));

    t_u8    chksum = rtn_Info.pkt_Info.data.at(1);  // 2 bytes Buffer Size (514 bytes) that target txed
                                                    // High byte first. i.e. chksum not (0x100 - sum) but just with low byte only
    t_u8    len = 1;

    fRslt_T &= handle_base_rqst_resp(&chksum, &chksum, len);
    fRslt_T &= parse_buff_size(&(rtn_Info.pkt_Info.data[0]));

    rtn_Info.status = TRUE;
    rtn_Info.rslt = TRUE;

    return fRslt_T;
}

bool Uart_Messages::rqst_bootload_connecting(Rtn_Info &rtn_Info)
{
    bool    fRslt = FALSE; 
    bool    fChk = FALSE;

    rtn_Info.status = TRUE;
    rtn_Info.pwrCycle = TRUE;

    std::vector<t_u8>   cmd{CONNECT1, CONNECT2};
    std::vector<t_u8>   ack{CONNECT1_RE, CONNECT2_RE};

    fRslt = handle_connect_pkt(cmd, ack);

    rtn_Info.status = TRUE;
    rtn_Info.pwrCycle = TRUE;

    return fRslt;
}

void Uart_Messages::set_flag_tx_completed(bool flag)
{
    fTxCmpltd = flag;
}

void Uart_Messages::set_flag_rx_full(bool flag)
{
    fRxBufFull = flag;
    // printf("@@@ uart_Messages::set_flag_rx_full: fRxBufFull= %d\n", fRxBufFull);
}

void Uart_Messages::set_flag_rx_completed(bool flag)
{
    fRxCmpltd = flag;
}

t_u8 Uart_Messages::get_bootloader_ver()
{
    blVer = 1;
    return blVer;
}

t_u8* Uart_Messages::get_last_written_addr()
{
    memcpy(lastWrittenAddr, m_addr_InfoC.lastWrittenAddr, 4);
    return lastWrittenAddr;
}

bool Uart_Messages::get_flag_tx_completed()
{
    return fTxCmpltd;
}

bool Uart_Messages::get_flag_rx_full()
{
    return fRxBufFull;
}

bool Uart_Messages::get_flag_rx_completed()
{
    return fRxCmpltd;
}

Uart_Messages::Uart_Messages()
{
    fBlReady = FALSE;

    fTxCmpltd = FALSE;
    fRxBufFull = FALSE;
    fRxCmpltd = FALSE;
    fCDataUpdated = FALSE;

    handShakeCnt = 0;
    totalRxBytes = 0;

    memset(lastWrittenAddr, 0xff, 4);
}

void threadFunc()
{
    std::cout << "Current Time :: ";
    current_time_point(std::chrono::system_clock::now());
  
    std::chrono::system_clock::time_point timePt =
            std::chrono::system_clock::now() + std::chrono::seconds(60);
  
    std::cout << "Sleeping Until :: "; 
    current_time_point(timePt);
  
    std::this_thread::sleep_until(timePt);
  
    std::cout << "Woke up...Current Time :: ";
    current_time_point(std::chrono::system_clock::now());
}

