This folder contains an application for taking a certain duration of samples.  It is based on the UHD example rx_samples_to_file.  When built, running ./take_samples --help will provide additional details.

build.sh is a utility script for building the application.  
The executable can then be found in the build directory.

Each of these applications steps from a start frequency to an end frequency at a constant step, taking and saving samples.

take_samples.cpp can record samples at 4MS/s reliably without overflow.

take_samples_verbose.cpp is a version of take_samples.cpp that prints information about overflows.

take_samples_circular.cpp is an in-progress application that will be able to record at higher sampling rates for a finite amount of time by storing data in active memory, then copying it to the SD card.

Use --help for more detailed information.
Notably, the --file flag should be used to set the file location as well
as the base name.

Which of the above .cpp files is compiled can be controlled by modifying CMakeLists.txt.

