#ifndef                 PARSE_MESSAGES_H
#define                 PARSE_MESSAGES_H

#include                "cmn_cfg.h"
#include                "cmn_lib.h"
#include                <iterator>

#if defined(__cplusplus)
extern "C" {
#endif

#define                 HEX_BUFFER_SIZE     255

struct Ihex_Info {
    t_u16               type;
    t_u16               len;
    std::vector<t_u8>   addr;
    std::vector<t_u8>   data;
    t_u8                chksum;
    bool                eof;
    bool                success;             // Only set true after checking the EOF, the checksums etc.
};

// enum for types
enum IHEX_TYPE {
    DATA                = 0, 
    EOFILE              = 1, 
    ESA                 = 2,        // EXTENDED_SEGMENT_ADDRESS        
    SSA                 = 3,        // START_SEGMENT_ADDRESS
    ELA                 = 4,        // EXTENDED_LINEAR_ADDRESS 
    SLA                 = 5         // START_LINEAR_ADDRESS
}; 

// bit position of the record line
enum class IHEX_RECORD {
    DELIMTR_COL         = 0,        // ':' at the stating of a line
    LEN_MSB             = 1, 
    LEN_LSB             = 2, 
    ADDR_MSB            = 3, 
    ADDR_MSB2           = 4, 
    ADDR_LSB2           = 5, 
    ADDR_LSB            = 6,
    TYPE_MSB            = 7, 
    TYPE_LSB            = 8, 
    DATA_START          = 9
};


class Parse_Messages
{
    private:
        t_u16                           dataIdx = 0;
        std::vector<byte>               baseAddr;
        std::vector<byte>               offsetAddr;
        std::vector<byte>               memAddr;
        // std::vector<std::vector<byte>>  memAddr;
        std::vector<byte>               dataChunk;
        t_u16                           cntEla;
        bool                            fValidEla;
        bool                            fEof;

        Ihex_Info                       ihex_Info;

        // IHEX_RECORD IHEX_REC = IHEX_RECORD::DELIMTR_COL;        // starting of the enum class as reference

        t_u16                           extract_data_len(std::vector<byte> const &line);
        void                            extract_base_addr(std::vector<byte> const &line);
        void                            extract_offset_addr(std::vector<byte> const &line);       
        t_u16                           extract_rec_type(std::vector<byte> const &line);
        void                            extract_data_chunk(std::vector<byte> const &line, t_u16 len);

        void                            handle_eof();

    public:
        void                            parse_line(std::vector<byte> const &line);

        t_u16                           get_ela_status();
        t_u16                           get_rec_type();
        t_u16                           get_data_len();
        std::vector<byte>               get_mem_addr();
        // std::vector<std::vector<byte>>  get_mem_addr();
        std::vector<byte>               get_data_chunk();
        bool                            get_eof_status();
        void                            reset_data_len();

        std::vector<byte>               test(t_u16 intVal, t_u16 num);
        

        Parse_Messages();
        virtual ~Parse_Messages() {};
};


#if defined(__cplusplus)
}
#endif      // end (__cplusplus)

#endif      // end PARSE_MESSAGES_H