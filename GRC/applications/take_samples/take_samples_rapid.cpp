//
// Modification of rx_samples_to_file to record many samples
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <uhd/types/tune_request.hpp>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 
#include <iostream>
#include <fstream>
#include <csignal>
#include <complex>
#include <signal.h>
#include <deque>

namespace po = boost::program_options;

static bool stop_signal_called = false;
static int samps_per_buff = 1016;
void sig_int_handler(int){stop_signal_called = true;}

//function for receiving samples from usrp
//samples are written to the interim storage buffs[][]
//communicates with output via boolean flags
//this function is permitted to write to r_ flags only
template<typename samp_type> void receive(uhd::rx_streamer::sptr rx_stream,
    unsigned long long num_requested_samples,
    int flags[],
    samp_type buffs[][1016], 
    size_t num_rx[],
    int multiplier,
    std::vector<bool> * bools,
    double time_requested = 0.0){

    uhd::rx_metadata_t md;

    //unpack pointers from vector
    int * r_pointer = &(flags[0]);
    int * o_pointer = &(flags[1]);
    int * r_lap = &(flags[2]);
    int * o_lap = &(flags[3]);
    int * o_started = &(flags[4]);
    int * r_done = &(flags[5]);

    //unpack input bools
    bool bw_summary = (*bools).at(0);
    bool stats = (*bools).at(1);
    bool enable_size_map = (*bools).at(2);
    bool continue_on_bad_packet = (*bools).at(3);
    
    bool one_packet = true; //receive only one packet at a time-- may help prevent overflows?
    bool overflow_message = true;

    //wait for output to start, if necessary
    while(*o_started == 0) {
        boost::this_thread::sleep_for(boost::chrono::nanoseconds(10));
        
    }

    //std::cout << "receive: about to start streaming" << std::endl;

    //setup streaming
    uhd::stream_cmd_t stream_cmd((num_requested_samples == 0)?
        uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS:
        uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE
    );
    stream_cmd.num_samps = size_t(num_requested_samples);
    stream_cmd.stream_now = true;
    stream_cmd.time_spec = uhd::time_spec_t();
    rx_stream->issue_stream_cmd(stream_cmd); //start streaming

    unsigned long long num_total_samps = 0;

    boost::system_time start = boost::get_system_time();
    unsigned long long ticks_requested = (long)(time_requested * (double)boost::posix_time::time_duration::ticks_per_second());
    boost::posix_time::time_duration ticks_diff;
    boost::system_time last_update = start;
    unsigned long long last_update_samps = 0;

    typedef std::map<size_t,size_t> SizeMap;
    SizeMap mapSizes;
    
    //print decimator
    int a = -1;
    bool overflow = false;

    //while receiving
    while(not stop_signal_called and (num_requested_samples != num_total_samps or num_requested_samples == 0)) {

        boost::system_time now = boost::get_system_time();
        
        //record samples
        size_t num_rx_samps = rx_stream->recv(&buffs[*r_pointer][0], 1016, md, 3.0, (enable_size_map || one_packet));
        num_rx[*r_pointer] = num_rx_samps;

        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
            std::cout << boost::format("Timeout while streaming") << std::endl;
            break;
        }
        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW && !md.out_of_sequence){
            if (overflow_message) {
                overflow_message = false;
                std::cerr << boost::format(
                    "Overflow on the receiver side.\n"
                );
            }
            std::cerr << "V";
            overflow = true;
            break; //when overflow occurs, quit taking samples.
        }
        if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE){
            std::string error = str(boost::format("Receiver error: %s") % md.strerror());
            if (continue_on_bad_packet){
                std::cerr << error << std::endl;
                continue;
            }
            else
                throw std::runtime_error(error);
        }

        //size map stats
        if (enable_size_map) {
            SizeMap::iterator it = mapSizes.find(num_rx_samps);
            if (it == mapSizes.end())
                mapSizes[num_rx_samps] = 0;
            mapSizes[num_rx_samps] += 1;
        }

        num_total_samps += num_rx_samps;

        //if user asked for summary outputs, give them
        if (bw_summary) {
            last_update_samps += num_rx_samps;
            boost::posix_time::time_duration update_diff = now - last_update;
            if (update_diff.ticks() > boost::posix_time::time_duration::ticks_per_second()) {
                double t = (double)update_diff.ticks() / (double)boost::posix_time::time_duration::ticks_per_second();
                double r = (double)last_update_samps / t;
                std::cout << boost::format("\t%f Msps") % (r/1e6) << std::endl;
                last_update_samps = 0;
                last_update = now;
            }
        }

        ticks_diff = now - start;
        if (ticks_requested > 0){
            if ((unsigned long long)ticks_diff.ticks() > ticks_requested)
                break;
        }

        //update counters
        *r_pointer = (*r_pointer + 1) % multiplier;
        if (*r_pointer == 0) {*r_lap += 1;}
 
        //recognize overflow and stop streaming
        if ((*r_lap > *o_lap) && (*r_pointer == *o_pointer)){ //if out of space / similar
            if (a == -1){std::cerr << "Overflow on the file writing side: " << *r_lap << " > " << *o_lap  << " at " << *r_pointer << std::endl;}
            //if (a == 0) {std::cout << "A";}
            //a = (a + 1) % 100;
            overflow = true;
            break; //quit
        }
    }

    //set flag and end streaming
    *r_done = 1;

    stream_cmd.stream_mode = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
    rx_stream->issue_stream_cmd(stream_cmd);

    //clear and throw away remaining stream buffer so that the next frequency file doesn't contain these samples
    samp_type garbage[1016] = {};
    while(md.error_code != uhd::rx_metadata_t::ERROR_CODE_TIMEOUT){
        rx_stream->recv(&garbage[0], 1016, md, 0.1, true);
    }

    if (overflow) {
        //if recording was stopped early
        std::cout << "Stopped recording, waiting 5 seconds..." << std::endl;
        boost::this_thread::sleep_for(boost::chrono::seconds(5));
    }

    //if necessary, print statistics
    if (stats) {
        std::cout << std::endl;

        double t = (double)ticks_diff.ticks() / (double)boost::posix_time::time_duration::ticks_per_second();
        std::cout << boost::format("Received %d samples in %f seconds") % num_total_samps % t << std::endl;
        double r = (double)num_total_samps / t;
        std::cout << boost::format("%f Msps") % (r/1e6) << std::endl;

        if (enable_size_map) {
            std::cout << std::endl;
            std::cout << "Packet size map (bytes: count)" << std::endl;
            for (SizeMap::iterator it = mapSizes.begin(); it != mapSizes.end(); it++)
                std::cout << it->first << ":\t" << it->second << std::endl;
        }
    }

}

//function for writing samples to file
//samples are read from the interim storage buffs[][]
//communicates with receive via boolean flags
//this function is permitted to write to o_ flags only
template<typename samp_type> void output(const std::string &file,
            int flags[],
            samp_type buffs[][1016], 
            size_t num_rx[],
            int multiplier){
    //get ready to write
    std::ofstream outfile;
    outfile.open(file.c_str(), std::ofstream::binary);

    //unpack pointers from vector
    int * r_pointer = &(flags[0]);
    int * o_pointer = &(flags[1]);
    int * r_lap = &(flags[2]);
    int * o_lap = &(flags[3]);
    int * o_started = &(flags[4]);
    int * r_done = &(flags[5]);

    //signal variables
    bool writing = true;

    //until receiver has signalled that it's done and lap and pointer markers are consistent
    int b = 0;
    int o = 0;
    *o_started = 1;
    while(writing){
        //if receiver is done and fully caught up
        if ((*r_done == 1) && (*r_lap == *o_lap) && (*r_pointer == *o_pointer)){
            writing = false; //leave
            continue;
        }
        if ((*r_lap == *o_lap) && (*r_pointer == *o_pointer)){ //if fully caught up to the receiver
            
            boost::this_thread::sleep_for(boost::chrono::nanoseconds(10));
            if (b==0){std::cout << "B";}
            b = (b+1) % 100;
            continue; //don't do anything for now
        }
        
        if (o == 0){std::cout << "o: " << *o_pointer << "   r: " << *r_pointer << std::endl;}
        o = (o + 1) % 100;

        //how many samples should be written?
        size_t num_rx_samps = num_rx[*o_pointer];

        //write samples to output file
        if (outfile.is_open())
            outfile.write((const char*)&buffs[*o_pointer][0], num_rx_samps*sizeof(samp_type));
        
        //update pointers
        *o_pointer = (*o_pointer + 1) % multiplier;
        if (*o_pointer == 0) {*o_lap += 1;}

    }

    
    

    if (outfile.is_open())
        outfile.close();
}

template<typename samp_type> void record(
    int multiplier,
    samp_type buffs[][1016],
    size_t num_rx[],
    uhd::rx_streamer::sptr rx_stream,
    const std::string &file,
    unsigned long long num_requested_samples,
    double time_requested = 0.0,
    bool bw_summary = false,
    bool stats = false,
    bool enable_size_map = false,
    bool continue_on_bad_packet = false
){

    //effectively atomic.
    // r_ variables are written only by r_thread, o_ variables are written only by o_thread
    int r_pointer = 0; //current value [0,multiplier) that will be filled next
    int o_pointer = 0; //current value [0,multiplier) that will be output and cleared next
    int r_lap = 0; //+= 1 when r_pointer passes end of buffs
    int o_lap = 0; //+= 1 when o_pointer passes end of buffs
    int o_started = 0; //becomes 1 when o_thread has begun looping
    int r_done = 0; //becomes 1 when the r_thread is about to terminate.

    //because thread takes a fixed number of arguments, stick variables in a vector
    int flags[6] = {r_pointer, o_pointer, r_lap, o_lap, o_started, r_done};

    //also other information to be passed
    std::vector<bool> bools(4);
    bools.at(0) = bw_summary;
    bools.at(1) = stats;
    bools.at(2) = enable_size_map;
    bools.at(3) = continue_on_bad_packet;


    //threads
    //start the relevant thread
    boost::thread o_thread = boost::thread(output<samp_type>, file,
        flags, buffs, num_rx, multiplier);

    //wait for output to start, if necessary
/*    while(flags[4] == 0) { //o_started == 0
        boost::this_thread::sleep_for(boost::chrono::nanoseconds(10));
        
    }*/

    boost::thread r_thread = boost::thread(receive<samp_type>, rx_stream,
        num_requested_samples,
        flags, buffs, num_rx, multiplier, &bools, time_requested);

    o_thread.join(); //don't exit until output is done writing

}


typedef boost::function<uhd::sensor_value_t (const std::string&)> get_sensor_fn_t;

bool check_locked_sensor(std::vector<std::string> sensor_names, const char* sensor_name, get_sensor_fn_t get_sensor_fn, double setup_time){
    if (std::find(sensor_names.begin(), sensor_names.end(), sensor_name) == sensor_names.end())
        return false;

    boost::system_time start = boost::get_system_time();
    boost::system_time first_lock_time;

    std::cout << boost::format("Waiting for \"%s\": ") % sensor_name;
    std::cout.flush();

    while (true) {
        if ((not first_lock_time.is_not_a_date_time()) and
                (boost::get_system_time() > (first_lock_time + boost::posix_time::seconds(setup_time))))
        {
            std::cout << " locked." << std::endl;
            break;
        }
        if (get_sensor_fn(sensor_name).to_bool()){
            if (first_lock_time.is_not_a_date_time())
                first_lock_time = boost::get_system_time();
            std::cout << "+";
            std::cout.flush();
        }
        else {
            first_lock_time = boost::system_time();	//reset to 'not a date time'

            if (boost::get_system_time() > (start + boost::posix_time::seconds(setup_time))){
                std::cout << std::endl;
                throw std::runtime_error(str(boost::format("timed out waiting for consecutive locks on sensor \"%s\"") % sensor_name));
            }
            std::cout << "_";
            std::cout.flush();
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    std::cout << std::endl;
    return true;
}

int UHD_SAFE_MAIN(int argc, char *argv[]){
    uhd::set_thread_priority_safe();

    //variables to be set by po
    std::string args, type, ant, subdev, ref, wirefmt, filebase;
    size_t total_num_samps;
//    int spb;
    double rate, start, end, step, gain, bw_mhz, setup_time, total_time;
    double freq; //current frequency
    std::string filename;
    
    total_num_samps = 0;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "multi uhd device address args")
        ("file", po::value<std::string>(&filebase)->default_value("samples"), "base file location")
        ("type", po::value<std::string>(&type)->default_value("short"), "sample type: double, float, or short")
//        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(0), "total number of samples to receive")
        ("duration", po::value<double>(&total_time)->default_value(0.1), "total number of seconds to receive")
//        ("spb", po::value<int>(&spb)->default_value(10000), "samples per buffer")
        ("rate", po::value<double>(&rate)->default_value(16e6), "rate of incoming samples")
        ("start", po::value<double>(&start)->default_value(72.0), "RF start frequency in MHz")
        ("end", po::value<double>(&end)->default_value(400.0), "RF end frequency in MHz")
        ("step", po::value<double>(&step)->default_value(4.0), "RF step frequency in MHz")
        ("gain", po::value<double>(&gain)->default_value(0.0), "gain for the RF chain")
        ("ant", po::value<std::string>(&ant)->default_value("RX2"), "antenna selection")
        ("subdev", po::value<std::string>(&subdev)->default_value("A:A"), "subdevice specification")
        ("bw", po::value<double>(&bw_mhz)->default_value(8.0), "analog frontend filter bandwidth in MHz")
        ("ref", po::value<std::string>(&ref)->default_value("internal"), "reference source (internal, external, mimo)")
        ("wirefmt", po::value<std::string>(&wirefmt)->default_value("sc16"), "wire format (sc8 or sc16)")
        ("setup", po::value<double>(&setup_time)->default_value(1.0), "seconds of setup time")
        ("progress", "periodically display short-term bandwidth")
        ("stats", "show average bandwidth on exit")
        ("sizemap", "track packet size and display breakdown on exit")
        ("continue", "don't abort on a bad packet")
        ("skip-lo", "skip checking LO lock status")
        ("int-n", "tune USRP with integer-N tuning")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //scaling
    double bw = bw_mhz*1e6;

    //print the help message
    if (vm.count("help")) {
        std::cout << boost::format("Take cage samples %s") % desc << std::endl;
        std::cout
            << std::endl
            << "This application streams data from a USRP device, throughout the specified freq range, to a set of files.\n"
            << std::endl;
        return ~0;
    }

    bool bw_summary = vm.count("progress") > 0;
    bool stats = vm.count("stats") > 0;
    bool enable_size_map = vm.count("sizemap") > 0;
    bool continue_on_bad_packet = vm.count("continue") > 0;

    if (enable_size_map)
        std::cout << "Packet size tracking enabled - will only recv one packet at a time!" << std::endl;

    //create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    //Lock mboard clocks
    usrp->set_clock_source(ref);

    //always select the subdevice first, the channel mapping affects the other settings
    if (vm.count("subdev")) usrp->set_rx_subdev_spec(subdev);

    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

    //set the sample rate
    if (rate <= 0.0){
        std::cerr << "Please specify a valid sample rate" << std::endl;
        return ~0;
    }
    std::cout << boost::format("Setting RX Rate: %f Msps...") % (rate/1e6) << std::endl;
    usrp->set_rx_rate(rate);
    std::cout << boost::format("Actual RX Rate: %f Msps...") % (usrp->get_rx_rate()/1e6) << std::endl << std::endl;

    //set the rf gain
    if (vm.count("gain")) {
        std::cout << boost::format("Setting RX Gain: %f dB...") % gain << std::endl;
        usrp->set_rx_gain(gain);
        std::cout << boost::format("Actual RX Gain: %f dB...") % usrp->get_rx_gain() << std::endl << std::endl;
    }

    //set the IF filter bandwidth
    if (vm.count("bw")) {
        std::cout << boost::format("Setting RX Bandwidth: %f MHz...") % (bw/1e6) << std::endl;
        usrp->set_rx_bandwidth(bw);
        std::cout << boost::format("Actual RX Bandwidth: %f MHz...") % (usrp->get_rx_bandwidth()/1e6) << std::endl << std::endl;
    }


    //set the antenna
    if (vm.count("ant")) usrp->set_rx_antenna(ant);


//start of receiving data
    freq = start*1e6;
    double rate_ms = usrp->get_rx_rate()/1e6;

    
        
    //format
    std::string format;
    if (type == "double") format = "fc64";
    else if (type == "float") format = "fc32";
    else if (type == "short") format = "sc16";
    else throw std::runtime_error("Unknown type " + type);

    //create a receive streamer
    uhd::stream_args_t stream_args(format,wirefmt);
    uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);

    //samps_per_buff = (rx_stream->get_max_num_samps())*2;
    std::cout << "Samples per Buffer:" << (samps_per_buff) << std::endl;

    //std::signal(SIGINT, &sig_int_handler);
    struct sigaction action;
    action.sa_handler = sig_int_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    
    sigaction(SIGINT, NULL, &action);

    std::cout << "Press Ctrl + C to stop streaming..." << std::endl;

    //storing data
    int multiplier = 950;
    //storing numbers of received samples
    size_t num_rx[multiplier] = {};
    
    //argument macros
    #define record_args(file, buffs) \
            (multiplier, buffs, num_rx, rx_stream, file, total_num_samps, total_time, bw_summary, stats, enable_size_map, continue_on_bad_packet)

    //for every requested frequency
    while (freq <= end*1e6) {
        //set variables
        filename = filebase + "" + boost::lexical_cast<std::string>(int(freq/1e6)) + "mhz_" + boost::lexical_cast<std::string>(int(rate_ms)) + "MS.dat";


        //set the current frequency
        std::cout << boost::format("Setting RX Freq: %f MHz...") % (freq/1e6) << std::endl;
        uhd::tune_request_t tune_request(freq);
        if(vm.count("int-n")) tune_request.args = uhd::device_addr_t("mode_n=integer");
        usrp->set_rx_freq(tune_request);
        std::cout << boost::format("Actual RX Freq: %f MHz...") % (usrp->get_rx_freq()/1e6) << std::endl << std::endl;

        boost::this_thread::sleep(boost::posix_time::seconds(setup_time)); //allow for some setup time

        //std::cout << "Setup time passed" << std::endl;

        //check Ref and LO Lock detect
        if (not vm.count("skip-lo")){
            check_locked_sensor(usrp->get_rx_sensor_names(0), "lo_locked", boost::bind(&uhd::usrp::multi_usrp::get_rx_sensor, usrp, _1, 0), setup_time);
            if (ref == "mimo")
                check_locked_sensor(usrp->get_mboard_sensor_names(0), "mimo_locked", boost::bind(&uhd::usrp::multi_usrp::get_mboard_sensor, usrp, _1, 0), setup_time);
            if (ref == "external")
                check_locked_sensor(usrp->get_mboard_sensor_names(0), "ref_locked", boost::bind(&uhd::usrp::multi_usrp::get_mboard_sensor, usrp, _1, 0), setup_time);
        }

        //std::cout << "Done checking Ref and LO Lock detect" << std::endl;

        //record for this frequency
        if (type == "double") { 
            std::complex<double> buffs [multiplier][1016] = {};
            record<std::complex<double> >record_args(filename, buffs);
        }
        else if (type == "float") {
            std::complex<float> buffs [multiplier][1016] = {};
            record<std::complex<float> >record_args(filename, buffs);
        }
        else if (type == "short") {
            std::complex<short> buffs [multiplier][1016] = {};
            record<std::complex<short> >record_args(filename, buffs);
        }
        else throw std::runtime_error("Unknown type " + type);

        std::cout << std::endl;

        //print progress
        std::cout << filename << std::endl;
        
        //increment
        freq = freq + step*1e6;
    }




    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
