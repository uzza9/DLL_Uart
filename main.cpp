// g++   *.cpp -o uart_test -pthread

#include    "uart_messages.h"
#include    "parse_Messages.h"

#include    <algorithm>
#include    <map>
#include    <thread>
#include    <functional>           // for std::ref

#ifdef __cplusplus
 #include   <cstdio>     // C++ only
 using std::printf;
#else
 #include   <stdio.h>    // C only
#endif

#define elif        else if

std::vector<t_u8> chunkData;

// void current_time_point_main(time_t timePt);
bool read_data_packet(t_u32 sAddr, t_u32 eAddr);
bool write_data_packet();

bool read_data_packet(t_u32 sAddr, t_u32 eAddr)
// bool read_data_packet(b'8000000', b'a0007fff')     
{
    bool            fRslt_T = TRUE;
    t_u32           dataSize = eAddr - sAddr + 1;
    Uart_Messages   uart_Msg;

    chunkData.reserve(dataSize);

    Pkt_Info        rxPkt_Info;
    rxPkt_Info.len  = MAX_READ_DATA_SIZE;

    bool fTxCmpltd  = FALSE;
    bool fRxBufFull = FALSE;
    bool fRxCmpltd  = FALSE;

    std::thread tChunkData(&Uart_Messages::handle_read_memory_chunk_data, &uart_Msg, std::ref(chunkData), std::ref(rxPkt_Info), dataSize);
    fRslt_T = uart_Msg.handle_read_memory_txPkt(sAddr, eAddr, rxPkt_Info);
   
    while(!fRxCmpltd)
    {
        fRxCmpltd = uart_Msg.get_flag_rx_completed();
    }
    tChunkData.join();

    return fRslt_T;
}

bool write_data_packet()
{
    Uart_Messages           uart_Msg;
    Rtn_Info                rtn_Info;

    bool                    fRslt_T = TRUE;

    std::ifstream           hexfile("D1405_712K.hex", std::ios::binary);
    // std::ifstream           hexfile("D1405_486K.hex", std::ios::binary);


    std::vector<byte>       pktBuff;
    pktBuff.reserve(PKT_LEN_BYTES + PKT_ADDR_BYTES + BUFFER_SIZE + 1);       // CRC16_BYTES to be added at rqst_write_memory_pkt()

    if(hexfile)
    {
        Parse_Messages      parse_Msg;
        std::string         str;
        std::vector<byte>   lineBuff;
        lineBuff.reserve(BUFFER_SIZE);               

        std::vector<byte>   dataBuff;

        std::vector<byte>   pktLen;
        pktLen.reserve(PKT_LEN_BYTES);

        std::vector<byte>   pktAddr;
        // std::vector<std::vector<byte>> pktAddr;
        pktAddr.reserve(PKT_ADDR_BYTES);

        bool                fValidEla = FALSE;
        bool                fEof = FALSE;

        t_u16               len = 0;
        t_u16               recType = 0;
        t_u16               pktSize = 0;

        t_u16               dataSize = 0;

        while(!fEof)
        {
            std::getline(hexfile, str);
            std::copy(str.begin(), str.end() - 1, std::back_inserter(lineBuff));            // -1 to discard CR in str

            parse_Msg.parse_line(lineBuff);                            
            lineBuff.clear();           // vector size = 0

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            len = parse_Msg.get_data_len();         // data index i.e. accumulated num ==> should be managed based on index

            recType = parse_Msg.get_rec_type();
            fValidEla = parse_Msg.get_ela_status();

            if ((len == BUFFER_SIZE) or (recType == IHEX_TYPE::EOFILE) or (fValidEla == TRUE))
            {                      
                pktLen = Inline_Lib::convert_int_bytes(len + CRC16_BYTES, 2);           // pktLen incl. CRC16_BYTES (2 bytes) e.g len = 514 = 512 + 2

                pktAddr = parse_Msg.get_mem_addr();                        
                dataBuff = parse_Msg.get_data_chunk();

                pktBuff.insert(pktBuff.end(), pktLen.begin(), pktLen.end());
                pktBuff.insert(pktBuff.end(), pktAddr.begin(), pktAddr.end());
                pktBuff.insert(pktBuff.end(), dataBuff.begin(), dataBuff.end());

                fRslt_T &= uart_Msg.rqst_write_memory(rtn_Info, pktBuff);
                pktLen.clear();
                pktAddr.clear();
                pktBuff.clear();           // vector size = 0
                parse_Msg.reset_data_len();
                dataSize = 0;
                fEof = parse_Msg.get_eof_status();
            }
        }
    }
    return fRslt_T;
}


int main(int argc, char *argv[]) 
{  
    Uart_Messages   uart_Msg;
    Tgt_Info        tgt_Info;
    Rtn_Info        rtn_Info;

    t_u32           blVer;

    bool fRslt      = FALSE;
    bool fRslt_T    = TRUE;

    // rslt = init_uart_test();
    fRslt = init_uart();
    fRslt = config_port();

    bool flagTst = TRUE;

    t_u8 *cmdBuff;
    t_u8 *respBuff;

    t_u8 cmd[3] = {0};
    t_u8 resp[3] = {0};

    cmd[0] = CONNECT1;
    resp[0] = CONNECT1_RE;

    cmd[1] = CONNECT2;
    resp[1] = CONNECT2_RE;  

    printf("@@@ main::main: rtn_Info.status= %d,  before >>>  rtn_Info.pwrCycle= %d >>>>>>>>>>\n", rtn_Info.status, rtn_Info.status);

    if (*argv[1] == '1')
    {
        Rtn_Info rtn_Info = {};          // initialize with 0 
        printf("@@@ main::main: cin= 1 >>> calling  rqst_bootload_connecting() \n");
        fRslt_T &= uart_Msg.rqst_bootload_connecting(rtn_Info);

        // connecting with thread for test purpose !!! 
        // fRslt_T &= uart_Msg.rqst_bootload_connecting_thread(rtn_Info);
    }
    elif (*argv[1] == '2')
    {
        if (*argv[2] == '1')
            blVer = 1;
        
        else if (*argv[2] == '2')
            blVer = 2;
        else
            blVer = 1;

        printf("@@@ main::main: cin= 2 >>> calling  rqst_buffer_size/ addr info / last written addr () \n");   

        fRslt_T &= uart_Msg.rqst_buffer_size(rtn_Info);
        if (fRslt_T)
        {
            fRslt_T &= uart_Msg.rqst_tgt_addr_info(rtn_Info, blVer);
        }
        if (fRslt_T)
        {
            fRslt_T &= uart_Msg.rqst_last_non_blank_flash_addr(rtn_Info);
        }
    }    
    elif (*argv[1] == '3')
    {
        if (*argv[2] == '1')
            blVer = 1;
        
        else if (*argv[2] == '2')
            blVer = 2;
        else
            blVer = 1;        
        if (1)
        {
            printf("@@@ main::main: cin= 3 >>> calling  rqst_erase_flash_memory() \n");   
            fRslt_T &= uart_Msg.rqst_erase_flash_memory(rtn_Info, ERASE_FLASH_MASK, ERASE_FLASH_MASK_RE);
        }
        if (1)
        {
            timestamp_start("@@@ main::main: Current Time at starting WRITE MEMORY >>> ");

            fRslt_T = write_data_packet();

            timestamp_end("@@@ main::main: Current Time at completing WRITE MEMORY >>> ");
            timestamp_elapsed("@@@ main::main: Elapsede time for WRITE MEMORY >>> ");        
        }
        
        if (1)
        {
            timestamp_start("@@@ main::main: Current Time at starting READ MEMORY >>> ");      

            // as if self.read_data_packet(sAddr, eAddr)        // Addr_Info managing address holes 
                                                                //bet. sAddr(base + offset: 0x800 0000) and eAddr (base + offset: 0xa000 7fff)
            // 0x800 0000 ~ 0x800 ffff
            // 0x801 0000 ~ 0x801 ffff
            // 0x802 0000 ~ 0x802 3253
            // 0xa000 0000 ~ 0xa000 7fff   
            // office
                                                             
            fRslt_T &= read_data_packet(0x8004000, 0x800ffff);
            fRslt_T &= read_data_packet(0x8010000, 0x801ffff);
            
            // fRslt_T &= read_data_packet(0x8020000, 0x8023253);
            fRslt_T &= read_data_packet(0x8020000, 0x802ffff);
            fRslt_T &= read_data_packet(0x8030000, 0x80393ff);

            fRslt_T &= read_data_packet(0xa0000000, 0xa0007fff);

            // home
            /*
            fRslt_T &= read_data_packet(0x80002000, 0x8000ffff);
            fRslt_T &= read_data_packet(0x80010000, 0x8001ffff);
            fRslt_T &= read_data_packet(0x80020000, 0x8002ffff);
            fRslt_T &= read_data_packet(0x90000000, 0x90007fff);
            */
          
            timestamp_end("@@@ main::main: Current Time at completing READ MEMORY >>> ");
            timestamp_elapsed("@@@ main::main: Elapsede time for READ MEMORY >>> ");    
        }
    }
    printf("@@@ main::main: rtn_Info.status= %d,  after >>>  rtn_Info.pwrCycle= %d >>>>>>>>>>\n", rtn_Info.status, rtn_Info.status);

    return 0;
}

