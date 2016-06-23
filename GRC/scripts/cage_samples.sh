#!/bin/bash

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
  esac
  shift
done

NAME=${NAME:-"NA"}
if [ "$NAME" == "NA" ]; then
    echo "Please input name (-n)"
    exit 0
fi

GAIN=${GAIN:-30}

mkdir ../samples/$NAME

# cage elements
FREQMHZ=72     # starting
FREQMHZMAX=400 # ending
FREQMHZSTEP=4  # step

~/grc/applications/take_samples/build/take_samples --file ~/grc/samples/$NAME/ --gain $GAIN --start $FREQMHZ --end $FREQMHZMAX --step $FREQMHZSTEP

#make archive and clean up
ARCHIVENAME=""
ARCHIVENAME+=$NAME
ARCHIVENAME+="_cage.tar"

cd ~/grc/samples

tar -cf ~/$ARCHIVENAME $NAME/*
rm -r ~/grc/samples/$NAME

cd ~/grc/scripts


