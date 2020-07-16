# Open WebRTC Toolkit Native SDK

## Introduction
Open WebRTC Toolkit client SDK for native Windows/Linux/iOS applications are built upon the W3C WebRTC standard to accelerate the development of real time communication applications on these platforms. It supports peer to peer and conference mode communication working with Open Media Stream MCU server.

- Supported Windows platform: Windows 7 and above.
- Supported Linux platform: Ubuntu 16.04/18.04 with GCC 7 or higher GCC versions..
- Supported iOS platform: iOS 9.0 and above.

## Documentation
To generate the API document, go to scripts directory, and run `python build-win.py --docs` for Windows or `./gendoc.sh` in `talk/owt/docs/ios` for iOS.

You need [Doxygen](http://www.doxygen.nl/) in your path.

## How to build

### Prepare the development environment
Before you start, make sure you have following prerequisites installed/built:

- [WebRTC stack build dependencies](https://webrtc.org/native-code/development/prerequisite-sw/).
- This version of cloudgaming webrtc stack require Windows SDK version 16299 or higher to build. Make sure you have required version installed,
  and check in "Control Panel -> Programs and Features -> Windows Software Development Kit - Windows 10.0.16299.xx"(or any other version),
  right click and select "Change", check "Debugging tools for Windows" if not already.
- If you do not have direct Internet access to Google repositories and storages, please make sure you set HTTPS_PROXY in your system enviroment;
- Also make sure NO_AUTH_BOTO_CONFIG is set to correct boto file if you're using https proxy for gsutil downloading files from Google storage;
- Make sure DEPOT_TOOLS_WIN_TOOLCHAIN is set to 0 in your system environment if you're building for Windows.

### Get the code
- Make sure you clone the source code to `src` dir, that is: that is `git clone -b cloudgaming-83 https://github.com/taste1981/oms-client-native src`
  (Be noted the top dir of the source must be named 'src').
- Create file named .gclient in the same level directory as `src` dir, with below contents:

```
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
```

### Build
#### Windows
- Run `gclient sync` in the directory that contains 'src'. It may take a long time to download large amount of data. It may well fail when downloading build tools from Google storage. Check your boto file settings and proxy settings, and re-run `gclient sync` to retry.
At the end of gclient sync process, the patches under `talk/owt/patches/` will be applied. Make sure the patches are correctly applied before you proceed with the build process.

- Go to 'src' directory, and run `gn args out/release-x64' for release build. On the prompted config setting, set: 
````
rtc_use_h264 = true
rtc_use_h265 = true
is_component_build = false
use_lld = false
rtc_include_tests = false
owt_include_tests = false
rtc_build_examples = false
rtc_enable_protobuf = false
ffmpeg_branding = "Chrome"
use_rtti = false
is_clang = false
treat_warnings_as_errors = false
target_cpu = "x64"
is_debug = false
````
- If you wish to use the Intel GPRA BWE library, you need to add three additional variables, shown below. The lib root should be the directory that contains `gpra_lib.lib`, and the header root should be directory that contains `grpa_bwe.h`. Using the // syntax, the path is relative to your `src` directory. You can also use absolute paths.
```
owt_gpra_lib_root = "//path/to/libs/x64"
owt_gpra_header_root = "//path/to/include"
owt_use_gpra = true
```
- Run `ninja -C out/release-x64` to finish the build. Output owt.lib will be under out/release-x64/obj/owt/talk/owt.lib; rename it appropriately and copy to target project's lib directories.
- Copy the header files under src/talk/owt/sdk/include/cpp/ to the cloud-gaming project's include directories.

Please be noted the same stack can be used to build the client-side SDK as well. In that case, please add below extra config settigs when runnnig `gn args out/release-x64`:
````
owt_msdk_header_root = "c:\Program Files (x86)\IntelSWTools\Intel(R) Media SDK 2018 R2\Software Development Kit\\include"
owt_msdk_lib_root = "c:\Program Files (x86)\IntelSWTools\Intel(R) Media SDK 2018 R2\Software Development Kit\\lib\\x64"
````
Please modify the owt_msdk_header_root and owt_msdk_lib_root value according to actual Media SDK installation path. Latest Media SDK for Windows can be downloaded from: [Intel Media SDK](https://software.intel.com/en-us/media-sdk/).

Typically when building for client-side SDK we would require openssl to be used (For server-side, this can also be enabled).
In that case, please add following settings to your gn args (for example):
````
owt_use_openssl = true
owt_openssl_header_root = "C:\ssl_110h_64\include"
owt_openssl_lib_root = "C:\ssl_110h_64\lib"
````

You should point to openssl's installation dir instead of source dir here.

When you specify 'is_clang' to "false", it will by default build /MT version of the library. If you would like to create the /MD version, add below gn arg:
````
owt_msvc_build_md = true
````

## How to contribute
We warmly welcome community contributions to owt-client-native repository. If you are willing to contribute your features and ideas to OWT, follow the process below:

- Make sure your patch will not break anything, including all the build and tests.
- Submit a pull request onto [Pull Requests](https://github.com/taste1981/oms-client-native/pulls).
- Watch your patch for review comments if any, until it is accepted and merged.

OWT project is licensed under Apache License, Version 2.0. By contributing to the project, you **agree** to the license and copyright terms therein and release your contributions under these terms.

## How to report issues
Use the "Issues" tab on Github.

## See Also
http://webrtc.intel.com
 