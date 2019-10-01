# @(#)example.mk	1.1

all: test.btl

user.lku: user.c
	icc -t8 user.c
	ilink -t8 user.tco -f startup.lnk

test.btl: user.lku test.cfs
	icvconf test.cfs
	icollect test.cfb

