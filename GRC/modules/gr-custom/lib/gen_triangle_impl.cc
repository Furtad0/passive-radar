/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "gen_triangle_impl.h"

namespace gr {
  namespace custom {

    gen_triangle::sptr
    gen_triangle::make(int len, int peak)
    {
      return gnuradio::get_initial_sptr
        (new gen_triangle_impl(len, peak));
    }

    /*
     * The private constructor
     */
    gen_triangle_impl::gen_triangle_impl(int len, int peak)
      : gr::sync_block("gen_triangle",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(float))),
        length((double)len), //pass in given parameters
        height((double)peak),
        increasing(true),
        done(false), //start with done = false and time = 0
        t(0)
    {}

    /*
     * Our virtual destructor.
     */
    gen_triangle_impl::~gen_triangle_impl()
    {
    }

    int
    gen_triangle_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      float *out = (float *) output_items[0];

      // Do <+signal processing+>
      for (int i = 0; i < noutput_items; i++)
      {
          //update state
          if (increasing && (t > length)) //increasing while t <= length
          {
              increasing = false;
          }
          if (!done && t > 2 * length) //done when t > 2 * length
          {
              done = true;
          }

          //calculate output
          float curr_out = 0;
          if (!done && increasing) //if on rising edge of triangle
          {
              curr_out = height/length * t;
          }
          else if (!done) //if on falling edge of triangle
          {
              curr_out = 2 * height - (height/length * t);
          }
          out[i] = (int)curr_out;
          t++;
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace custom */
} /* namespace gr */

