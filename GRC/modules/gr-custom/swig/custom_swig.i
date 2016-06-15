/* -*- c++ -*- */

#define CUSTOM_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "custom_swig_doc.i"

%{
#include "custom/gen_triangle.h"
%}


%include "custom/gen_triangle.h"
GR_SWIG_BLOCK_MAGIC2(custom, gen_triangle);
