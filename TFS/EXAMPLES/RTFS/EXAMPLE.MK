# @(#)example.mk	1.1

all: test.btl

fulltest.lku: fulltest.c
	icc -t8 fulltest.c
	ilink -t8 fulltest.tco rtfs.lib -f startup.lnk

test.btl: fulltest.lku test.cfs
	icvconf test.cfs
	icollect test.cfb

