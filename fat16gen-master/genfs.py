#!/usr/bin/python

import os
import re
import sys
import time
from fatname import FatName

def gen_lfn(DF,checksum,name,seq):
    if len(name) > 13:
        gen_lfn(DF,checksum,name[13:],seq+1)
    else:
        seq = seq | 0x40
    a = [hex(ord(x)) for x in name]
    if len(a) < 13: 
        a.extend(['0x0000'])       # null terminate 
        a.extend(['0xffff']*12)    # pad with 0xffff
    namea = ",".join(a[0:5])
    nameb = ",".join(a[5:11])
    namec = ",".join(a[11:13])
    DF('    {.l = {0x%x, {%s}, 0xf, 0, 0x%x, {%s}, {0, 0}, {%s}}},'%(seq, namea,
                                                                     checksum,nameb,namec))

#
# Dump Boot Record
#

def boot_record(DF,sectors, fat_sectors, cluster_size, root_dir_size, vname):
    DF('static const bootrecord br = {')
    DF('\t.jump_code = {0xEB,0x3C,0x90},')
    DF('\t.oem_name = "MSDOS5.0",')
    DF('\t.bytes_per_sector = BLOCKSIZE,')
    DF('\t.cluster_size = %d, '%(cluster_size))
    DF('\t.reserved_sectors = 1,')
    DF('\t.fat_copies = 2,')
    DF('\t.max_root_dir = %d,'%((root_dir_size+31)//32))
    if sectors < 64*1024:
        DF('\t.sectors_per_partition = %d,'%(sectors))
    else:
        DF('\t.sectors_per_partition_large = %d,'%(sectors))
    DF('\t.media_descriptor = 0xF8,')
    DF('\t.sectors_per_fat = %d,'%(fat_sectors))
    DF('\t.logical_drive = 0x80,')
    DF('\t.signature = 0x29,')
    DF('\t.serial_number = %d,'%(time.time()))
    DF('\t.volume = "%-011s",'%(vname))
    DF('\t.fatname = "FAT16   ",')
    DF('\t.marker = {0x55, 0xAA},')
    DF('};')

#
# Dump Static File
#

def dumpfile(DF,path,index):
    with open(path, 'rb') as fin:
        fin.seek(0,2)
        size = fin.tell()
        fin.seek(0,0)
        if size > 0:
            DF("static const uint8_t file_%d[%d] = { /* %s */"%(index,size,path))
            i = 0
            for chunk in iter((lambda:fin.read(10)),''):
                DF('\t',','.join('0x%02x'%(ord(x)) for x in chunk),',')
                i = i + 1
            DF("\n};")
        fin.close()

#
# Dump Static Directory
#

def dirstring(name,ext,attr,epoch,clust,size):
    dirtmpl= "    {.d=DIR_ENTRY(\"%-8s\",\"%-3s\",%d,%d,%d,%d,%d,%d,%d,%d,%d)},"
    tmstamp = time.gmtime(epoch)
    year = tmstamp.tm_year 
    mon  = tmstamp.tm_mon 
    day  = tmstamp.tm_mday 
    hour = tmstamp.tm_hour 
    min  = tmstamp.tm_min 
    sec  = tmstamp.tm_sec // 2 
    return dirtmpl%(name,ext,attr,year,mon,day,hour,min,sec,clust,size)

def dumpdir(DF, filedict, elem, index, vname):
    dirsize = elem['size']//32
    DF("static const dir_lfn dir_%d[%d] = {/* %s */"%(index,dirsize+1,elem['fatname'].path))
    if index > 0:
        DF(dirstring('.','', 0x11,time.time(),elem['cluster'],0))
        DF(dirstring('..','',0x11,time.time(),filedict[elem['parent']]['cluster'],0))
    else:
        DF(dirstring(vname[0:8],vname[8:11],0x8,time.time(),0,0))
    for e in elem['children']:
        child = filedict[e]
        fatname = child['fatname']
        path = child['fatname'].path
        if child['type'] == 'vfile':
            epoch = time.time()
        else:
            epoch = os.path.getmtime(path)
        if child['type'] == 'dir':
            attr = 0x11
            size = 0
        else:
            size = child['size']
            attr = 0x1
        if fatname.needlfn():
            gen_lfn(DF,fatname.checksum,fatname.longname,1)
        DF(dirstring(fatname.name,fatname.ext,attr,epoch,child['cluster'],size))
    DF("    {.l={0}}\n};\n")

#
#  Walk static tree and gather info
#

def walk_rootfs(root,filedict,shortdict):
    i = 0
    parent = [0]

    for dirpath, dirs, files in os.walk(root):
        p = parent[-1]
        if i == 0:
            p = -1
        filedict.append( {
            'fatname' : FatName(dirpath,shortdict),
            'parent' : p,
            'type' : 'dir',
            'children' : []
            })
        if i != 0:
            filedict[p]['children'].append(i)
        parent.append(i)
        i = i+1

        for file in files:
            pathname = os.path.join(dirpath,file)
            p = parent[-1]
            filedict.append( {
                'fatname'  : FatName(pathname,shortdict),
                'parent' : p,
                'type' : 'file',
                })
            filedict[p]['children'].append(i)
            i = i+1
        parent.pop()

#
#  Class to wrap parsed file system
#
            
class vFileSystem:
    def __init__(self,root,vfiles):
        self.filedict = []
        self.shortdict = {}
        walk_rootfs(root,self.filedict,self.shortdict)
        for f in vfiles :
            dirpath = os.path.dirname(f['path'])
            size = f['size']
            func = f['func']
            operand = f['operand']
            found = False
            for index, elem in enumerate(self.filedict):
                if dirpath == elem['fatname'].path:

                    found = True
                    break
            if found:
                this = len(self.filedict)
                self.filedict.append( {
                        'fatname' : FatName(f['path'],self.shortdict),
                        'parent'  : index,
                        'type'    : 'vfile',
                        'func'    : func,
                        'operand' : operand,
                        'size'    : size,
                        })
                elem['children'].append(this)

            else:
                print >> sys.stderr, "path not found vfile %s fun %s"%(dirpath,func)

    def sz(self,elem,index):
        if elem['type'] == 'dir':
            size = 1
            for f in elem['children']:
                size = size + 1
                fatname = (self.filedict[f])['fatname']
                if fatname.needlfn():
                    nmlen = (len(fatname.longname) + 12 )// 13
                    size = size + nmlen
            if index > 0:
                size = size + 1
            size = size*32
        elif elem['type'] == 'file':
            with open(elem['fatname'].path) as f:
                f.seek(0,2)
                size = f.tell()
                f.close()
        elif elem['type'] == 'vfile':
            size = elem['size']
        return size

    def gensys(self,out,name,vname):
        def DF(*args):
            print >> out, " ".join(map(str,args))

        cluster = 2
        sectors_per_cluster = 4

        # edit volume label
        
        vname = re.sub('^[\. ]*','',vname).upper()# strip leading dots and spaces
        vname = re.sub('[^\w -]',"_",vname)[0:11] # replace illegal characters

        # compute sizes, 

        for index, elem in enumerate(self.filedict):
            elem['size'] = self.sz(elem,index)
            if index > 0:
                elem['cluster'] = cluster
                cluster = cluster + (elem['size'] +   512*sectors_per_cluster-1)//(512*sectors_per_cluster)
            else:
                elem['cluster'] = 0

        #
        # Generate output
        #

        DF("#include <stdint.h>")
        DF("#include \"fat.h\"\n")

        #
        # Dump directories
        #

        for index, elem in enumerate(self.filedict):
            if elem['type'] == 'dir':
                dumpdir(DF,self.filedict,elem,index,vname)

        # Dump files, virtual files

        for index,elem in enumerate(self.filedict):
            if elem['type'] == 'file':
                path = elem['fatname'].path
                dumpfile(DF,path,index)
            elif elem['type'] == 'vfile':
                DF("\nextern datagen_t %s;\n"%(elem['func']))

        # dump boot record

        DF("")

        # compute sector and cluster counts
        # sectors per fat

        fat_sectors         = 1 + cluster // 256                               

        # force this to be at least mimimum size fat 16 table

        if fat_sectors < 16:                                                   
            fat_sectors = 16
        rootdirlen          = (self.filedict[0])['size']            

        # sectors in root dir          

        rootdirsectors      = (rootdirlen + 511)//512                          

        # sector of cluster 2

        cluster_start       = 1 + rootdirsectors + 2*fat_sectors               
        total_sectors       = cluster_start + (cluster-2)*sectors_per_cluster  

        # force minimum sectors for fat16 
        # See "Microsoft Extensible Firmware Inititive FAT32 File System Specification"

        if total_sectors < 4085*4 + cluster_start:
            total_sectors = 4085*4 + cluster_start

        # generate boot record

        boot_record(DF,total_sectors ,fat_sectors,sectors_per_cluster,
                    rootdirsectors*512,"volume")
        DF("")

        # dump file table

        DF("const vfile_t %s = {"%(name))
        DF("  .blocks_per_cluster = %d,"%(sectors_per_cluster))
        DF("  .maxcluster = %d,"%(cluster))
        DF("  .clusterstart = %d,"%(cluster_start))
        DF("  .total_sectors = %d,"%(total_sectors))

        DF("  .table = {")
        # boot record
        DF("\t{0,0,512,datagen,&br},")                                 
        # fat tables (2)
        DF("\t{1,0,%d,fat16,0},"%(fat_sectors*512))                   
        DF("\t{%d,0,%d,fat16,0},"%(1+fat_sectors,fat_sectors*512))    
        # root dirctory
        DF("\t{%d,0,%d,datagen,dir_0},"%(1+2*fat_sectors,rootdirlen))  
        
        for index, elem in enumerate(self.filedict):
            elem_sector = (elem['cluster']-2)*sectors_per_cluster + cluster_start
            if index > 0:
                size = elem['size']
                elem_cluster = 2+(elem_sector - cluster_start)//sectors_per_cluster
                if size > 0:
                    path = elem['fatname'].path
                    if elem['type'] == 'dir':
                        DF("\t{%4d,%4d,%4d,datagen,dir_%d},  /* %s */"%(elem_sector,
                                                                        elem_cluster,
                                                                        size,index,path))
                    elif elem['type'] == 'file':
                        DF("\t{%4d,%4d,%4d,datagen,file_%d}, /* %s */"%(elem_sector,
                                                                        elem_cluster,
                                                                        size,index,path))
                    else:
                        DF("\t{%4d,%4d,%4d,%s,%s}, /* %s */"%(elem_sector,
                                                                   elem_cluster,
                                                                   size,elem['func'], elem['operand'],
                                                                   path))
                else: 
                    size = 1
                    DF("\t{%4d,%4d,%4d,datagen,0},      /* %s */ "%(elem_sector,
                                                                    elem_cluster,
                                                                    size,path))


        DF("\t{UINT32_MAX,%d,0,0,0}\n}};"%(cluster))

if __name__ == "__main__":
    volume_name = 'Test Volume'   # fat16 volume name
    table_name  = 'filesys'       # C name of file system table
    fdout = sys.stdout            # output file
    rootdir = 'testdir'           # path to root file system

    vfiles = [{'path' : os.path.join(rootdir,'a virtual file.txt'), 
               'size' : 1024, 
               'func' : 'vfile_1', 
               'operand' : '(void *) 0'
               }
              ]

    fs = vFileSystem(rootdir,vfiles)
    fs.gensys(fdout,table_name,volume_name)
