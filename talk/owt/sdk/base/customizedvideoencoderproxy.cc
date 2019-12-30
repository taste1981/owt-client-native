// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/customizedvideoencoderproxy.h"
#include <string>
#include <vector>
#include "talk/owt/sdk/base/customizedencoderbufferhandle.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/include/cpp/owt/base/commontypes.h"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/common_types.h"
#include "webrtc/modules/include/module_common_types.h"
#include "webrtc/modules/video_coding/include/video_codec_interface.h"
#include "webrtc/modules/video_coding/include/video_error_codes.h"
#include "webrtc/rtc_base/buffer.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
// H.264 start code length.
#define H264_SC_LENGTH 4
// Maximum allowed NALUs in one output frame.
#define MAX_NALUS_PERFRAME 32
using namespace rtc;
namespace owt {
namespace base {
CustomizedVideoEncoderProxy::CustomizedVideoEncoderProxy(
    webrtc::VideoCodecType type)
    : callback_(nullptr) {
  codec_type_ = type;
  picture_id_ = 0;
}
CustomizedVideoEncoderProxy::~CustomizedVideoEncoderProxy() {
  if (encoder_event_callback_) {
    encoder_event_callback_ = nullptr;
  }
}
int CustomizedVideoEncoderProxy::InitEncode(
    const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
  RTC_DCHECK(codec_settings);
  RTC_DCHECK_EQ(codec_settings->codecType, codec_type_);
  width_ = codec_settings->width;
  height_ = codec_settings->height;
  bitrate_ = codec_settings->startBitrate * 1000;
  picture_id_ = static_cast<uint16_t>(rand()) & 0x7FFF;
  return WEBRTC_VIDEO_CODEC_OK;
}
int CustomizedVideoEncoderProxy::Encode(
    const webrtc::VideoFrame& input_image,
    const webrtc::CodecSpecificInfo* codec_specific_info,
    const std::vector<webrtc::FrameType>* frame_types) {
  CustomizedEncoderBufferHandle2* encoder_buffer_handle =
      reinterpret_cast<CustomizedEncoderBufferHandle2*>(
          static_cast<owt::base::EncodedFrameBuffer2*>(
              input_image.video_frame_buffer().get())
              ->native_handle());
  if (encoder_buffer_handle == nullptr ||
      encoder_buffer_handle->buffer_ == nullptr ||
      encoder_buffer_handle->buffer_length_ == 0) {
    RTC_LOG(LS_ERROR) << "Received invalid encoded frame.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Set encoder event callback object if not done already.
  if (encoder_event_callback_ == nullptr) {
    encoder_event_callback_ = encoder_buffer_handle->encoder_event_callback_;
  }

  // Check codec type before proceeding.
  {  // normal case.
#ifndef DISABLE_H265
    if (codec_type_ != webrtc::kVideoCodecH264 &&
        codec_type_ != webrtc::kVideoCodecVP8 &&
        codec_type_ != webrtc::kVideoCodecVP9 &&
        codec_type_ != webrtc::kVideoCodecH265)
#else
    if (codec_type_ != webrtc::kVideoCodecH264 &&
        codec_type_ != webrtc::kVideoCodecVP8 &&
        codec_type_ != webrtc::kVideoCodecVP9)
#endif
      return WEBRTC_VIDEO_CODEC_ERROR;
  }
  std::vector<uint8_t> buffer;
  bool request_key_frame = false;
  if (frame_types) {
    for (auto frame_type : *frame_types) {
      if (frame_type == webrtc::kVideoFrameKey) {
        request_key_frame = true;
        break;
      }
    }
  }

  if (encoder_event_callback_ != nullptr && request_key_frame) {
    encoder_event_callback_->RequestKeyFrame();
  }

  std::unique_ptr<uint8_t[]> data(new uint8_t[encoder_buffer_handle->buffer_length_]);
  uint8_t* data_ptr = data.get();
  uint32_t data_size =
      static_cast<uint32_t>(encoder_buffer_handle->buffer_length_);
  memcpy(data_ptr, encoder_buffer_handle->buffer_,
         encoder_buffer_handle->buffer_length_);
  webrtc::EncodedImage encodedframe(data_ptr, data_size, data_size);

  encodedframe._encodedWidth = input_image.width();
  encodedframe._encodedHeight = input_image.height();
  encodedframe._completeFrame = true;
  encodedframe.capture_time_ms_ =
      /*input_image.render_time_ms()*/ encoder_buffer_handle->meta_data_
          .capture_timestamp;
  encodedframe._timeStamp = input_image.timestamp();
  encodedframe.playout_delay_.min_ms = 0;
  encodedframe.playout_delay_.max_ms = 0;
  encodedframe.timing_.encode_start_ms =
      encoder_buffer_handle->meta_data_.encoding_start;
  encodedframe.timing_.encode_finish_ms =
      encoder_buffer_handle->meta_data_.encoding_end;
  encodedframe.timing_.flags = webrtc::VideoSendTiming::kTriggeredByTimer;
  if (!update_ts_) {
    encodedframe.capture_time_ms_ = last_capture_timestamp_;
    encodedframe._timeStamp = last_timestamp_;
  } else {
    last_capture_timestamp_ = encodedframe.capture_time_ms_;
    last_timestamp_ = encodedframe._timeStamp;
  }

  if (encoder_buffer_handle->meta_data_.last_fragment)
    update_ts_ = true;
  else
    update_ts_ = false;

  // VP9 requires setting the frame type according to actual frame type.
  if (codec_type_ == webrtc::kVideoCodecVP9 && data_size > 2) {
    uint8_t au_key = 1;
    uint8_t first_byte = data_ptr[0], second_byte = data_ptr[1];
    uint8_t shift_bits = 4, profile = (first_byte >> shift_bits) & 0x3;
    shift_bits = (profile == 3) ? 2 : 3;
    uint8_t show_existing_frame = (first_byte >> shift_bits) & 0x1;
    if (profile == 3 && show_existing_frame) {
      au_key = (second_byte >> 6) & 0x1;
    } else if (profile == 3 && !show_existing_frame) {
      au_key = (first_byte >> 1) & 0x1;
    } else if (profile != 3 && show_existing_frame) {
      au_key = second_byte >> 7;
    } else {
      au_key = (first_byte >> 2) & 0x1;
    }
    encodedframe._frameType = (au_key == 0) ? kVideoFrameKey : kVideoFrameDelta;
  }
  webrtc::CodecSpecificInfo info;
  memset(&info, 0, sizeof(info));
  info.codecType = codec_type_;
  if (codec_type_ == webrtc::kVideoCodecVP8) {
    info.codecSpecific.VP8.nonReference = false;
    info.codecSpecific.VP8.simulcastIdx = 0;
    info.codecSpecific.VP8.temporalIdx = webrtc::kNoTemporalIdx;
    info.codecSpecific.VP8.layerSync = false;
    info.codecSpecific.VP8.keyIdx = webrtc::kNoKeyIdx;
    picture_id_ = (picture_id_ + 1) & 0x7FFF;
  } else if (codec_type_ == webrtc::kVideoCodecVP9) {
    // TODO(jianlin): this is still not sufficient to enable
    // encoded vp9 input. when ss_data_available is true,
    // more info is needed. We may need a parser here.
    info.codecSpecific.VP9.inter_pic_predicted =
        (encodedframe._frameType == kVideoFrameKey);
    info.codecSpecific.VP9.flexible_mode = false;
    info.codecSpecific.VP9.inter_layer_predicted = false;
    info.codecSpecific.VP9.temporal_up_switch = false;
    info.codecSpecific.VP9.gof_idx = kNoGofIdx;
    info.codecSpecific.VP9.ss_data_available = false;
    info.codecSpecific.VP9.num_spatial_layers = 1;
    info.codecSpecific.VP9.num_ref_pics = 0;
    info.codecSpecific.VP9.height[0] = encodedframe._encodedHeight;
    info.codecSpecific.VP9.width[0] = encodedframe._encodedWidth;
    info.codecSpecific.VP9.spatial_idx = kNoSpatialIdx;
    info.codecSpecific.VP9.temporal_idx = kNoTemporalIdx;
  } else if (codec_type_ == webrtc::kVideoCodecH264) {
    info.codecSpecific.H264.picture_id =
        encoder_buffer_handle->meta_data_.picture_id;
    info.codecSpecific.H264.last_fragment_in_frame =
        encoder_buffer_handle->meta_data_.last_fragment;
  }
  // Generate a header describing a single fragment.
  webrtc::RTPFragmentationHeader header;
  memset(&header, 0, sizeof(header));
  if (codec_type_ == webrtc::kVideoCodecVP8 ||
      codec_type_ == webrtc::kVideoCodecVP9) {
    header.VerifyAndAllocateFragmentationHeader(1);
    header.fragmentationOffset[0] = 0;
    header.fragmentationLength[0] = encodedframe._length;
    header.fragmentationPlType[0] = 0;
    header.fragmentationTimeDiff[0] = 0;
#ifndef DISABLE_H265
  } else if (codec_type_ == webrtc::kVideoCodecH264 ||
             codec_type_ == webrtc::kVideoCodecH265) {
#else
  } else if (codec_type_ == webrtc::kVideoCodecH264) {
#endif
    // For H.264/H.265 search for start codes.
    int32_t scPositions[MAX_NALUS_PERFRAME + 1] = {};
    size_t scLengths[MAX_NALUS_PERFRAME + 1] = {};
    int32_t scPositionsLength = 0;
    int32_t scPosition = 0;
    while (scPositionsLength < MAX_NALUS_PERFRAME) {
      size_t scLength = 0;
      int32_t naluPosition = NextNaluPosition(
          data_ptr + scPosition, data_size - scPosition, &scLength);
      if (naluPosition < 0) {
        break;
      }
      scPosition += naluPosition;
      scPositions[scPositionsLength++] = scPosition;
      scLengths[scPositionsLength - 1] = static_cast<int32_t>(scLength);
      scPosition += static_cast<int32_t>(scLength);
    }
    if (scPositionsLength == 0) {
      RTC_LOG(LS_ERROR) << "Start code is not found for H264/H265 codec!";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    scPositions[scPositionsLength] = data_size;
    header.VerifyAndAllocateFragmentationHeader(scPositionsLength);
    for (int i = 0; i < scPositionsLength; i++) {
      header.fragmentationOffset[i] = scPositions[i] + scLengths[i];
      header.fragmentationLength[i] =
          scPositions[i + 1] - header.fragmentationOffset[i];
      header.fragmentationPlType[i] = 0;
      header.fragmentationTimeDiff[i] = 0;
    }
  }
  const auto result = callback_->OnEncodedImage(encodedframe, &info, &header);
  if (result.error != webrtc::EncodedImageCallback::Result::Error::OK) {
    RTC_LOG(LS_ERROR) << "Deliver encoded frame callback failed: "
                      << result.error;
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  return WEBRTC_VIDEO_CODEC_OK;
}
int CustomizedVideoEncoderProxy::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}
int CustomizedVideoEncoderProxy::SetChannelParameters(uint32_t packet_loss,
                                                      int64_t rtt) {
  return WEBRTC_VIDEO_CODEC_OK;
}
int CustomizedVideoEncoderProxy::SetRates(uint32_t new_bitrate_kbit,
                                          uint32_t frame_rate) {
  bitrate_ = new_bitrate_kbit * 1000;
  return WEBRTC_VIDEO_CODEC_OK;
}
bool CustomizedVideoEncoderProxy::SupportsNativeHandle() const {
  return true;
}
int CustomizedVideoEncoderProxy::Release() {
  callback_ = nullptr;
  if (encoder_event_callback_ != nullptr) {
    encoder_event_callback_ = nullptr;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}
int32_t CustomizedVideoEncoderProxy::NextNaluPosition(uint8_t* buffer,
                                                      size_t buffer_size,
                                                      size_t* sc_length) {
  if (buffer_size < H264_SC_LENGTH) {
    return -1;
  }
  uint8_t* head = buffer;
  // Set end buffer pointer to 4 bytes before actual buffer end so we can
  // access head[1], head[2] and head[3] in a loop without buffer overrun.
  uint8_t* end = buffer + buffer_size - H264_SC_LENGTH;
  while (head < end) {
    if (head[0]) {
      head++;
      continue;
    }
    if (head[1]) {  // got 00xx
      head += 2;
      continue;
    }
    if (head[2]) {  // got 0000xx
      if (head[2] == 0x01) {
        *sc_length = 3;
        return (int32_t)(head - buffer);
      }
      head += 3;
      continue;
    }
    if (head[3] != 0x01) {  // got 000000xx
      head++;               // xx != 1, continue searching.
      continue;
    }
    *sc_length = 4;
    return (int32_t)(head - buffer);
  }
  return -1;
}
}  // namespace base
}  // namespace owt
