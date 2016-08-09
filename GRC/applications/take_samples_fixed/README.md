This folder contains an application for taking a certain duration of samples.  It is based on the UHD example rx_samples_to_file.  When built, running ./take_samples --help will provide additional details.

build.sh is a utility script for building the application.  
The executable can then be found in the build directory.

take_samples_fixed.cpp repeatedly takes and saves samples at the target frequency.

take_samples_rapid.cpp, upon which this application is heavily based, can record at higher sampling rates for a finite amount of time by storing data in active memory, then copying it to the SD card.  It can record approximately 2 seconds of data at 16 MS/s without overflow.

Use --help for more detailed information.
Notably, the --file flag should be used to set the file location as well
as the base name.


