#!/bin/bash

while [[ $# -gt 0 ]] 
do
key="$1"
case $key in
    -d|--duration)
    DURATION="$2"
    shift
    ;;
    -t|--time)
    TIME="$2"
    shift
    ;;
    -r|--rate)
    RATE="$2"
    shift
    ;;
    -g|--gain)
    GAIN="$2"
    shift
    ;;
#    --default)
#    DEFAULT=YES
#    ;;
  esac
  shift
done

TIME=${TIME:-"NA"}
if [ "$TIME" == "NA" ]; then
    echo "Please input time (-t)"
    exit 0
fi

mkdir ../samples/$TIME

# cage elements
FREQMHZ=72     # starting
FREQMHZMAX=400 # ending
FREQMHZSTEP=4  # step

FREQS=""
FREQ=1000000*$FREQMHZ
FILENAME=""
FILEBASE="mhz.dat"

while [ $FREQMHZ -lt $FREQMHZMAX ]; do
    let FREQ=1000000*$FREQMHZ
    let FREQS="10#$FREQMHZ"
    FILENAME=""
    FILENAME+=$TIME
    FILENAME+="_"
    FILENAME+=$FREQS
    FILENAME+=$FILEBASE
    echo $FREQS
    echo "$TIME$FREQS$FILEBASE"
    /usr/lib/uhd/examples/rx_samples_to_file --file ../samples/$TIME/$FILENAME --type float --duration ${DURATION:-0.1} --rate ${RATE:-16000000} --freq $FREQ --gain ${GAIN:-30} --ant "RX2" --subdev "A:A"
    let FREQMHZ=$FREQMHZ+$FREQMHZSTEP
done

ARCHIVENAME=""
ARCHIVENAME+=$TIME
ARCHIVENAME+="_cage.tar"

cd ../samples

tar -cf ~/$ARCHIVENAME $TIME/*
cd ../scripts
rm ../samples/$TIME/*







# /usr/lib/uhd/examples/rx_samples_to_file --file ../samples/$FILENAME --type float --duration ${DURATION:-1} --rate ${RATE:-1000000} --freq $FREQ --gain ${GAIN:-30} --ant $RX2 --subdev "A:A"


