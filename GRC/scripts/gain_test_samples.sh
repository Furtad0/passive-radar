#!/bin/bash

while [[ $# -gt 0 ]] 
do
key="$1"
case $key in
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
END=${END:-100}

mkdir ../samples/$NAME

while [ $GAIN -lt $END ]; do
    FILENAME="$GAIN"
    FILENAME+=".dat"
    /usr/lib/uhd/examples/rx_samples_to_file --file ~/grc/samples/$NAME/$FILENAME --type float --duration 5 --freq $FREQ --gain $GAIN --ant "RX2" --subdev "A:A"
    let GAIN=$GAIN+$STEP
done


#make archive and clean up
ARCHIVENAME=""
ARCHIVENAME+=$NAME
ARCHIVENAME+=".tar"

cd ~/grc/samples

tar -cf ~/$ARCHIVENAME $NAME/*
rm -r ~/grc/samples/$NAME

cd ~/grc/scripts


