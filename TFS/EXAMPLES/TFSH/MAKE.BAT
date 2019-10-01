rem make rtfsh.btl
icc /t8 rtfsh.c
ilink /t8 rtfsh.tco /f objs.lnk rtfs.lib /f startup.lnk
icvconf rtfsh.cfs
icollect rtfsh.cfb

rem make tfsh.btl
icc /t8 /o tfs_util.t8x tfs_util.c
icc /t8 /o lib\bench.t8x bench.c
ilink /t8 /o tfsh.lku tfs_util.t8x /f objs.lnk tfs.lib ttm50.lib /f startup.lnk
icollect /t /M 4M tfsh.lku



