#!/bin/bash

usage="$(basename "$0") [-h] [-g gain] -n name

Program to take samples at a fixed frequency from the direction antenna.

where: 
    -h|--help              show this help text
    -g|--gain     (=20)    set the gain of the receiver
    -r|--rate     (=16e6)  set the sampling rate of the receiver
    -d|--duration (=0.1)   set the duration (seconds) during which to take samples
    -i|--nfiles   (=100)   number of files of samples to save
    -f|--freq     (=170)   set the frequency (MHz) at which to take samples
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
    -i|--nfiles)
    NFILES="$2"
    shift
    ;;
    -f|--freq)
    FREQ="$2"
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
NFILES=${NFILES:-100}
FREQ=${FREQ:-170}

mkdir ../samples/$NAME

~/grc/applications/take_samples_fixed/build/take_samples_fixed --file ~/grc/samples/$NAME/ --gain $GAIN --nfiles $NFILES --freq $FREQ --rate $RATE --duration $DUR


#make archive and clean up
echo "Compressing..."
ARCHIVENAME="dir_fixed_"
ARCHIVENAME+=$NAME
ARCHIVENAME+=".tar.xz"

cd ~/grc/samples

#tar -cf ~/$ARCHIVENAME $NAME/*
tar -cf /media/usb0/$ARCHIVENAME $NAME/*
rm -r ~/grc/samples/$NAME

cd ~/grc/scripts

