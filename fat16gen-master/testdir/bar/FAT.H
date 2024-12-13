#ifndef _FAT_H
#define _FAT_H

typedef struct __attribute__ ((packed))  {
  char name[8];
  char extn[3];
  uint8_t attr;
  uint8_t reserved;
  uint8_t crt_time_tenth;
  uint16_t crt_time;
  uint16_t crt_date;
  uint16_t lst_access_date;
  uint16_t strt_clus_hword;
  uint16_t lst_mod_time;
  uint16_t lst_mod_date;
  uint16_t strt_clus_lword;
  uint32_t size;
} dir_entry;

typedef struct __attribute__ ((packed))  {
  uint8_t ord_field;
  uint8_t fname0_4[10];
  uint8_t flag;
  uint8_t reserved;
  uint8_t ch