/*
Copyright 2014 Google Inc. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "library/dash/box.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string>

#include "library/dash/avc1_contents.h"
#include "library/dash/avcc_contents.h"
#include "library/dash/box_contents.h"
#include "library/dash/cbmp_contents.h"
#include "library/dash/dash_parser.h"
#include "library/dash/elst_contents.h"
#include "library/dash/emsg_contents.h"
#include "library/dash/equi_contents.h"
#include "library/dash/esds_contents.h"
#include "library/dash/mdat_contents.h"
#include "library/dash/mdhd_contents.h"
#include "library/dash/mp4a_contents.h"
#include "library/dash/mshp_contents.h"
#include "library/dash/mvhd_contents.h"
#include "library/dash/prft_contents.h"
#include "library/dash/prhd_contents.h"
#include "library/dash/pssh_contents.h"
#include "library/dash/saio_contents.h"
#include "library/dash/saiz_contents.h"
#include "library/dash/sidx_contents.h"
#include "library/dash/sbgp_contents.h"
#include "library/dash/sgpd_contents.h"
#include "library/dash/st3d_contents.h"
#include "library/dash/stsd_contents.h"
#include "library/dash/stsz_contents.h"
#include "library/dash/tenc_contents.h"
#include "library/dash/tfdt_contents.h"
#include "library/dash/tfhd_contents.h"
#include "library/dash/trex_contents.h"
#include "library/dash/trun_contents.h"
#include "library/dash/unknown_contents.h"
#include "library/utilities.h"

namespace dash2hls {

Box::Box(uint64_t stream_position)
    : state_(kInitialState), size_(0), bytes_read_(0),
      stream_position_(stream_position) {
}

bool Box::DoneParsing() const {
  return state_ == kParsed;
}

size_t Box::BytesNeededToContinue() const {
  switch (state_) {
    case kInitialState: return sizeof(uint32_t);
    case kReadingType: return sizeof(uint32_t);
    case kReadingBytes: return size_ - bytes_read_;
    case kParsed: return 0;
    default:
      {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "State = %d\n", state_);
        DASH_LOG("Box in bad state",
                 "Unknown state while trying to find BytesNeededToContinue",
                 buffer);
        return 0;
      }
  }
}

void Box::CreateContentsObject() {
  switch (type_.asUint32()) {
    // Boxes that contain other boxes.
    case BoxType::kBox_dinf:
    case BoxType::kBox_edts:
    case BoxType::kBox_mdia:
    case BoxType::kBox_minf:
    case BoxType::kBox_moof:
    case BoxType::kBox_moov:
    case BoxType::kBox_mvex:
    case BoxType::kBox_proj:
    case BoxType::kBox_schi:
    case BoxType::kBox_sinf:
    case BoxType::kBox_stbl:
    case BoxType::kBox_sv3d:
    case BoxType::kBox_traf:
    case BoxType::kBox_trak:
      contents_.reset(new BoxContents(type_.asUint32(), stream_position_));
      break;
    case BoxType::kBox_encv:
    case BoxType::kBox_avc1:
      contents_.reset(new Avc1Contents(stream_position_));
      break;
    case BoxType::kBox_avcC:
      contents_.reset(new AvcCContents(stream_position_));
      break;
    case BoxType::kBox_cbmp:
      contents_.reset(new CbmpContents(stream_position_));
      break;
    case BoxType::kBox_elst:
      contents_.reset(new ElstContents(stream_position_));
      break;
    case BoxType::kBox_emsg:
      contents_.reset(new EmsgContents(stream_position_));
      break;
    case BoxType::kBox_equi:
      contents_.reset(new EquiContents(stream_position_));
      break;
    case BoxType::kBox_esds:
      contents_.reset(new EsdsContents(stream_position_));
      break;
    case BoxType::kBox_mdat:
      contents_.reset(new MdatContents(stream_position_));
      break;
    case BoxType::kBox_enca:
    case BoxType::kBox_mp4a:
      contents_.reset(new Mp4aContents(stream_position_));
      break;
    case BoxType::kBox_mdhd:
      contents_.reset(new MdhdContents(stream_position_));
      break;
    case BoxType::kBox_mshp:
    case BoxType::kBox_ytmp:
      contents_.reset(new MshpContents(stream_position_));
      break;
    case BoxType::kBox_mvhd:
      contents_.reset(new MvhdContents(stream_position_));
      break;
    case BoxType::kBox_sidx:
      contents_.reset(new SidxContents(stream_position_));
      break;
    case BoxType::kBox_prft:
      contents_.reset(new PrftContents(stream_position_));
      break;
    case BoxType::kBox_prhd:
      contents_.reset(new PrhdContents(stream_position_));
      break;
    case BoxType::kBox_pssh:
      contents_.reset(new PsshContents(stream_position_));
      break;
    case BoxType::kBox_saio:
      contents_.reset(new SaioContents(stream_position_));
      break;
    case BoxType::kBox_saiz:
      contents_.reset(new SaizContents(stream_position_));
      break;
    case BoxType::kBox_sbgp:
      contents_.reset(new SbgpContents(stream_position_));
      break;
    case BoxType::kBox_sgpd:
      contents_.reset(new SgpdContents(stream_position_));
      break;
    case BoxType::kBox_st3d:
      contents_.reset(new St3dContents(stream_position_));
      break;
    case BoxType::kBox_stsd:
      contents_.reset(new StsdContents(stream_position_));
      break;
    case BoxType::kBox_stsz:
      contents_.reset(new StszContents(stream_position_));
      break;
    case BoxType::kBox_tenc:
      contents_.reset(new TencContents(stream_position_));
      break;
    case BoxType::kBox_tfdt:
      contents_.reset(new TfdtContents(stream_position_));
      break;
    case BoxType::kBox_tfhd:
      contents_.reset(new TfhdContents(stream_position_));
      break;
    case BoxType::kBox_trex:
      contents_.reset(new TrexContents(stream_position_));
      break;
    case BoxType::kBox_trun:
      contents_.reset(new TrunContents(stream_position_));
      break;
    default:
      if (g_verbose_pretty_print) {
        printf("creating unknown box %s %x\n", type_.PrettyPrint("").c_str(),
               type_.asUint32());
      }
      contents_.reset(new UnknownContents(stream_position_));
      break;
  }
}

size_t Box::Parse(const uint8_t* buffer, size_t length) {
  size_t bytes_left = length;
  bool done_parsing = false;
  while (!done_parsing && (bytes_left >= BytesNeededToContinue())) {
    switch (state_) {
      case kInitialState:
        size_ = ntohlFromBuffer(buffer);
        if (size_ < sizeof(uint32_t) * 2) {
          DASH_LOG("Bad size in box.", "Box must be at least 8 bytes.",
                   DumpMemory(buffer, length).c_str());
          return 0;
        }
        state_ = kReadingType;
        bytes_left -= sizeof(uint32_t);
        bytes_read_ = sizeof(uint32_t);
        break;
      case kReadingType:
        type_.set_type(buffer);
        bytes_left -= 4;
        bytes_read_ += 4;
        state_ = kReadingBytes;
        break;
      case kReadingBytes:
        CreateContentsObject();
        bytes_left -= BytesNeededToContinue();
        bytes_read_ += BytesNeededToContinue();
        if (contents_->Parse(buffer, length) == 0) {
          return DashParser::kParseFailure;
        }
        state_ = kParsed;
        break;
      case kParsed:
        done_parsing = true;
        break;
      default:
        break;
    }
  }
  return length - bytes_left;
}

std::string Box::PrettyPrint(std::string indent) const {
  switch (state_) {
    case kInitialState: return "Unparsed Box\n";
    case kReadingType: return "Unparsed Box\n";
    case kReadingBytes:
      return PrettyPrintValue(stream_position_) + std::string("-Box<") +
          type_.PrettyPrint(indent + "  ") + ":" +
          PrettyPrintValue(size_) + "> still reading";
    case kParsed:
      return PrettyPrintValue(stream_position_) + std::string("-Box<") +
          type_.PrettyPrint(indent + " ") + ":" + PrettyPrintValue(size_) +
          " " + contents_->BoxName() + "> " +
          contents_->PrettyPrint(indent + " ");
    default:
      return "Box is strange state\n";
      break;
  }
  return "";
}

}  // namespace dash2hls
