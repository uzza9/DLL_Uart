/* --------------------------------------------------------------------------- */
// Name:         parse_messages
// Purpose:      parse intel hex file
// Revision:     rev_1.0
// Author:       C Jeon
// Created:      05/15/2021
// last revision:06/07/2021
/* ---------------------------------------------------------------------------*/

#include "parse_messages.h"

t_u16 Parse_Messages::extract_data_len(std::vector<byte> const &line)
{
    t_u16   len = 0;
    char    buff[3];

    buff[0] = line[1];          // IHEX_REC::LEN_MSB
    buff[1] = line[2];
    buff[2] = DELIMITER;

    // for better performance with less exception
    len = strtol(buff, 0, 16);

    /*
    try
    {
        len = std::stoi(buff);
    }
    catch(...)
    {
        printf("@@@ Parse_Messages::extract_data_len: Invalid length !!!\n");
    }
    */
    // printf("@@@ Parse_Messages::extract_data_len: len= %d <<< len[0]= %d, len[1]= %d\n", len, buff[0], buff[1]);
    return len;
}

t_u16 Parse_Messages::extract_rec_type(std::vector<byte> const &line)
{
    t_u16   type;
    char    buff[3];

    buff[0] = line[7];
    buff[1] = line[8];
    buff[2] = DELIMITER;

    // for better performance with less exception
    type = strtol(buff, 0, 16);
    /*
    try
    {
        type = std::stoi(buff);
    }
    catch(...)
    {
        printf("@@@ Parse_Messages::extract_rec_type: Invalid type !!!\n");
    }
    */
    // printf("@@@ Parse_Messages::extract_rec_type: type= %d <<< type[0]= %d, type[1]= %d\n", type, buff[0], buff[1]);
    return type;
}

void Parse_Messages::extract_base_addr(std::vector<byte> const &line)
{
    t_u32   addr;
    char    buff[5];

    for (int i = 0; i < 4;  i++)
        buff[i] = line[i + 9];
    buff[5] = DELIMITER;

    addr = strtol(buff, 0, 16);
    baseAddr = Inline_Lib::convert_int_bytes(addr, 2);
    // printf("@@@ Parse_Messages::extract_base_ddr: baseAddr= %d <<< baseAddr[0]= %d, baseAddr[1]= %d\n", addr, baseAddr.at(0), baseAddr.at(1));
}


void Parse_Messages::extract_offset_addr(std::vector<byte> const &line)
{
    t_u32   addr;
    char    buff[5];

    for (int i = 0; i < 4;  i++)
        buff[i] = line[i + 3];
    buff[5] = DELIMITER;

    addr = strtol(buff, 0, 16);
    offsetAddr.clear();
    offsetAddr = Inline_Lib::convert_int_bytes(addr, 2);
    // printf("@@@ Parse_Messages::extract_offset_addr: offsetAddr= %d <<< offsetAddr[0]= %d, offsetAddr[1]= %d\n", addr, offsetAddr.at(0), offsetAddr.at(1));
}

void Parse_Messages::extract_data_chunk(std::vector<byte> const &line, t_u16 len)
{
    t_u16   start = 9;
    t_u16   end = start + len * 2;      // 2 bytes (2 byte-type array) per one data e.g. 0x30 0x30 for 00
    char    buff[3];
    t_u16   tmp;

    buff[2] = DELIMITER;

	for(int i = start; i < end ; i = i+2)
	{
		buff[0] = line[i];
		buff[1] = line[i+1];

		tmp = strtol(buff, 0, 16);       // compare performance which better to use std::stoi() or strtol()
        dataChunk.insert(dataChunk.end(), tmp);
        // printf("@@@ Parse_Messages::: dataChunk[%d]= %d, buff[0]= %d, buff[1]= %d\n", dataIdx, dataChunk[dataIdx], buff[0], buff[1]);
        // SleepEx(1000, TRUE);
		dataIdx++;                  // keeping track of chunk size
	} 

}

void Parse_Messages::handle_eof()
{
    fEof = TRUE;
    printf("Parse_Messages::handle_eof: >>>>>>>>>>>>\n");
}

void Parse_Messages::parse_line(std::vector<byte> const &line)
{  
    ihex_Info.type = extract_rec_type(line);
    // printf("Parse_Messages::parse_line: record type= %d\n", ihex_Info.type);

    t_u16 len;
    if(ihex_Info.type == IHEX_TYPE::DATA)                             //  data present
	{
	    len = extract_data_len(line); 

        if (ihex_Info.len == 0)
            extract_offset_addr(line);

	    extract_data_chunk(line, len);
        ihex_Info.len += len;
	}
    if(ihex_Info.type == IHEX_TYPE::EOFILE)                        // end of file
	{
	    handle_eof();
	}
    if(ihex_Info.type == IHEX_TYPE::ELA)
    {
        if (cntEla > 1);
            fValidEla = TRUE;
        cntEla++;
        extract_base_addr(line);
    }
}

t_u16 Parse_Messages::get_ela_status()
{
    return fValidEla;
}

t_u16 Parse_Messages::get_rec_type()
{
    return ihex_Info.type;
}

t_u16 Parse_Messages::get_data_len()
{
    return ihex_Info.len;
}

std::vector<byte> Parse_Messages::get_data_chunk()
{   
    return dataChunk;
}

std::vector<byte> Parse_Messages::get_mem_addr()
// std::vector<std::vector<byte>> Parse_Messages::get_mem_addr()
{
    memAddr.clear();

    memAddr.insert(memAddr.begin(), baseAddr.begin(), baseAddr.end());
    memAddr.insert(memAddr.end(), offsetAddr.begin(), offsetAddr.end());  

    return memAddr;
}

bool Parse_Messages::get_eof_status()
{
    return  fEof;
}

void Parse_Messages::reset_data_len()
{
    dataChunk.clear();
    ihex_Info.len = 0;
    dataIdx = 0;
    fValidEla = FALSE;
}

std::vector<byte> Parse_Messages::test(t_u16 intVal, t_u16 num)
{
    std::vector<t_u8>   temp;
    temp.reserve(4);
    // temp.clear();

    t_u16 i;
    t_u16 a[4];
    std::vector<t_u8> byteAry;
    byteAry.reserve(num);
    for (i = 0; i < num; i++)
    {
        a[num - 1 - i] = (intVal >> (i * 8));
        byteAry.push_back(a[num - 1 - i]);
        printf("@@@ Parse_Messages::test: byteAry[%d]= %d\n", i, byteAry.at(i));
    }
    return byteAry;
    
}

Parse_Messages::Parse_Messages()
{
    cntEla = 0;
    fValidEla = FALSE;
    fEof = FALSE;
    ihex_Info.len = 0;
    ihex_Info.chksum = 0;
    baseAddr.reserve(2);
    offsetAddr.reserve(2);
    memAddr.reserve(10);
    dataChunk.reserve(BUFFER_SIZE);
}