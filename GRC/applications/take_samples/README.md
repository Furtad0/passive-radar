This folder contains an application for taking a certain duration of samples.  It is based on the UHD example rx_samples_to_file.  When built, running ./take_samples --help will provide additional details.

build.sh is a utility script for building the application.  The executable can be found in the build directory.

take_samples.cpp can record samples at 4MS/s reliably without overflow.
take_samples_verbose.cpp is a version of take_samples.cpp that prints information about overflows.
take_samples_circular.cpp is an in-progress application that will be able to record at higher sampling rates for a finite amount of time by storing data in active memory, then copying it to the SD card.

Which of the above .cpp files is compiled can be controlled by modifying CMakeLists.txt.
