/* -*- c++ -*- */

#define CUSTOMPROCESSING_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "customprocessing_swig_doc.i"

%{
#include "customprocessing/crosscorrelator.h"
%}


%include "customprocessing/crosscorrelator.h"
GR_SWIG_BLOCK_MAGIC2(customprocessing, crosscorrelator);
