# msync
## A low bandwidth store and forward Mastodon api client

[![Build Status](https://travis-ci.org/Kansattica/msync.svg?branch=master)](https://travis-ci.org/Kansattica/msync)

`msync` is a command line client for Mastodon (and anything else that implements the same API) that works a little different. `msync` doesn't stay connected to the internet all the time and constantly pull new posts. Instead, it only connects when you use the `msync sync` command. Every other change is stored on your computer until then. You can queue up posts to send, boost, and fav when you're online, download posts, DMs and notifications to look at offline, and do the same for as many accounts as you want.

`msync` currently supports queueing and sending posts, boosts, and favorites for any number of accounts, as well as downloading the home timeline and notifications.

### Get msync
You can download the latest stable release of `msync` [here](https://github.com/Kansattica/msync/releases) or the Releases tab above. 

If you would like to build `msync` yourself, read on. If you have a fairly recent version of CMake (3.12 or later), you can simply clone the repo, make a `build` directory, run CMake inside, and CMake will download and build `msync` and its dependencies automatically. 

On Linux systems, it works a lot better if it can link in your system's openssl (or whatever other TLS implementation cURL knows how to use) and, if you have it, libcurl. Consider installing these through your package manager. For example, I install `libcurl4-openssl-dev` on Debian.

##### Notes on libcurl 

- Some systems come with `libcurl4-gnutls-dev` preinstalled. This works as well- no need to uninstall it for the openssl version.
- If you'd rather have msync compile curl into itself, add `-DUSE_SYSTEM_CURL=OFF` after `-DCMAKE_BUILD_TYPE=Release`. This will automatically download and configure curl as part of the build process. If you go this route, I suggest having zlib (e.g. `zlib1g-dev`, optional but highly recommended) and an ssl library (e.g. `libssl-dev`, required) installed where curl can find them.

#### Building on Linux
On a new Debian-like system, the setup process looks something like this:

```
apt install libcurl4-openssl-dev cmake git gcc g++
git clone https://github.com/Kansattica/msync.git
cd msync
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DMSYNC_BUILD_TESTS=FALSE
cmake --build . --parallel
```

The last two steps will take a while, but when you're done, you should see a `msync` executable in your folder, and that's all you need! 

Older Ubuntu releases might require you to `apt install g++-8 libstdc++-8-dev` and run `CC=gcc-8 CXX=g++-8 cmake .. -DCMAKE_BUILD_TYPE=Release -DMSYNC_BUILD_TESTS=FALSE` instead, in order to get and use a version of the standard library that supports std::filesystem.

#### Building on macOS

The instructions for building on Linux should mostly work, though note that Apple only officially supports std::filesystem on Xcode 11.1 and Catalina/10.15 and up. `msync` does support pre-10.15 versions with Boost::filesystem, which you can install through Homebrew. I don't have a Mac to test on, but I do ensure that it builds and all unit tests pass. I test on OSX 10.14.6 with Xcode 11 and Boost::filesystem 1.72.0.

#### Building on Windows
On Windows, you'll want Visual Studio with the C++ development workload and CMake installed. After that, `git clone https://github.com/Kansattica/msync.git`, then open Visual Studio and go to `File > Open > CMake...` (some versions just have `File > Open > Folder...` instead, this will also work) and choose the `msync` repo you just downloaded. Visual Studio will take a while to download and configure everything, and then you should be able to pick `x64-Release` from the dropdown at the top, make sure the `msync.exe` target is selected, and build from there. 

If you want something lighter weight, I suspect you can install the [build tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) and build from the command line. I don't have as much experience with this, but you typically have to open the start menu, search for something called "developer command prompt", and then the CMake commands should be the same as the Linux commands up above.

#### Available builds

`msync` currently builds and all tests pass for the following compilers and environments:

- gcc and clang on 64-bit Ubuntu and Debian
- gcc on 32-bit Ubuntu
- OSX 10.14 with Xcode 11
- MSVC 2017 on 32 and 64-bit Windows 

#### Testing your build

To ensure that `msync` found and compiled its network dependencies correctly, run the CMake commands above without `-DMSYNC_BUILD_TESTS=FALSE` (or, equivalently, `-DMSYNC_BUILD_TESTS=TRUE`). Then, run `./tests/net_tests`. This will determine whether `msync` can correctly make authenticated HTTPS requests and will print warnings if it cannot request and recieve compressed responses.

### Next steps

Once you have `msync` compiled, check out [MANUAL.md](manual.md) for installation and usage information.

### Features (that are implemented or coming soon)
- [X] Multiple account support
- [X] Queue posts, boosts, and favs
- [X] Queue up threads before sending.
- [X] Send posts, boosts, and favs
- [X] Download posts from home timeline
- [ ] Download direct messages
- [X] Download notifications
- [ ] Download posts from lists
- [ ] Download posts from your profile
- [X] Post template file generation
- [X] Attachment support
- [ ] Following support
- [X] Ability to exclude notifications by type.
- [ ] Nicknames for accounts

### Roadmap (things for later)
- [ ] Queue and send profile updates
- [ ] Download individual posts and threads by URL or ID
- [X] Poll support (polls download and render properly, voting not implemented yet)
- [ ] Nicer UI, like a minimal webpage to show downloaded posts in.
- [ ] [A proper manual for the weirder ins and outs of msync](MANUAL.md)

### Build Dependencies
CMake will download all of these for you, except it'll use your system's libcurl if you have it and can't update your compiler or standard library. I recommend at least having an SSL/TLS library for CMake to find and dynamically link.
You can verify that your setup works by building tests and running `net_tests`.
- CMake 3.12 or newer
- a compiler with C++17 support
- a standard library with std::filesystem support (gcc libstdc++ 8, clang libc++ 7, MSVC 2017 15.7, or Xcode 11.0) or Boost::filesystem.
- [Nlohmann Json](https://github.com/nlohmann/json)
- [whereami](https://github.com/gpakosz/whereami)
- [clipp](https://github.com/muellan/clipp)
- [CPR](https://github.com/whoshuu/cpr)
- [curl](https://github.com/curl/curl)
- [zlib](https://www.zlib.net/) is downloaded on Windows if `MSYNC_DOWNLOAD_ZLIB` is true (defaults to true).
- [catch2](https://github.com/catchorg/Catch2) if `MSYNC_BUILD_TESTS` is true (defaults to true).

