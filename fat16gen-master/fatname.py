#!/usr/bin/python

import os
import re
import sys

def chksum(nm,ext):
    name = "%-8s%-3s"%(nm,ext)
    if len(name) != 11:
        print >> sys.stderr, "unexpected name length in checksum %d %s"%(len(name),name)
    sum = 0
    for c in name:
        sum = ((((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + ord(c))
    return sum &  0xff

def shortname(nm,num):
    nm = re.sub('^[\.]*','',nm)                           # strip leading dots
    nm = re.sub(' ','',nm)                                # strip spaces
    nm = re.sub('[ "*+,/:;<=>?\\\[\]\|]',"_",nm).upper()  # replace illegal dos chars
    parts = nm.rsplit('.',1)                              # split on last dot
    head = re.sub('\.','',parts[0])[:8]                   # remove extra dots and trim
    if len(parts) > 1:
        tail = parts[1][:3]
    else:
        tail = ""
    # if shortname == longname, return original
    if (head + '.' + tail) == nm:
        return (head,tail)
    # create numbered version
    if num < 10:
        return ("%s~%1d"%(head[:6],num),tail[:3])
    elif num < 100:
        return ("%s~%2d"%(head[:5],num),tail[:3])
    else:
        print >> sys.stderr, "Too many identical shortnames"
        raise

def longname(nm):
    nm = re.sub('^[\. ]*','',nm)     # strip leading dots and spaces
    nm = re.sub('[^\w \.-]',"_",nm)  # replace illegal characters
    parts = nm.rsplit('.',1)         # split on last dot
    if len(parts) > 1:
        parts[0] = re.sub('\.','',parts[0])
        nm = ".".join(parts)
    return nm

class FatName:
    def __init__(self,path,shortdict):
        self._path = path
        self._longname = longname(os.path.basename(path))
        dirname = os.path.dirname(path)
        if not dirname in shortdict:
            shortdict[dirname] = {}

        success = False
        for i in range(99)[1:]:
            nm = shortname(self._longname,i)
            if not nm in shortdict[dirname]:
                shortdict[dirname][nm] = True
                success = True
                break
        if success:
            self._name     = nm[0]
            self._ext      = nm[1]
            self._checksum = chksum(nm[0],nm[1])
        else:
            raise Exception("too many similar file names")

    @property
    def path(self):
        return self._path

    @property
    def longname(self):
        return self._longname

    @property
    def name(self):
        return self._name

    @property
    def ext(self):
        return self._ext

    @property
    def checksum(self):
        return self._checksum

    def needlfn(self):
        return '.'.join([self._name,self._ext]) != self._longname


def test(nm,sortdict):
    print "trying %s"%(nm)
    f = FatName(nm,shortdict)
    print "\tpath %s"%(f.path)
    print "\tlongname %s"%(f.longname)
    print "\tname %s ext %s checksum 0x%x"%(f.name,f.ext,f.checksum)


if __name__ == "__main__":
    shortdict = {}
    test("./a",shortdict)
    test("./b/a",shortdict)
    test("./ABCDEFG.x",shortdict)
    test("./",shortdict)

            
    
