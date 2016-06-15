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


#ifndef INCLUDED_CUSTOMPROCESSING_CROSSCORRELATOR_H
#define INCLUDED_CUSTOMPROCESSING_CROSSCORRELATOR_H

#include <customprocessing/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace customprocessing {

    /*!
     * \brief <+description of block+>
     * \ingroup customprocessing
     *
     */
    class CUSTOMPROCESSING_API crosscorrelator : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<crosscorrelator> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of customprocessing::crosscorrelator.
       *
       * To avoid accidental use of raw pointers, customprocessing::crosscorrelator's
       * constructor is in a private implementation
       * class. customprocessing::crosscorrelator::make is the public interface for
       * creating new instances.
       */
      static sptr make(int min_lag, int max_lag, int min_samp);
    };

  } // namespace customprocessing
} // namespace gr

#endif /* INCLUDED_CUSTOMPROCESSING_CROSSCORRELATOR_H */

