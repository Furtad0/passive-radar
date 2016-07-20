#!/bin/bash

usage="$(basename "$0") [-h] [-g gain] -n name

Program to take samples between 404 and 948 MHz in steps of 4 MHz.

where: 
    -h|--help              show this help text
    -g|--gain     (=30)    set the gain of the receiver
    -r|--rate     (=16e6)  set the sampling rate of the receiver
    -d|--duration (=0.2)   set the duration (seconds) during which to take samples
    -n|--name              set the name of the folder and archive in which samples will be saved"

while [[ $# -gt 0 ]] 
do
key="$1"
case $key in
    -n|--name)
    NAME="$2"
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

NAME=${NAME:-"NA"}
if [ "$NAME" == "NA" ]; then
    echo "Please input name (-n)"
    exit 0
fi

GAIN=${GAIN:-30}
RATE=${RATE:-16000000}
DUR=${DUR:-0.1}

mkdir ../samples/$NAME

# cage elements
FREQMHZ=404     # starting
FREQMHZMAX=948  # ending
FREQMHZSTEP=4   # step

~/grc/applications/take_samples/build/take_samples --file ~/grc/samples/$NAME/ --gain $GAIN --start $FREQMHZ --end $FREQMHZMAX --step $FREQMHZSTEP --rate $RATE --duration $DUR
#make archive and clean up
ARCHIVENAME="low_cone_"
ARCHIVENAME+=$NAME
ARCHIVENAME+=".tar.xz"

cd ~/grc/samples

#tar -cf ~/$ARCHIVENAME $NAME/*
tar -cf /media/usb0/$ARCHIVENAME $NAME/*
rm -r ~/grc/samples/$NAME

cd ~/grc/scripts

