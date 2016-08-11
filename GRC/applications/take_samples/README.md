This folder contains an application for taking a certain duration of samples.  It is based on the UHD example rx_samples_to_file.  When built, running ./take_samples --help will provide additional details.

build.sh is a utility script for building the application.  
The executable can then be found in the build directory.

Each of these applications steps from a start frequency to an end frequency at a constant step, taking and saving samples.

take_samples.cpp can record samples at 4MS/s reliably without overflow.

take_samples_rapid.cpp can record at higher sampling rates for a finite amount of time by storing data in active memory, then copying it to the SD card.  It can theoretically record approximately 2 seconds of data (per frequency) at 16 MS/s without overflow.  In practice, due to SD card write rate unpredictability, I found that 0.6s or so can reliably be recorded without overflow when many files are being recorded at a time.

Use --help for more detailed information.
Notably, the --file flag should be used to set the file location as well as the base name.

Which of the above .cpp files is compiled can be controlled by modifying CMakeLists.txt.

