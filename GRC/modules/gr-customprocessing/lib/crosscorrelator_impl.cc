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
#include "crosscorrelator_impl.h"
#include <complex>

namespace gr {
  namespace customprocessing {

    crosscorrelator::sptr
    crosscorrelator::make(int min_lag, int max_lag, int min_samp)
    {
        std::vector<int> input_sizes;
        input_sizes.push_back(sizeof(std::complex<float>));
        input_sizes.push_back(sizeof(std::complex<float>));
        input_sizes.push_back(sizeof(int));

        return gnuradio::get_initial_sptr
        (new crosscorrelator_impl(min_lag, max_lag, min_samp, input_sizes));
    }

    /*
     * The private constructor
     */
    crosscorrelator_impl::crosscorrelator_impl(int min_lag, int max_lag, int min_samp, 
      std::vector<int> sizes)
      : gr::sync_block("crosscorrelator",
              gr::io_signature::makev(3, 3, sizes),
              gr::io_signature::make(1, 1, sizeof(std::complex<float>))),
        minl(min_lag),
        maxl(max_lag),
        mins(min_samp),
        atten(max_lag + min_samp),
        lag_size(max_lag + min_samp - min_lag),
        lead_size(max_lag + min_samp)
    {//the total amount of data saved at a time must equal the maximum lag plus the minimum samples
        //set_history(maxl+mins); 
        old_lag = new std::complex<float>[max_lag + min_samp - min_lag];
        old_lead = new std::complex<float>[max_lag + min_samp];
    }

    /*
     * Our virtual destructor.
     */
    crosscorrelator_impl::~crosscorrelator_impl()
    {
    }

    int
    crosscorrelator_impl::work(int noutput_items, //4096
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const std::complex<float> *x_lead = (const std::complex<float> *) input_items[0];
      const std::complex<float> *x_lag = (const std::complex<float> *) input_items[1];
      const int *tau = (const int *) input_items[2];
      std::complex<float> *out = (std::complex<float> *) output_items[0];

      // Do <+signal processing+>
      for (int i = 0; i < noutput_items; i++)
      {
          //update history
          prev_lag = (prev_lag + 1) % lag_size;  //prev_lag should really be named curr_lag...
          prev_lead = (prev_lead + 1) % lead_size;
          old_lag[prev_lag] = x_lag[i];
          old_lead[prev_lead] = x_lead[i];

          //to add all products
          std::complex<float> accum (0.0, 0.0);

          //t is amount of time ago in the lagging x; at t=0, the newest lagging x is used
          for (int t = 0; t < maxl+mins-tau[i]; t++)
          {
              std::complex<float> x_lag_t = old_lag[(prev_lag-t) % lag_size]/atten;//x_lead[tau[i]+t];
              std::complex<float> x_lead_t = old_lead[(prev_lead-t-tau[i]) % lead_size]/atten;//x_lag[t];
              std::complex<float> prod = std::conj(x_lead_t) * x_lag_t;
              accum += prod / atten;
          }
          out[i] = accum / (float)(maxl+mins-tau[i]) / (atten);

          
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace customprocessing */
} /* namespace gr */

