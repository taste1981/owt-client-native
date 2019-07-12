# Open WebRTC Toolkit Native SDK

## Introduction
Open WebRTC Toolkit client SDK for native Windows/Linux/iOS applications are built upon the W3C WebRTC standard to accelerate the development of real time communication applications on these platforms. It supports peer to peer and conference mode communication working with Open Media Stream MCU server.

- Supported Windows platform: Windows 7 and above.
- Supported Linux platform: Ubuntu 16.04.
- Supported iOS platform: iOS 9.0 and above.

## Documentation
To generate the API document, go to scripts directory, and run `python build-win.py --docs` for Windows or `./gendoc.sh` in `talk/owt/docs/ios` for iOS.

You need [Doxygen](http://www.doxygen.nl/) in your path.

## How to build

### Prepare the development environment
Before you start, make sure you have following prerequisites installed/built:
- [WebRTC stack build dependencies](https://webrtc.org/native-code/development/prerequisite-sw/).

Following dependencies are for Windows only:
- [Intel Media SDK 2018 R2 or higher](https://software.intel.com/en-us/media-sdk/choose-download/client).

### Get the code
- Make sure you clone the source code to `src` dir. that is `git clone -b latency_measure https://github.com/taste1981/oms-client-native src`
- Create file named .gclient in the same directory as `src` dir, with below contents:

````
solutions = [ 
  {  
     "managed": False,  
     "name": "src",  
     "url": "https://github.com/taste1981/oms-client-native.git",  
     "custom_deps": {},  
     "deps_file": "DEPS",  
     "safesync_url": "",  
  },  
]  
target_os = []  
````
After that, the source tree looks like
````
--|--.gclient
  |--src
````

### Build
#### Windows
- Run `gclient sync`. It may take a long time to download large amount of data.
- To build the stack for client, go to 'src' directory, and run `gn args out/release-x64' for release build. On the prompted config setting, set: 
````
rtc_use_h264 = true
rtc_use_h265 = true
is_component_build = false
use_lld = false
rtc_include_tests = false
woogeen_include_tests = false
rtc_build_examples = false
target_cpu = "x64"
is_debug = false
````

- If you're building the stack for the client, besides the gn args listed above, you also need to add below gn args, and prior to build, make sure Media SDK 2018 R2 or higher is
alredy installed on your build system:
````
ffmpeg_branding = "Chrome"
woogeen_msdk_header_root = "c:\Program Files (x86)\IntelSWTools\Intel(R) Media SDK 2018 R2\Software Development Kit\include"
woogeen_msdk_lib_root = "c:\Program Files (x86)\IntelSWTools\Intel(R) Media SDK 2018 R2\Software Development Kit\lib\x64"
````
- Run `ninja -C out/release-x64` to finish the build. Output owt.lib will be under out/release-x64/obj/owt/talk/owt.lib; rename it to owt-release.lib for copying to cloud-gaming dependency directories.
- Copy the header files under src/talk/owt/sdk/include/cpp/ to the cloud-gaming include directories.

## How to contribute
We warmly welcome community contributions to owt-client-native repository. If you are willing to contribute your features and ideas to OWT, follow the process below:

- Make sure your patch will not break anything, including all the build and tests.
- Submit a pull request onto [Pull Requests](https://github.com/open-webrtc-toolkit/owt-client-native/pulls).
- Watch your patch for review comments if any, until it is accepted and merged.

OWT project is licensed under Apache License, Version 2.0. By contributing to the project, you **agree** to the license and copyright terms therein and release your contributions under these terms.

## How to report issues
Use the "Issues" tab on Github.

## See Also
http://webrtc.intel.com
 

