#Makefile for mkyaffs
#
# NB this is not yet suitable for putting into the kernel tree.
# YAFFS: Yet another Flash File System. A NAND-flash specific file system. 
#
# Copyright (C) 2002 Aleph One Ltd.
#   for Toby Churchill Ltd and Brightstar Engineering
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

## Change or override  KERNELDIR to your kernel

#KERNELDIR = /usr/src/kernel-headers-2.4.18

CFLAGS =   -O2 -Wall -DCONFIG_YAFFS_UTIL -DCONFIG_YAFFS_DEFINES_TYPES
CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline

## Change if you are using a cross-compiler
MAKETOOLS = 

CC=$(MAKETOOLS)gcc

COMMON_BASE_C_LINKS = yaffs_ecc.c
COMMON_BASE_LINKS = $(COMMON_BASE_C_LINKS) yaffs_ecc.h yaffs_guts.h yaffs_packedtags2.h yaffs_trace.h
COMMON_DIRECT_C_LINKS = yaffs_hweight.c
COMMON_C_LINKS = $(COMMON_DIRECT_C_LINKS) $(COMMON_BASE_C_LINKS)
COMMON_DIRECT_LINKS= $(COMMON_DIRECT_C_LINKS) yaffs_hweight.h yaffs_list.h
COMMONOBJS = $(COMMON_C_LINKS:.c=.o)

MKYAFFSSOURCES = mkyaffsimage.c
MKYAFFSIMAGEOBJS = $(MKYAFFSSOURCES:.c=.o)

MKYAFFS2SOURCES = mkyaffs2image.c
MKYAFFS2LINKS = yaffs_packedtags2.c
MKYAFFS2IMAGEOBJS = $(MKYAFFS2SOURCES:.c=.o) $(MKYAFFS2LINKS:.c=.o)

BASE_LINKS = $(MKYAFFSLINKS) $(MKYAFFS2LINKS) $(COMMON_BASE_LINKS)
DIRECT_LINKS = $(MKYAFFS_DIRECT_LINKS) $(MKYAFFS2_DIRECT_LINKS) $(COMMON_DIRECT_LINKS)
ALL_LINKS = $(BASE_LINKS) $(DIRECT_LINKS)

all: mkyaffsimage mkyaffs2image

$(BASE_LINKS):
	cp ../$@ $@

$(DIRECT_LINKS):
	cp ../direct/$@ $@

$(COMMONOBJS) $(MKYAFFSIMAGEOBJS) $(MKYAFFS2IMAGEOBJS) : $(ALL_LINKS)

$(COMMONOBJS) $(MKYAFFSIMAGEOBJS) $(MKYAFFS2IMAGEOBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

mkyaffsimage: $(MKYAFFSIMAGEOBJS) $(COMMONOBJS)
	$(CC) -o $@  $^

mkyaffs2image: $(MKYAFFS2IMAGEOBJS) $(COMMONOBJS)
	$(CC) -o $@  $^

clean:
	@echo RM	ALL OBJS ALL LINKS
	@-rm -f $(COMMONOBJS) $(MKYAFFSIMAGEOBJS) $(MKYAFFS2IMAGEOBJS) $(ALL_LINKS) >/dev/null
distclean:clean
	@echo RM	mkyaffsimage mkyaffs2image
	@-rm -f mkyaffsimage mkyaffs2image >/dev/null