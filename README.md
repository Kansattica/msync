# msync
## A low bandwidth store and forward Mastodon api client

`msync` is a command line client for Mastodon (and anything else that implements the same API) that works a little different. `msync` doesn't stay connected to the internet all the time and constantly pull new posts. Instead, it only connects when you use the `msync sync` command. Every other change is stored on your computer until then. You can queue up posts to send, boost, and fav when you're online, download posts, DMs and notifications to look at offline, and do the same for as many accounts as you want.

`msync` currently supports queueing and sending posts, boosts, and favorites for any number of accounts, as well as downloading the home timeline and notifications.

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
- [X] Ability to exclude notifications by type.
- [ ] Nicknames for accounts

### Roadmap (things for later)
- [ ] Queue and send profile updates
- [ ] Download individual posts and threads by URL or ID
- [X] Poll support (polls download and render properly, voting not implemented yet)
- [ ] Nicer UI, like a minimal webpage to show downloaded posts in.
- [ ] A proper manual for the weirder ins and outs of msync


### Get msync
Once `msync` has a stable release, I'll have releases available for download. For now, it has to be built with cmake. If you have a fairly recent version of cmake (3.12 or later), you can simply clone the repo, make a `build` directory, run cmake inside, and cmake will download and build `msync` and its dependencies automatically. 

On Linux systems, it works a lot better if it can link in your system's openssl (or whatever other TLS implementation cURL knows how to use) and, if you have it, libcurl. Consider installing these through your package manager. For example, I install `libcurl4-openssl-dev` on Debian.


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


### Building on Windows
On Windows, you'll want Visual Studio with the C++ development workload and Cmake installed. After that, `git clone https://github.com/Kansattica/msync.git`, then open Visual Studio and go to `File > Open > CMake...` (some versions just have `File > Open > Folder...` instead, this will also work) and choose the `msync` repo you just downloaded. Visual Studio will take a while to download and configure everything, and then you should be able to pick `x64-Release` from the dropdown at the top, make sure the `msync.exe` target is selected, and build from there. 

If you want something lighter weight, I suspect you can install the [build tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) and build from the command line. I don't have as much experience with this, but the cmake commands should be the same as the Linux commands up above.

I currently build and ensure all tests pass on Linux with both clang and gcc, as well as the Microsoft compiler on Windows.

Once you have `msync` compiled, check out [MANUAL.md](manual.md) for usage information.

### Build Dependencies
Cmake will download all of these for you, except it'll use your system's libcurl if you have it. I recommend at least having an SSL/TLS library for cmake to find and dynamically link.
You can verify that your setup works by building tests and running `net_tests`.
- a compiler with C++17 support
- a standard library with std::filesystem support
- [Nlohmann Json](https://github.com/nlohmann/json)
- [whereami](https://github.com/gpakosz/whereami)
- [clipp](https://github.com/muellan/clipp)
- [CPR](https://github.com/whoshuu/cpr)
- [curl](https://github.com/curl/curl)
- [catch2](https://github.com/catchorg/Catch2) if MSYNC_BUILD_TESTS is set.
