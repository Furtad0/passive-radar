#!/bin/bash

usage="$(basename "$0") [-h] [-f freq] [-g gain] [-r rate] [-d duration]

Program to take samples at the frequency of an FM station and then decode the resulting data into a .wav audio file.

where: 
    -h|--help             show this help text
    -f|--freq     (=90.1) set the frequency, in MHz, of the channel to record
    -g|--gain     (=30)   set the gain of the receiver
    -r|--rate     (=1e6)  set the sampling rate of the receiver
    -d|--duration (=5)    set the duration (seconds) during which to take samples"

while [[ $# -gt 0 ]] 
do
key="$1"
case $key in
    -f|--freq)
    FREQ="$2"
    shift
    ;;
    -g|--gain)
    GAIN="$2"
    shift
    ;;
   -r|--rate)
    RATE="$2"
    shift
    ;;
    -d|--duration)
    DUR="$2"
    shift
    ;;
    -h|--help)
    echo "$usage"
    exit
    ;;
  esac
  shift
done

# default values
FREQ=${FREQ:-90.1}
GAIN=${GAIN:-30}
RATE=${RATE:-1000000}
DUR=${DUR:-5}
NAME="test_FM"

mkdir ~/grc/samples/$NAME

# take sample
~/grc/applications/take_samples/build/take_samples --file ~/grc/samples/$NAME/ --gain $GAIN --start $FREQ --end $FREQ --step 1 --rate $RATE --duration $DUR


# convert to .wav
echo "Converting..."

cd ~/grc/flowgraphs

echo "(wait ~5 seconds before quitting)"
python FM_receiver_short.py -n ~/grc/samples/$NAME/*.dat -r $RATE

mv wav_out.wav ~/wav_out.wav

rm -r ~/grc/samples/$NAME

