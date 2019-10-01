
T8OBJS= libs/args.t8x libs/bench.t8x libs/chmod.t8x libs/copy.t8x \
	libs/df.t8x libs/du.t8x libs/find.t8x libs/getcwd.t8x \
	libs/ls.t8x libs/misc.t8x libs/move.t8x libs/rm.t8x \
	libs/tar.t8x libs/transfer.t8x

TAOBJS= libs/args.tax libs/bench.tax libs/chmod.tax libs/copy.tax \
	libs/df.tax libs/du.tax libs/find.tax libs/getcwd.tax \
	libs/ls.tax libs/misc.tax libs/move.tax libs/rm.tax \
	libs/tar.tax libs/transfer.tax

all: rtfsh.btl tfsh.btl

rtfsh.btl: rtfsh.lku rtfsh.cfs
	icvconf rtfsh.cfs
	icollect rtfsh.cfb

rtfsh.lku: rtfsh.c
	icc -t8 rtfsh.c
	ilink -t8 rtfsh.tco $(T8OBJS) rtfs.lib -f startup.lnk

tfsh.btl: tfs_util.t8x lib/bench.t8x
	ilink -t8 -o tfsh.lku tfs_util.t8x $(T8OBJS) tfs.lib ttm50.lib -f startup.lnk
	icollect -t -M 4M tfsh.lku

tfs_util.t8x: tfs_util.c
	icc -t8 -o tfs_util.t8x tfs_util.c

lib/bench.t8x: bench.c
	icc -t8 -o lib/bench.t8x bench.c

