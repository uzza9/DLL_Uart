#ifndef     PROTOCOL_MSG_H
#define     PROTOCOL MSG_H

#include    "cmn_cfg.h"

#define     CONNECT1                    (t_u8)0x82
#define     CONNECT1_RE                 (t_u8)0x81
#define     CONNECT2                    (t_u8)0x64
#define     CONNECT2_RE                 (t_u8)0x63
#define     BUFFER_CHK_1                (t_u8)0xBD
#define     BUFFER_CHK_1_RE             (t_u8)0x42
#define     BUFFER_CHK_2                (t_u8)0xA1
#define     BUFFER_CHK_2_RE             (t_u8)0xA1
#define     ADDRESS_INFO_REQ            (t_u8)0xB5
#define     ADDRESS_INFO_REQ_RE         (t_u8)0x4A
#define     ADDRESS_INFO_REQ_2          (t_u8)0x3A
#define     ADDRESS_INFO_REQ_2_RE       (t_u8)0x3A
#define     LAST_USED_FLASH             (t_u8)0xBA
#define     LAST_USED_FLASH_RE          (t_u8)0x45
#define     LAST_USED_FLASH_2           (t_u8)0xA5
#define     LAST_USED_FLASH_2_RE        (t_u8)0xA5
#define     ERASE_FLASH_1               (t_u8)0xD1
#define     ERASE_FLASH_1_RE            (t_u8)0x2E
#define     ERASE_FLASH_MASK            (t_u8)0xFE
#define     ERASE_FLASH_MASK_RE         (t_u8)0x56
#define     ERASE_FLASH_2               (t_u8)0x3F
#define     ERASE_FLASH_SUCCESS_RE      (t_u8)0xFF
#define     ERASE_FLASH_FAIL_RE         (t_u8)0x00
#define     READ_MEMORY_1               (t_u8)0x0E
#define     READ_MEMORY_1_RE            (t_u8)0xF1
#define     WRITE_MEMORY_1              (t_u8)0x5B
#define     WRITE_MEMORY_1_RE           (t_u8)0xA4
#define     RESTART_CPU_1               (t_u8)0xBC
#define     RESTART_CPU_1_RE            (t_u8)0x43
#define     RESTART_CPU_2               (t_u8)0x44
#define     RESTART_CPU_2_RE            (t_u8)0xBB

#define     ENG_ACCESS_1                (t_u8)0xB9
#define     ENG_ACCESS_2                (t_u8)0x49
#define     ENG_ACCESS_3                (t_u8)0xD2
#define     ENG_ACCESS_3_RE             (t_u8)0x46

#endif      // end PROTOCOL_MSG_H