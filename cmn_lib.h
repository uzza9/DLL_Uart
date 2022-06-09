#ifndef     CMN_LIB_H
#define     CMN_LIB_H

#include    "cmn_cfg.h"
#include    <algorithm>


#if defined(__cplusplus)
extern "C" {
#endif


void current_time_point(std::chrono::system_clock::time_point timePt);
void timestamp_start(const std::string &msg);
void timestamp_end(const std::string &msg);
void timestamp_elapsed(const std::string &msg);


class Cmn_Lib
{
private:


public:


    Cmn_lib();
    virtual ~Cmn_Lib();

};

namespace Inline_Lib
{
    inline  t_u16       convert_binary_int(t_u8 data1, t_u8 data2)
    {
        // printf("@@@ cmn_lib::convert_binary_2int: data1= %d, data2= %d\n", data1, data2);

        return (((t_u16)data1 << 8) + ((t_u16)data2));
    }

    inline  t_u32       convert_binary_int4(t_u8 data[], t_u16 num)
    {
        t_u8 u8val[num] = {0};
        t_u32 u32val = 0;

        for (int i = 0; i < num; i++)
        {
            u8val[i] = data[i] << (8 * (num - 1 - i));
            u32val += u8val[i];
        }
        // printf("@@@ cmn_lib::convert_binary_2int4: u32val= %d\n", u32val);

        return u32val;
    }
    
    inline  t_u8*   reverse_bytes(void *start, t_u16 num)
    {
        t_u8 *buff = (t_u8*) start;

        for( int i = 0; i < num / 2; i++ ) 
        {
           std::swap(buff[i], buff[num - i - 1]);
        }

        return buff;
    }

    inline void convert_charAry_strAry(t_u8 *inBuf, t_u16 num)
    {
        std::string strRslt;

        for (int i = 0; i < num; i++)
        {
            // if (!(txBuff[i] == 0 && result.size() == 0))
            // printf("@@@ cmn_lib::convert_charAry_2_strAry: inBuf= %d\n", inBuf[i]);

            strRslt += ("  " + std::to_string(inBuf[i]) + " ");
        }
        std::cout << "@@@ cmn_lib::convert_charAry_2_strAry:  " << strRslt << ">>>>>>>>>>>>>>" << std::endl;

        // return strRslt;        
    }

    // template <typename T>
    inline std::vector<t_u8> convert_int_bytes(t_u32 intVal, t_u16 num)
    {
        t_u16   i;
        t_u8    temp[4];
        std::vector<t_u8> byteAry;
        byteAry.reserve(num);
        for (i = 0; i < num; i++)
        {
            temp[num - 1 - i] = (intVal >> (i * 8));
            byteAry.insert(byteAry.begin(), temp[num - 1 - i]);      // save MSB at front
            // printf("@@@ Parse_Messages::test: byteAry[%d]= %d\n", i, byteAry.at(i));
        }
        return byteAry;
    }

     inline void convert_int_bytes_R1(t_u32 intVal, t_u16 num, t_u8 &rtn)
    {
        t_u16   i;
        t_u8    temp[4];
        for (i = 0; i < num; i++)
        {
            temp[num - 1 - i] = (intVal >> (i * 8));
            // printf("@@@ Parse_Messages::test: byteAry[%d]= %d\n", i, byteAry.at(i));
        }
        // memcpy(rtn, temp, num);
    }   
 
    inline t_u16 generate_crc16(t_u8 *data, t_u16 num)       // num = sizeof(data) - 2; here 2 equals to CRC16 bytes send from target
    {
        t_u16   crc16 = 0xffff;
        t_u16   crcWORD = 0xa001;    

        for (int i = 0; i < num; i++)
        {
            crc16 ^= (t_u16)data[i];

            for(int j = 0; j < 8; j++)
            {
                if (crc16 & 0x01)
                {
                    crc16 >>= 1;
                    crc16 ^= crcWORD;
                }
                else
                    crc16 >>= 1;
            } 
        }
        // printf("@@@ cmn_lib::generate_crc16: crc16= %d  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", crc16);

        return (crc16 & 0xffff);
    }
}

#if defined(__cplusplus)
}
#endif      // end (__cplusplus)

#endif      // end CMN_CFG_H