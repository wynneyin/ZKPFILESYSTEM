#ifndef _FAT_H
#define _FAT_H

#include <stdint.h>

#define BLOCKSIZE 512

#define CRTTIME(hour,min,sec) ((hour)<<11 | (min) << 5 | ((sec)/2))
#define CRTDATE(year,mon,day) ((year)-1980)<<9 |(mon) << 5 | (day)

#define DIR_ENTRY(nm,e, at, year,month,day,hour,min,sec,clust,s) \
{ .name = nm, .extn = e, .attr = at,  \
  .crt_time = CRTTIME(hour,min,sec), \
  .crt_date = CRTDATE(year,month,day),  \
  .lst_mod_time = CRTTIME(hour,min,sec), \
  .lst_mod_date = CRTDATE(year,month,day),  \
  .lst_access_date = CRTDATE(year,month,day),  \
  .strt_clus_lword = clust, \
  .size = s }

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
  uint8_t ord;
  uint16_t fname1_5[5];
  uint8_t flag;
  uint8_t reserved;
  uint8_t chksum;
  uint16_t fname6_11[6];
  uint8_t empty[2];
  uint16_t fname12_13[2];
} lfn_entry;

typedef union {
  dir_entry d;
  lfn_entry l;
} dir_lfn;

typedef struct __attribute__ ((packed))  {
  uint8_t  jump_code[3];
  uint8_t  oem_name[8];
  uint16_t bytes_per_sector;
  uint8_t  cluster_size;
  uint16_t reserved_sectors;
  uint8_t  fat_copies;
  uint16_t max_root_dir;
  uint16_t sectors_per_partition;
  uint8_t  media_descriptor;
  uint16_t sectors_per_fat;
  uint16_t sectors_per_track;
  uint16_t heads;
  uint32_t hidden;
  uint32_t sectors_per_partition_large;
  uint16_t logical_drive;
  uint8_t  signature;
  uint32_t serial_number;
  uint8_t  volume[11];
  uint8_t  fatname[8];
  uint8_t  code[448];
  uint8_t  marker[2];
} bootrecord;

typedef struct filetab_s filetab_t;
typedef struct vfile_s vfile_t;

typedef int datagen_t(uint32_t blknum, uint8_t *buf, const vfile_t *filesys, uint32_t index);

struct filetab_s {
  uint32_t start_block;   // UINT32_MAX for end
  uint32_t start_cluster;
  uint32_t len;
  datagen_t  *fun;
  const void   *data;
};

struct vfile_s {
  uint32_t maxblock;
  uint32_t maxcluster;
  uint32_t blocks_per_cluster;
  uint32_t clusterstart;
  uint32_t total_sectors;
  filetab_t table[];
};

extern datagen_t datagen;
extern datagen_t fat16;
extern int read_vdisk(const void *instance, uint32_t startblk, uint8_t *buf, uint32_t n);

#endif
  
  
