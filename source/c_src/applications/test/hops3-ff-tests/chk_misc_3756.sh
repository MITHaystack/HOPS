#!/bin/bash
#
# $Id: chk_misc_3756.sh 4383 2025-07-20 21:38:08Z gbc $
#
# canonical test suite for fourfit
#

verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/testdata; pwd`

cat > cf3756.misc <<EOF
* four new features
  noautofringes true
  mod4numbering true
  polfringnames true
  fringeout_dir `pwd`/fringes3756
  mbdrplopt 1 3 5
  sb_win -0.3 0.3
*
* back to ff-3756 conf file...
*
EOF

cat $DATADIR/3756/cf_3758_GEHIMSTVY_pstokes4 >> cf3756.misc

rm -f ff-misc-3756*.ps
$verb && echo \
$fourfit -d diskfile:ff-misc-3756-%d.ps \\ && echo \
    -c cf3756.misc \\ && echo \
    $DATADIR/3756/328-1800/1803+784.1TDGBD

$fourfit -d diskfile:ff-misc-3756-%d.ps \
    -c cf3756.misc \
    $DATADIR/3756/328-1800/1803+784.1TDGBD # 1>&2

# pluck out lines containing the snr and amp, parse and check
filecnt=0
aoksum=0
for file in ff-misc-3756-?.ps
do
    $verb && echo considering snr $file
    case $file in
    # ff-misc-3756-0.ps:7570 9653 M (116.1) SR
    ff-misc-3756-0.ps) low=116.0 high=116.2 ;;
    # ff-misc-3756-1.ps:7570 9653 M (100.8) SR
    ff-misc-3756-1.ps) low=100.7 high=100.9 ;;
    # ff-misc-3756-2.ps:7570 9653 M (24.8) SR
    ff-misc-3756-2.ps) low=24.7  high=24.9  ;;
    # ff-misc-3756-3.ps:7570 9653 M (35.3) SR
    ff-misc-3756-3.ps) low=35.2  high=35.4  ;;
    *) echo file is $file ;  exit 99   ;;
    esac
    line=$(grep '7570 9653' $file)
    IFS='()'
    read a snr b <<<"$line"
    IFS=''
    aok=$(echo "$snr>$low && $snr<$high" | bc)
    $verb && echo $file aok is $aok and \
        "$low < $snr < $high" is expected from: $line
    filecnt=$(($filecnt + 1))
    aoksum=$(($aoksum + $aok))
    $verb && echo filecnt: $filecnt
done
for file in ff-misc-3756-?.ps
do
    $verb && echo considering amp $file
    case $file in
    # ff-misc-3756-0.ps:7570 9384 M (5.535) SR
    ff-misc-3756-0.ps) low=5.530 high=5.540 ;;
    # ff-misc-3756-1.ps:7570 9384 M (4.805) SR
    ff-misc-3756-1.ps) low=4.800 high=4.810 ;;
    # ff-misc-3756-2.ps:7570 9384 M (1.182) SR
    ff-misc-3756-2.ps) low=1.180 high=1.184 ;;
    # ff-misc-3756-3.ps:7570 9384 M (1.683) SR
    ff-misc-3756-3.ps) low=1.680 high=1.686 ;;
    *) echo file is $file ;  exit 99   ;;
    esac
    line=$(grep '7570 9384' $file)
    IFS='()'
    read a amp b <<<"$line"
    IFS=''
    aok=$(echo "$amp>$low && $amp<$high" | bc)
    $verb && echo $file aok is $aok and \
        "$low < $amp < $high" is expected from: $line
    filecnt=$(($filecnt + 1))
    aoksum=$(($aoksum + $aok))
    $verb && echo filecnt: $filecnt
done

$verb && echo "$aoksum" -eq 8 -a "$filecnt" -eq 8
[ "$aoksum" -eq 8 -a "$filecnt" -eq 8 ]
#
# eof
#
