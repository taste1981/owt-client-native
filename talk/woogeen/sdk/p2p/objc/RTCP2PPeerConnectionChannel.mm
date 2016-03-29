/*
 * Intel License
 */

#import "Foundation/Foundation.h"
#import "RTCErrors.h"
#import "RTCP2PPeerConnectionChannelObserver.h"
#import "RTCP2PErrors.h"
#import "talk/app/webrtc/objc/RTCICEServer+Internal.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PPeerConnectionChannel.h"
#import "talk/woogeen/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"
#import "talk/woogeen/sdk/p2p/objc/RTCP2PSignalingSenderObjcImpl.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCMediaCodec+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"

#include "talk/woogeen/sdk/p2p/p2ppeerconnectionchannel.h"

@implementation RTCP2PPeerConnectionChannel {
  woogeen::p2p::P2PPeerConnectionChannel* _nativeChannel;
  NSString* _remoteId;
}

- (instancetype)initWithConfiguration:(RTCPeerClientConfiguration*)config
                              localId:(NSString*)localId
                             remoteId:(NSString*)remoteId
                      signalingSender:
                          (id<RTCP2PSignalingSenderProtocol>)signalingSender {
  self = [super init];
  woogeen::p2p::P2PSignalingSenderInterface* sender =
      new woogeen::p2p::RTCP2PSignalingSenderObjcImpl(signalingSender);
  _remoteId = remoteId;
  const std::string nativeRemoteId = [remoteId UTF8String];
  const std::string nativeLocalId = [localId UTF8String];
  webrtc::PeerConnectionInterface::IceServers nativeIceServers;
  for (RTCICEServer* server in config.ICEServers) {
    nativeIceServers.push_back(server.iceServer);
  }
  woogeen::p2p::PeerConnectionChannelConfiguration nativeConfig;
  nativeConfig.servers = nativeIceServers;
  nativeConfig.max_audio_bandwidth = [config maxAudioBandwidth];
  nativeConfig.max_video_bandwidth = [config maxVideoBandwidth];
  nativeConfig.media_codec.audio_codec =
      [RTCMediaCodec nativeAudioCodec:config.mediaCodec.audioCodec];
  nativeConfig.media_codec.video_codec =
      [RTCMediaCodec nativeVideoCodec:config.mediaCodec.videoCodec];
  _nativeChannel = new woogeen::p2p::P2PPeerConnectionChannel(
      nativeConfig, nativeLocalId, nativeRemoteId, sender);
  return self;
}

- (void)inviteWithOnSuccess:(void (^)())onSuccess
                  onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Invite(
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::p2p::P2PException> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:RTCErrorDomain
                      code:WoogeenP2PErrorUnknown
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              [NSString
                                  stringWithCString:e->Message().c_str()
                                           encoding:
                                               [NSString
                                                   defaultCStringEncoding]],
                              NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)denyWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Deny(
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::p2p::P2PException> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:RTCErrorDomain
                      code:WoogeenP2PErrorUnknown
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              [NSString
                                  stringWithCString:e->Message().c_str()
                                           encoding:
                                               [NSString
                                                   defaultCStringEncoding]],
                              NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)acceptWithOnSuccess:(void (^)())onSuccess
                  onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Accept(
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::p2p::P2PException> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:RTCErrorDomain
                      code:WoogeenP2PErrorUnknown
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              [NSString
                                  stringWithCString:e->Message().c_str()
                                           encoding:
                                               [NSString
                                                   defaultCStringEncoding]],
                              NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}
- (void)publish:(RTCLocalStream*)stream
      onSuccess:(void (^)())onSuccess
      onFailure:(void (^)(NSError*))onFailure {
  NSLog(@"RTCP2PPeerConnectionChannel publish stream.");
  _nativeChannel->Publish(
      std::static_pointer_cast<woogeen::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::p2p::P2PException> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:RTCErrorDomain
                      code:WoogeenP2PErrorUnknown
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              [NSString
                                  stringWithCString:e->Message().c_str()
                                           encoding:
                                               [NSString
                                                   defaultCStringEncoding]],
                              NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)unpublish:(RTCLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  NSLog(@"RTCP2PPeerConnectionChannel unpublish stream.");
  _nativeChannel->Unpublish(
      std::static_pointer_cast<woogeen::base::LocalStream>([stream nativeStream]),
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::p2p::P2PException> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:RTCErrorDomain
                      code:WoogeenP2PErrorUnknown
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              [NSString
                                  stringWithCString:e->Message().c_str()
                                           encoding:
                                               [NSString
                                                   defaultCStringEncoding]],
                              NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)send:(NSString*)message
    withOnSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Send(
      [message UTF8String],
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::p2p::P2PException> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:RTCErrorDomain
                      code:WoogeenP2PErrorUnknown
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              [NSString
                                  stringWithCString:e->Message().c_str()
                                           encoding:
                                               [NSString
                                                   defaultCStringEncoding]],
                              NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)stopWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure {
  _nativeChannel->Stop(
      [=]() {
        if (onSuccess != nil)
          onSuccess();
      },
      [=](std::unique_ptr<woogeen::p2p::P2PException> e) {
        if (onFailure == nil)
          return;
        NSError* err = [[NSError alloc]
            initWithDomain:RTCErrorDomain
                      code:WoogeenP2PErrorUnknown
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              [NSString
                                  stringWithCString:e->Message().c_str()
                                           encoding:
                                               [NSString
                                                   defaultCStringEncoding]],
                              NSLocalizedDescriptionKey, nil]];
        onFailure(err);
      });
}

- (void)onIncomingSignalingMessage:(NSString*)message {
  _nativeChannel->OnIncomingSignalingMessage([message UTF8String]);
}

- (void)addObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer {
  woogeen::p2p::P2PPeerConnectionChannelObserver* nativeObserver =
      new woogeen::p2p::P2PPeerConnectionChannelObserverObjcImpl(observer);
  _nativeChannel->AddObserver(nativeObserver);
}

- (void)removeObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer {
}

- (NSString*)getRemoteUserId {
  return _remoteId;
}

@end
