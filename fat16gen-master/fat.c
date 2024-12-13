#include <stdint.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include "fat.h"


int datagen(uint32_t blknum, uint8_t *buf, const vfile_t *filesys, 
	    uint32_t index) {
  const uint8_t *data = (const uint8_t *) filesys->table[index].data;
  uint32_t len = filesys->table[index].len;

  for (int i = 0; (i < BLOCKSIZE) && (i  + blknum*BLOCKSIZE < len); i++)
    buf[i] = data[i+blknum*BLOCKSIZE];
  return 0;
}

/*
 * Separate code to compute fat entry for a given cluster from code that
 * writes the fat table entry.  This should make it easier to support fat12 or fat32 if
 * needed.
 */

// compute the correct fat entry
// returns a value suitable for fat12,fat16, and fat32

static inline uint32_t fatentry(const vfile_t *filesys, uint32_t cluster, uint32_t *index){

  if (cluster == 0)
    return 0x0FFFFFF8;

  if (cluster == 1)
    return 0x0FFFFFFF;

  // Bad cluster ?

  if (cluster >= filesys->maxcluster) 
    return 0x0FFFFFF7;

  // Find the correct table entry

  if (cluster < filesys->table[*index].start_cluster)
    *index = 0;

  while (cluster >= filesys->table[*index+1].start_cluster) 
    *index = *index + 1;
  
  // compute entry

  int cmp = (filesys->table[*index].start_cluster +
	     filesys->table[*index].len/(BLOCKSIZE*filesys->blocks_per_cluster)) - cluster;
  if (cmp == 0)  // last cluster in file 
    return 0x0FFFFFF8;
  if (cmp > 0)   // not last
    return cluster + 1;
    return 0;    // after last -- mark as empty
}

int fat16(uint32_t blknum, uint8_t *buf, const vfile_t *filesys, uint32_t idx) {
  (void) idx;
  uint32_t cluster = blknum * 256;
  uint32_t lastcluster  = cluster + 256;
  uint32_t index = 0;  // starting point of table search
  for (; cluster < lastcluster; cluster++) {
    uint32_t val = fatentry(filesys, cluster, &index);
    *buf++ = (val & 0xff);
    *buf++ = (val  >> 8) & 0xff;
  }
  return 0;
}

int read_vdisk(const void *instance, uint32_t startblk, uint8_t *buf, uint32_t n) {
  const vfile_t *ft = (vfile_t *) instance;
  int index = 0;
  while (n--) {
    bzero(buf,BLOCKSIZE);
    while (ft->table[index + 1].start_block <= startblk) index++;
    if (startblk < UINT32_MAX) {
      if (ft->table[index].fun) {
	ft->table[index].fun(startblk - ft->table[index].start_block, buf, ft, index);
      }
      startblk++;
    }
    buf += BLOCKSIZE;
  }
  return 0;
}

