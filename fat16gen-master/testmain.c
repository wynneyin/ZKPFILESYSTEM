#include <stdint.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include "fat.h"

int vfile_1(uint32_t blknum, uint8_t *buf, const vfile_t *filesys, uint32_t index) {
  int i;
  char f;

  if (blknum == 0) 
    f = '0';
  else
    f = '1';
  if (blknum < 2)
    for (i = 0; i < BLOCKSIZE; i++)
      buf[i] = f;
  return 0;
}

extern const vfile_t filesys;

int main() {
  uint8_t buf[BLOCKSIZE];
  int i;
  int limit = filesys.clusterstart + filesys.maxcluster*filesys.blocks_per_cluster;
  for (i = 0; i < limit; i++) {
    read_vdisk(&filesys, i, buf, 1);
    write(STDOUT_FILENO, buf, BLOCKSIZE);
  }
}
