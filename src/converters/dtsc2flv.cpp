/// \file dtsc2flv.cpp
/// Contains the code that will transform any valid DTSC input into valid FLVs.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <mist/flv_tag.h> //FLV support
#include <mist/dtsc.h> //DTSC support
#include <mist/amf.h> //AMF support
#include <mist/config.h>

///\brief Holds everything unique to converters.
namespace Converters {

  ///\brief Converts DTSC from stdin to FLV on stdout.
  ///\return The return code for the converter.
  int DTSC2FLV(){
    FLV::Tag FLV_out; // Temporary storage for outgoing FLV data.
    DTSC::Stream Strm;
    std::string inBuffer;
    char charBuffer[1024 * 10];
    unsigned int charCount;
    bool doneheader = false;
    int videoID = -1, audioID = -1;

    while (std::cin.good()){
      if (Strm.parsePacket(inBuffer)){
        if ( !doneheader){
          //find first audio and video tracks
          for (std::map<int,DTSC::Track>::iterator it = Strm.metadata.tracks.begin(); it != Strm.metadata.tracks.end(); it++){
            if (videoID == -1 && it->second.type == "video"){
              videoID = it->first;
            }
            if (audioID == -1 && it->second.type == "audio"){
              audioID = it->first;
            }
          }
          
          doneheader = true;
          std::cout.write(FLV::Header, 13);
          FLV_out.DTSCMetaInit(Strm, Strm.metadata.tracks[videoID], Strm.metadata.tracks[audioID]);
          std::cout.write(FLV_out.data, FLV_out.len);
          if (videoID != -1){
            FLV_out.DTSCVideoInit(Strm.metadata.tracks[videoID]);
            std::cout.write(FLV_out.data, FLV_out.len);
          }
          if (audioID != -1){
            FLV_out.DTSCAudioInit(Strm.metadata.tracks[audioID]);
            std::cout.write(FLV_out.data, FLV_out.len);
          }
        }
        if (FLV_out.DTSCLoader(Strm)){
          std::cout.write(FLV_out.data, FLV_out.len);
        }
      }else{
        std::cin.read(charBuffer, 1024 * 10);
        charCount = std::cin.gcount();
        inBuffer.append(charBuffer, charCount);
      }
    }

    std::cerr << "Done!" << std::endl;

    return 0;
  } //FLV2DTSC

} //Converter namespace

/// Entry point for DTSC2FLV, simply calls Converters::DTSC2FLV().
int main(int argc, char ** argv){
  Util::Config conf = Util::Config(argv[0], PACKAGE_VERSION);
  conf.parseArgs(argc, argv);
  return Converters::DTSC2FLV();
} //main
