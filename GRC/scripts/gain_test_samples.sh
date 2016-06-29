#!/bin/bash

usage="$(basename "$0") [-h] [-n name] [-f freq] [-b start] [-e end] [-s step]

Program to take samples at frequency freq and gains between start and end in steps.

where:
    -h|--help                show this help text
    -n|--name  (=gain_test)  string to name the archive
    -f|--freq  (=90.1e6)     frequency in Hz at which to take samples
    -b|--start (=0)          first gain to measure
    -e|--end   (=76)         last gain to measure
    -s|--step  (=10)         step between gains

"

while [[ $# -gt 0 ]] 
do
key="$1"
case $key in
    -h|--help)
    echo "$usage"
    exit
    ;;
    -n|--name)
    NAME="$2"
    shift
    ;;
    -f|--freq)
    FREQ="$2"
    shift
    ;;
    -b|--start)
    START="$2"
    shift
    ;;
    -e|--end)
    END="$2"
    shift
    ;;
    -s|--step)
    STEP="$2"
    shift
    ;;
  esac
  shift
done

NAME=${NAME:-"gain_test"}

FREQ=${FREQ:-90.1e6}

#start at 0
GAIN=${START:-0}
STEP=${STEP:-10}
END=${END:-76}

mkdir ../samples/$NAME

while [ $GAIN -lt $END ]; do
    FILENAME="$GAIN"
    FILENAME+=".dat"
    /usr/lib/uhd/examples/rx_samples_to_file --file ~/grc/samples/$NAME/$FILENAME --type float --duration 1 --freq $FREQ --gain $GAIN --ant "RX2" --subdev "A:A"
    let GAIN=$GAIN+$STEP
done


#make archive and clean up
ARCHIVENAME=""
ARCHIVENAME+=$NAME
ARCHIVENAME+=".tar.xz"

cd ~/grc/samples

tar -cf ~/$ARCHIVENAME $NAME/*
rm -r ~/grc/samples/$NAME

cd ~/grc/scripts


