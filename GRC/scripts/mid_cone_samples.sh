#!/bin/bash

usage="$(basename "$0") [-h] [-g gain] -n name

Program to take samples between 764 and 1124 MHz in steps of 4 MHz.

where: 
    -h|--help  show this help text
    -g|--gain  set the gain of the receiver, defaults to 0
    -n|--name  set the name of the folder and archive in which samples will be saved"

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

GAIN=${GAIN:-0}

mkdir ../samples/$NAME

# cage elements
FREQMHZ=764     # starting
FREQMHZMAX=1124 # ending
FREQMHZSTEP=4   # step


~/grc/applications/take_samples/build/take_samples --file ~/grc/samples/$NAME/ --gain $GAIN --start $FREQMHZ --end $FREQMHZMAX --step $FREQMHZSTEP
#make archive and clean up
ARCHIVENAME=""
ARCHIVENAME+=$NAME
ARCHIVENAME+="_mid_cone.tar.xz"

cd ~/grc/samples

tar -cf ~/$ARCHIVENAME $NAME/*
rm -r ~/grc/samples/$NAME

cd ~/grc/scripts

