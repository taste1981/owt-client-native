// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_WIN_VIDEORENDERER_WIN_H
#define OWT_BASE_WIN_VIDEORENDERER_WIN_H
#include <Windows.h>
#include <atlbase.h>
#include <codecapi.h>
#include <combaseapi.h>
#include <d3d9.h>
#include <d3d9types.h>
#include <dxva2api.h>
#include "system_wrappers/include/clock.h"
#include "system_wrappers/include/field_trial.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/api/video/video_sink_interface.h"
#include "webrtc/rtc_base/scoped_ref_ptr.h"
namespace owt {
namespace base {
class WebrtcVideoRendererD3D9Impl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererD3D9Impl(HWND wnd)
      : wnd_(wnd), inited_for_raw_(false), width_(0), height_(0) {
    render_latency_ = nullptr;
    if (webrtc::field_trial::IsEnabled("OWT-Log-Latency-To-File")) {
      char dump_file_name[128];
      snprintf(dump_file_name, 128, "render_latency_vrw-%p.txt", this);
      render_latency_ = fopen(dump_file_name, "w+");
    }
  }
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererD3D9Impl() {
    Destroy();
    if (render_latency_ != nullptr) {
      fclose(render_latency_);
      render_latency_ = nullptr;
    }
  }

 private:
  void Destroy();
  void Resize(size_t width, size_t height);
  HWND wnd_;
  bool inited_for_raw_;
  size_t width_;
  size_t height_;
  rtc::scoped_refptr<IDirect3D9> m_d3d_;
  rtc::scoped_refptr<IDirect3DDevice9> m_d3d_device_;
  rtc::scoped_refptr<IDirect3DTexture9> m_texture_;
  rtc::scoped_refptr<IDirect3DVertexBuffer9> m_vertex_buffer_;
  FILE* render_latency_;
#ifdef INTEL_TELEMETRY
  uint64_t last_render_invoke_ms_ = 0;
#endif
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_VIDEORENDERER_WIN_H
