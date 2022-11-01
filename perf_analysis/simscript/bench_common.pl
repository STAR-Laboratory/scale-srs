#!/usr/bin/perl -w

#******************************************************************************
# Benchmark Sets
# ************************************************************

%SUITES = ();

#***************************************************************
# Names
#***************************************************************
$SUITES{'interest_name'} =
'hmmer
bzip2
h264
calculix
gcc
zeusmp
astar
sphinx
mummer
ferret
gobmk
blender_17
freq
stream
gcc_17
swapt
black
comm1
xz_17
comm2
omnetpp_17
fluid
omnetpp
face
mcf
gromacs
comm5
comm3
gups';

$SUITES{'gups_name'}	=
'gups';


$SUITES{'all78'}	=
'astar
bwaves
bzip2
cactusADM
calculix
dealII
gamess
gcc
GemsFDTD
gobmk
gromacs
h264
hmmer
lbm
leslie3d
libquantum
mcf
milc
namd
omnetpp
perlbench
povray
sjeng
soplex
sphinx
tonto
wrf
xalancbmk
zeusmp
black
ferret
face
fluid
freq
stream
swapt
comm1
comm2
comm3
comm4
comm5
gups
mummer
tigr
blender_17
bwaves_17
cactuBSSN_17
cam4_17
deepsjeng_17
fotonik3d_17
gcc_17
imagick_17
lbm_17
leela_17
mcf_17
nab_17
namd_17
omnetpp_17
parest_17
perlbench_17
povray_17
roms_17
wrf_17
x264_17
xalancbmk_17
xz_17
bc_t
bc_w
cc_t
cc_w
pr_t
pr_w
mix1
mix2
mix3
mix4
mix5
mix6';

$SUITES{'parsec_name'} =
'black
ferret
face
fluid
freq
stream
swapt';

$SUITES{'comm_name'} =
'comm1
comm2
comm3
comm4
comm5';

$SUITES{'biobench_name'} =
'mummer
tigr';

$SUITES{'spec2017_name'} =
'blender_17
bwaves_17
cactuBSSN_17
cam4_17
deepsjeng_17
fotonik3d_17
gcc_17
imagick_17
lbm_17
leela_17
mcf_17
nab_17
namd_17
omnetpp_17
parest_17
perlbench_17
povray_17
roms_17
wrf_17
x264_17
xalancbmk_17
xz_17';

$SUITES{'gap_name'} =
'bc_t
bc_w
cc_t
cc_w
pr_t
pr_w';

$SUITES{'hmpki_name'} =
'mcf
lbm
soplex
milc
libquantum
omnetpp
bwaves
gcc
sphinx
GemsFDTD
leslie3d
wrf
cactusADM
zeusmp
bzip2
dealII
xalancbmk';

$SUITES{'lmpki_name'} =
'hmmer
perlbench
h264
astar
gromacs
gobmk
sjeng
namd
tonto
calculix
gamess
povray';

$SUITES{'mix_name'} =
'mix1
mix2
mix3
mix4
mix5
mix6';

$SUITES{'spec2006_name'} =
'astar
bwaves
bzip2
cactusADM
calculix
dealII
gamess
gcc
GemsFDTD
gobmk
gromacs
h264
hmmer
lbm
leslie3d
libquantum
mcf
milc
namd
omnetpp
perlbench
povray
sjeng
soplex
sphinx
tonto
wrf
xalancbmk
zeusmp';

#***************************************************************
# SPEC2K6 SUITE from IMAT TRACES -- 8CORE + 2 Channel
#***************************************************************

$SUITES{'8c_2ch_mix'} =
'input/2channel.cfg input/BIOBENCH/mummer.gz input/SPEC2K6/lbm.gz input/COMM/comm1.gz input/PARSEC/black.gz input/SPEC2K17/nab_17.gz input/GAP/cc_t.gz input/SPEC2K6/hmmer.gz input/SPEC2K17/leela_17.gz
input/2channel.cfg input/BIOBENCH/tigr.gz input/SPEC2K6/povray.gz input/COMM/comm2.gz input/PARSEC/ferret.gz input/SPEC2K17/xz_17.gz input/GAP/cc_w.gz input/SPEC2K6/wrf.gz input/SPEC2K17/gcc_17.gz
input/2channel.cfg input/SPEC2K6/h264.gz input/SPEC2K6/mcf.gz input/COMM/comm3.gz input/PARSEC/face.gz input/SPEC2K17/namd_17.gz input/GAP/bc_w.gz input/SPEC2K6/sphinx.gz input/SPEC2K17/cactuBSSN_17.gz
input/2channel.cfg input/COMM/comm4.gz input/SPEC2K6/gcc.gz input/PARSEC/fluid.gz input/GAP/bc_t.gz input/BIOBENCH/mummer.gz input/SPEC2K6/omnetpp.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/roms_17.gz
input/2channel.cfg input/SPEC2K6/libquantum.gz input/COMM/comm5.gz input/PARSEC/freq.gz input/GAP/pr_t.gz input/COMM/comm3.gz input/BIOBENCH/tigr.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/cam4_17.gz
input/2channel.cfg input/SPEC2K6/milc.gz input/COMM/gups.gz input/PARSEC/stream.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K17/parest_17.gz input/GAP/pr_w.gz input/SPEC2K6/leslie3d.gz input/PARSEC/swapt.gz';

$SUITES{'8c_2ch_parsec'} =
'input/2channel.cfg input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz
input/2channel.cfg input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz
input/2channel.cfg input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz
input/2channel.cfg input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz
input/2channel.cfg input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz
input/2channel.cfg input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz
input/2channel.cfg input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz';

$SUITES{'8c_2ch_comm'} =
'input/2channel.cfg input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz
input/2channel.cfg input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz
input/2channel.cfg input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz
input/2channel.cfg input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz
input/2channel.cfg input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz';

$SUITES{'8c_2ch_biobench'} =
'input/2channel.cfg input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz
input/2channel.cfg input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz';

$SUITES{'8c_2ch_gap'} =
'input/2channel.cfg input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz
input/2channel.cfg input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz
input/2channel.cfg input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz
input/2channel.cfg input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz
input/2channel.cfg input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz
input/2channel.cfg input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz';

$SUITES{'8c_2ch_spec2017'} =
'input/2channel.cfg input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz
input/2channel.cfg input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz
input/2channel.cfg input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz
input/2channel.cfg input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz
input/2channel.cfg input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz
input/2channel.cfg input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz
input/2channel.cfg input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz
input/2channel.cfg input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz
input/2channel.cfg input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz
input/2channel.cfg input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz
input/2channel.cfg input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz
input/2channel.cfg input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz
input/2channel.cfg input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz
input/2channel.cfg input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz
input/2channel.cfg input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz
input/2channel.cfg input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz
input/2channel.cfg input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz
input/2channel.cfg input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz
input/2channel.cfg input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz
input/2channel.cfg input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz
input/2channel.cfg input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz
input/2channel.cfg input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz';

$SUITES{'8c_2ch_hmpki'} =
'input/2channel.cfg input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz
input/2channel.cfg input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz
input/2channel.cfg input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz
input/2channel.cfg input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz
input/2channel.cfg input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz
input/2channel.cfg input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz
input/2channel.cfg input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz
input/2channel.cfg input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz
input/2channel.cfg input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz
input/2channel.cfg input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz
input/2channel.cfg input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz
input/2channel.cfg input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz
input/2channel.cfg input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz
input/2channel.cfg input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz
input/2channel.cfg input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz
input/2channel.cfg input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz
input/2channel.cfg input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz';


$SUITES{'8c_2ch_lmpki'} =
'input/2channel.cfg input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz
input/2channel.cfg input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz
input/2channel.cfg input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz
input/2channel.cfg input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz
input/2channel.cfg input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz
input/2channel.cfg input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz
input/2channel.cfg input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz
input/2channel.cfg input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz
input/2channel.cfg input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz
input/2channel.cfg input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz
input/2channel.cfg input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz
input/2channel.cfg input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz';

$SUITES{'8c_2ch_gups'} =
'input/2channel.cfg input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz';

######## ALL 78 WORKLOADS TOGETHER ######
$SUITES{'8c_2ch_ALL_78_WL'} =
'input/2channel.cfg input/BIOBENCH/mummer.gz input/SPEC2K6/lbm.gz input/COMM/comm1.gz input/PARSEC/black.gz input/SPEC2K17/nab_17.gz input/GAP/cc_t.gz input/SPEC2K6/hmmer.gz input/SPEC2K17/leela_17.gz
input/2channel.cfg input/BIOBENCH/tigr.gz input/SPEC2K6/povray.gz input/COMM/comm2.gz input/PARSEC/ferret.gz input/SPEC2K17/xz_17.gz input/GAP/cc_w.gz input/SPEC2K6/wrf.gz input/SPEC2K17/gcc_17.gz
input/2channel.cfg input/SPEC2K6/h264.gz input/SPEC2K6/mcf.gz input/COMM/comm3.gz input/PARSEC/face.gz input/SPEC2K17/namd_17.gz input/GAP/bc_w.gz input/SPEC2K6/sphinx.gz input/SPEC2K17/cactuBSSN_17.gz
input/2channel.cfg input/COMM/comm4.gz input/SPEC2K6/gcc.gz input/PARSEC/fluid.gz input/GAP/bc_t.gz input/BIOBENCH/mummer.gz input/SPEC2K6/omnetpp.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/roms_17.gz
input/2channel.cfg input/SPEC2K6/libquantum.gz input/COMM/comm5.gz input/PARSEC/freq.gz input/GAP/pr_t.gz input/COMM/comm3.gz input/BIOBENCH/tigr.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/cam4_17.gz
input/2channel.cfg input/SPEC2K6/milc.gz input/COMM/gups.gz input/PARSEC/stream.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K17/parest_17.gz input/GAP/pr_w.gz input/SPEC2K6/leslie3d.gz input/PARSEC/swapt.gz
input/2channel.cfg input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz
input/2channel.cfg input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz
input/2channel.cfg input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz
input/2channel.cfg input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz
input/2channel.cfg input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz
input/2channel.cfg input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz
input/2channel.cfg input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz
input/2channel.cfg input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz
input/2channel.cfg input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz
input/2channel.cfg input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz
input/2channel.cfg input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz input/COMM/comm4.gz
input/2channel.cfg input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz
input/2channel.cfg input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz
input/2channel.cfg input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz input/BIOBENCH/tigr.gz
input/2channel.cfg input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz input/GAP/bc_t.gz
input/2channel.cfg input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz input/GAP/bc_w.gz
input/2channel.cfg input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz input/GAP/cc_t.gz
input/2channel.cfg input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz input/GAP/cc_w.gz
input/2channel.cfg input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz input/GAP/pr_t.gz
input/2channel.cfg input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz input/GAP/pr_w.gz
input/2channel.cfg input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz
input/2channel.cfg input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz input/SPEC2K17/bwaves_17.gz
input/2channel.cfg input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz input/SPEC2K17/cactuBSSN_17.gz
input/2channel.cfg input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz input/SPEC2K17/cam4_17.gz
input/2channel.cfg input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz input/SPEC2K17/deepsjeng_17.gz
input/2channel.cfg input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz input/SPEC2K17/fotonik3d_17.gz
input/2channel.cfg input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz
input/2channel.cfg input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz input/SPEC2K17/imagick_17.gz
input/2channel.cfg input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz input/SPEC2K17/lbm_17.gz
input/2channel.cfg input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz input/SPEC2K17/leela_17.gz
input/2channel.cfg input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz input/SPEC2K17/mcf_17.gz
input/2channel.cfg input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz input/SPEC2K17/nab_17.gz
input/2channel.cfg input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz input/SPEC2K17/namd_17.gz
input/2channel.cfg input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz
input/2channel.cfg input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz input/SPEC2K17/parest_17.gz
input/2channel.cfg input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz input/SPEC2K17/perlbench_17.gz
input/2channel.cfg input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz input/SPEC2K17/povray_17.gz
input/2channel.cfg input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz input/SPEC2K17/roms_17.gz
input/2channel.cfg input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz input/SPEC2K17/wrf_17.gz
input/2channel.cfg input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz input/SPEC2K17/x264_17.gz
input/2channel.cfg input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz input/SPEC2K17/xalancbmk_17.gz
input/2channel.cfg input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz
input/2channel.cfg input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz
input/2channel.cfg input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz input/SPEC2K6/lbm.gz
input/2channel.cfg input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz input/SPEC2K6/soplex.gz
input/2channel.cfg input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz input/SPEC2K6/milc.gz
input/2channel.cfg input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz input/SPEC2K6/libquantum.gz
input/2channel.cfg input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz
input/2channel.cfg input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz input/SPEC2K6/bwaves.gz
input/2channel.cfg input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz
input/2channel.cfg input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz
input/2channel.cfg input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz input/SPEC2K6/GemsFDTD.gz
input/2channel.cfg input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz input/SPEC2K6/leslie3d.gz
input/2channel.cfg input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz input/SPEC2K6/wrf.gz
input/2channel.cfg input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz input/SPEC2K6/cactusADM.gz
input/2channel.cfg input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz
input/2channel.cfg input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz
input/2channel.cfg input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz input/SPEC2K6/dealII.gz
input/2channel.cfg input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz input/SPEC2K6/xalancbmk.gz
input/2channel.cfg input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz
input/2channel.cfg input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz input/SPEC2K6/perlbench.gz
input/2channel.cfg input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz
input/2channel.cfg input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz
input/2channel.cfg input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz
input/2channel.cfg input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz
input/2channel.cfg input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz input/SPEC2K6/sjeng.gz
input/2channel.cfg input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz input/SPEC2K6/namd.gz
input/2channel.cfg input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz input/SPEC2K6/tonto.gz
input/2channel.cfg input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz
input/2channel.cfg input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz input/SPEC2K6/gamess.gz
input/2channel.cfg input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz input/SPEC2K6/povray.gz
input/2channel.cfg input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz';

$SUITES{'ALL_78_WL_name'} = 
'mix1
mix2
mix3
mix4
mix5
mix6
black
ferret
face
fluid
freq
stream
swapt
comm1
comm2
comm3
comm4
comm5
mummer
tigr
bc_t
bc_w
cc_t
cc_w
pr_t
pr_w
blender_17
bwaves_17
cactuBSSN_17
cam4_17
deepsjeng_17
fotonik3d_17
gcc_17
imagick_17
lbm_17
leela_17
mcf_17
nab_17
namd_17
omnetpp_17
parest_17
perlbench_17
povray_17
roms_17
wrf_17
x264_17
xalancbmk_17
xz_17
mcf
lbm
soplex
milc
libquantum
omnetpp
bwaves
gcc
sphinx
GemsFDTD
leslie3d
wrf
cactusADM
zeusmp
bzip2
dealII
xalancbmk
hmmer
perlbench
h264
astar
gromacs
gobmk
sjeng
namd
tonto
calculix
gamess
povray
gups';


######## WORKLOADS OF INTEREST ######

$SUITES{'8c_2ch_WL_INTEREST'} = 
'input/2channel.cfg input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz input/SPEC2K6/hmmer.gz
input/2channel.cfg input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz input/SPEC2K6/bzip2.gz
input/2channel.cfg input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz input/SPEC2K6/h264.gz
input/2channel.cfg input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz input/SPEC2K6/calculix.gz
input/2channel.cfg input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz input/SPEC2K6/gcc.gz
input/2channel.cfg input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz input/SPEC2K6/zeusmp.gz
input/2channel.cfg input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz input/SPEC2K6/astar.gz
input/2channel.cfg input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz input/SPEC2K6/sphinx.gz
input/2channel.cfg input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz input/BIOBENCH/mummer.gz
input/2channel.cfg input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz input/PARSEC/ferret.gz
input/2channel.cfg input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz input/SPEC2K6/gobmk.gz
input/2channel.cfg input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz input/SPEC2K17/blender_17.gz
input/2channel.cfg input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz input/PARSEC/freq.gz
input/2channel.cfg input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz input/PARSEC/stream.gz
input/2channel.cfg input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz input/SPEC2K17/gcc_17.gz
input/2channel.cfg input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz input/PARSEC/swapt.gz
input/2channel.cfg input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz input/PARSEC/black.gz
input/2channel.cfg input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz input/COMM/comm1.gz
input/2channel.cfg input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz input/SPEC2K17/xz_17.gz
input/2channel.cfg input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz input/COMM/comm2.gz
input/2channel.cfg input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz input/SPEC2K17/omnetpp_17.gz
input/2channel.cfg input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz input/PARSEC/fluid.gz
input/2channel.cfg input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz input/SPEC2K6/omnetpp.gz
input/2channel.cfg input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz input/PARSEC/face.gz
input/2channel.cfg input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz input/SPEC2K6/mcf.gz
input/2channel.cfg input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz input/SPEC2K6/gromacs.gz
input/2channel.cfg input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz input/COMM/comm5.gz
input/2channel.cfg input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz input/COMM/comm3.gz
input/2channel.cfg input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz input/COMM/gups.gz';

$SUITES{'WL_INTEREST_name'} =
'hmmer
bzip2
h264
calculix
gcc
zeusmp
astar
sphinx
mummer
ferret
gobmk
blender_17
freq
stream
gcc_17
swapt
black
comm1
xz_17
comm2
omnetpp_17
fluid
omnetpp
face
mcf
gromacs
comm5
comm3
gups';
