#ifndef                     UART_MESSAGES_H
#define                     UART_MESSAGES_H

#include                    "cmn_cfg.h"
#include                    "protocol_msg.h"
#include                    "uart_if.h"
#include                    "cmn_lib.h"

#include                    <memory>
#include                    <thread>
#include                    <functional>           // for std::ref


#include                    <mutex>
#include                    <atomic>


#if defined(__cplusplus)
extern "C" {
#endif


class Uart_Messages
{
private:

    static bool             fBlReady;

    std::mutex              mtx;

    // std::atomic<bool>    fTxCmpltd;
    // std::atomic<bool>    fRxBufFull;
    // std::atomic<bool>    fRxCmpltd;

    bool                    fTxCmpltd;
    bool                    fRxBufFull;
    bool                    fRxCmpltd;    
    bool                    fCDataUpdated;

    t_u16                   handShakeCnt;
    t_u32                   totalRxBytes;
      
    t_u8                    *lastWrittenAddr =  new t_u8[4];

    struct Protocol_Info_R1
    {
        t_u16               lenCmd;
        std::vector<t_u8>   rqstCmd;
        t_u16               lenAck;
        std::vector<t_u8>   respAck;       
    };

    struct Protocol_Info
    {
        t_u16               lenCmd;
        t_u8                *rqstCmd;
        t_u16               lenAck;
        t_u8                *respAck;       
    };    


    void                    initialize();
    void                    clear_addr_infoR1();
    void                    clear_addr_infoR2();
    void                    mapping_to_addr_infoC();

    bool                    parse_addr_infoR1(Pkt_Info &rxPkt_Info);
    bool                    parse_addr_infoR2(Pkt_Info &rxPkt_Info);

    bool                    parse_last_writtenAddr(t_u8 *msg);
    bool                    parse_buff_size(t_u8 *msg);

    void                    convert_int_bytes_R2(t_u32 intVal, t_u16 num, t_u8 (&rtn)[4]);

    bool                    rx_uart_pkt(t_u16 msgLen, Pkt_Info &rxPkt_Info);

    bool                    handle_single_rqst_resp(t_u8 cmd, t_u8 ack);
    bool                    handle_single_rqst_resp(t_u8 *cmd, t_u8 *ack);

    bool                    handle_base_rqst_resp(Cmd_Info &cmd_Info);
    bool                    handle_base_rqst_resp(t_u8 *cmd, t_u8 *ack, t_u16 len);

    bool                    handle_read_memory_pkt(Cmd_Info &cmd_Info,  t_u16 len, std::vector<t_u8> &addr, Pkt_Info &rxPkt_Info, std::queue<bool> &rsltQ);
    bool                    handle_write_memory_pkt(Cmd_Info &cmd_Info, std::vector<t_u8> &pktBuff, std::vector<t_u8> &crc16, std::queue<bool> &rsltQ);

    t_u16                   handle_changeable_rxLen_pkt(Cmd_Info &cmd_Info, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info);
    bool                    handle_changeable_rxLen_pkt(t_u8 *rqstCmd, t_u8 *respAck, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info);

    bool                    handle_fixed_rxLen_pkt(Cmd_Info &cmd_Info, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info);
    bool                    handle_fixed_rxLen_pkt(t_u8 *rqstCmd, t_u8 *respAck, t_u16 msgLen, std::queue<bool> &rsltQ, Pkt_Info &rxPkt_Info);

    bool                    handle_connect_pkt(std::vector<t_u8> &rqstCmd, std::vector<t_u8> &respAck);

public:
    Addr_InfoR1             m_addr_InfoR1;  
    Addr_InfoR2             m_addr_InfoR2;
    // R1, R2, R3, ... mapping to C as common Addr_Info
    // Addr_InfoC if necessary e.g. infoR_lower is not subset of InfoR_upper
    Addr_InfoR2             m_addr_InfoC;       

    Tgt_Info                tgt_Info;
    t_u32                   blVer;
    
    // void                 check_timeout();

    ///////////////////////////////////////////////////
    void                    handle_read_memory_chunk_data(std::vector<t_u8> &chunkData, Pkt_Info &rxPkt_Info, t_u32 dataSize);
    bool                    handle_read_memory_txPkt(t_u32 sAddr, t_u32 eAddr, Pkt_Info &rxPkt_Info);   // to public
    bool                    handle_read_memory_rxPkt(Pkt_Info &rxPkt_Info);          // to public


    /////////////////////////////////////////////////////////////////
    void                    t_read_memory_rxPkt(Pkt_Info &rxPkt_Info);
    void                    t_read_memory_txPkt(t_u32 sAddr, t_u32 eAddr);

    static                  DWORD WINAPI t_read_memory_rxPkt_R0(Pkt_Info &rxPkt_Info);
    static                  DWORD WINAPI t_connect_pkt(void *proto_Info);
    bool                    t_rqst_bootload_connecting(Rtn_Info &rtn_Info);
    /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    // void                 set_SN(t_u32 sn1, t_32 sn2);
    // void                 set_desc(t_u8 *desc)

    ///////////////////////////////////////

    //  only CMD[0] and ACK[0], rqst_read_memory_(tx/rx)Pkt handling pkt with thread 
    bool                    rqst_read_memory();

    bool                    rqst_write_memory(Rtn_Info &rtn_Info, std::vector<t_u8> &pktBuff);
    bool                    rqst_erase_flash_memory(Rtn_Info &rtn_Info, t_u8 rqstMask, t_u8 respMask);
    bool                    rqst_last_non_blank_flash_addr(Rtn_Info &rtn_Info);
    bool                    rqst_tgt_addr_info(Rtn_Info &rtn_Info, t_u32 blVer);  
    bool                    rqst_buffer_size(Rtn_Info &rtn_Info);  
    bool                    rqst_bootload_connecting(Rtn_Info &rtn_Info);


    void                    set_flag_tx_completed(bool flag);   
    void                    set_flag_rx_full(bool flag);
    void                    set_flag_rx_completed(bool flag);   

    t_u8                    get_bootloader_ver();
    t_u8*                   get_last_written_addr();
    bool                    get_flag_tx_completed();
    bool                    get_flag_rx_full();
    bool                    get_flag_rx_completed();

    Uart_Messages();
    virtual ~Uart_Messages() {delete[] lastWrittenAddr;};

};

void                        current_time_point(std::chrono::system_clock::time_point timePt);


#if defined(__cplusplus)
}
#endif      // end (__cplusplus)

#endif      // end UART_MESSAGES_H