#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Fm Receiver
# Generated: Mon Jul 18 21:04:46 2016
##################################################

from gnuradio import analog
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser


class FM_receiver(gr.top_block):

    def __init__(self, filename="0.dat", freq=90.1e6, samp_rate=1e6):
        gr.top_block.__init__(self, "Fm Receiver")

        ##################################################
        # Parameters
        ##################################################
        self.filename = filename
        self.freq = freq
        self.samp_rate = samp_rate

        ##################################################
        # Variables
        ##################################################
        self.gain = gain = 30
        self.audio_rate = audio_rate = 48e3
        self.audio_interp = audio_interp = 4

        ##################################################
        # Blocks
        ##################################################
        self.rational_resampler_xxx_0 = filter.rational_resampler_ccc(
                interpolation=int(audio_rate*audio_interp),
                decimation=int(samp_rate),
                taps=None,
                fractional_bw=None,
        )
        self.low_pass_filter_0 = filter.interp_fir_filter_fff(1, firdes.low_pass(
        	1, int(audio_rate), 800, 100, firdes.WIN_HAMMING, 6.76))
        self.blocks_wavfile_sink_0 = blocks.wavfile_sink("wav_out.wav", 1, int(audio_rate), 8)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_gr_complex*1, filename, False)
        self.analog_wfm_rcv_0 = analog.wfm_rcv(
        	quad_rate=int(audio_rate*audio_interp),
        	audio_decimation=int(audio_interp),
        )

        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_wfm_rcv_0, 0), (self.low_pass_filter_0, 0))    
        self.connect((self.blocks_file_source_0, 0), (self.blocks_throttle_0, 0))    
        self.connect((self.blocks_throttle_0, 0), (self.rational_resampler_xxx_0, 0))    
        self.connect((self.low_pass_filter_0, 0), (self.blocks_wavfile_sink_0, 0))    
        self.connect((self.rational_resampler_xxx_0, 0), (self.analog_wfm_rcv_0, 0))    

    def get_filename(self):
        return self.filename

    def set_filename(self, filename):
        self.filename = filename
        self.blocks_file_source_0.open(self.filename, False)

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)

    def get_gain(self):
        return self.gain

    def set_gain(self, gain):
        self.gain = gain

    def get_audio_rate(self):
        return self.audio_rate

    def set_audio_rate(self, audio_rate):
        self.audio_rate = audio_rate
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, int(self.audio_rate), 800, 100, firdes.WIN_HAMMING, 6.76))

    def get_audio_interp(self):
        return self.audio_interp

    def set_audio_interp(self, audio_interp):
        self.audio_interp = audio_interp


def argument_parser():
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    parser.add_option(
        "-n", "--filename", dest="filename", type="string", default="0.dat",
        help="Set name [default=%default]")
    parser.add_option(
        "-f", "--freq", dest="freq", type="eng_float", default=eng_notation.num_to_str(90.1e6),
        help="Set freq [default=%default]")
    parser.add_option(
        "-r", "--samp-rate", dest="samp_rate", type="eng_float", default=eng_notation.num_to_str(1e6),
        help="Set rate [default=%default]")
    return parser


def main(top_block_cls=FM_receiver, options=None):
    if options is None:
        options, _ = argument_parser().parse_args()

    tb = top_block_cls(filename=options.filename, freq=options.freq, samp_rate=options.samp_rate)
    tb.start()
    try:
        raw_input('Press Enter to quit: ')
    except EOFError:
        pass
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
