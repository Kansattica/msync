# msync
## A low bandwidth store and forward Mastodon API client

`msync` is a command line client for Mastodon (and anything else that implements the same API, including Pleroma) that works a little different. `msync` doesn't stay connected to the internet all the time and constantly pull new posts. Instead, it only connects when you use the `msync sync` command. Every other change is stored on your computer until then. You can queue up posts to send, boost, bookmark, and favorite when you're online, download posts, notifications, and bookmarks to look at offline, and do the same for as many accounts as you want.

`msync` currently supports queueing and sending posts, boosts, favorites, and bookmarks for any number of accounts, as well as downloading the home timeline, notifications, bookmarks, and arbitrary threads.

`msync` is for everyone, but it's really designed for:

- Unreliable, slow, or not always-on internet connections.
- Computers or connections that can't handle the Mastodon web frontend.
- Systems that run "headless" or otherwise can't use a graphical Mastodon client.
- Use in automated scripts.
- Anyone who prefers to use a keyboard.

### Get msync

You can download the latest stable release of `msync` [here](https://github.com/Kansattica/msync/releases) or the Releases link to the right. 

If you're feeling adventurous, you can order a [floppy disk release](https://princess.software/order/).

#### Which release should I download?

I recommend downloading the `.deb` file if you're on a x64 Debian-like system that can install it. Otherwise, I recommend downloading the release for your system with `install` in the filename. This version will store user data in the appropriate place for your system. The versions without `install` store user data in the same directory as the executable. See [the manual](MANUAL.md#a-note-about-msync_accounts) for more information.

If you're on OSX, the builds for 10.15 use std::filesystem APIs that are only available on OSX 10.15. Not that you can download these any more- GitHub Actions removed support for pre-11 versions of macOS, so you'll have to build yourself if you're on an older macOS.

If you would like to build `msync` yourself, read on. If you have a fairly recent version of CMake (3.12 or later), you can simply clone the repo, make a `build` directory, run CMake inside, and CMake will download and build `msync` and its dependencies automatically. 

Arch Linux users can use the AUR packages [msync](https://aur.archlinux.org/packages/msync/) and [msync-git](https://aur.archlinux.org/packages/msync-git/). These are maintained by someone else and I cannot test them, but I have no reason to believe they won't work.

##### Notes on libcurl 

- If you're compiling `msync` from source, you'll need something like `libcurl4-gnutls-dev` or `libcurl4-openssl-dev` installed.
- If you'd rather have msync compile curl into itself, add `-DUSE_SYSTEM_CURL=OFF` after `-DCMAKE_BUILD_TYPE=Release`. This will automatically download and configure curl as part of the build process. If you go this route, I suggest having zlib (e.g. `zlib1g-dev`, optional but highly recommended) and an ssl library (e.g. `libssl-dev`, required) installed where curl can find them.

#### Building on Linux

On a new Debian-like system, the setup process looks something like this:

```
apt install cmake git gcc g++
git clone https://github.com/Kansattica/msync.git
cd msync
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DMSYNC_BUILD_TESTS=OFF -DMSYNC_USER_CONFIG=ON -DMSYNC_FILE_LOG=OFF 
cmake --build . --parallel
```

The full list of options that can be passed to CMake is [here](#relevant-cmake-flags).

The last two steps will take a while, but when you're done, you should see a `msync` executable in your folder, and that's all you need! 

Older Linux releases might require you to `apt install libstdc++-8-dev` or their equivalent to get the standard library features `msync` requires, such as std::filesystem. If that doesn't work, you may also need `g++-8` and to run `CC=gcc-8 CXX=g++-8 cmake [...]` instead.

#### Building on macOS

The instructions for building on Linux should mostly work, though note that Apple only officially supports std::filesystem on Xcode 11.1 and Catalina/10.15 and up. `msync` does support pre-10.15 versions with Boost::filesystem, which you can install through Homebrew. I don't have a Mac to test on, but I do ensure that it builds and all unit tests pass. 

I used to test on OSX 10.15, producing builds for both 10.14 with Boost::filesystem 1.72.0 and 10.15 with Apple's std::filesystem implementation. GitHub Actions stopped supporting earlier macOS versions, so I now build and test on macOS 11 and 12. I have no reason to believe that earlier versions won't still work, but I have no way to test that right now.

#### Building on Windows

On Windows, you'll want Visual Studio with the C++ development workload and CMake installed. After that, `git clone https://github.com/Kansattica/msync.git`, then open Visual Studio and go to `File > Open > CMake...` (some versions just have `File > Open > Folder...` instead, this will also work) and choose the `msync` repo you just downloaded. Visual Studio will take a while to download and configure everything, and then you should be able to pick `x64-Release` from the dropdown at the top, make sure the `msync.exe` target is selected, and build from there. 

If you want something lighter weight, I suspect you can install the [build tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) and build from the command line. I don't have as much experience with this, but you typically have to open the start menu, search for something called "developer command prompt", and then the CMake commands should be the same as the Linux commands up above.

#### Available builds

`msync` currently builds and all tests pass for the following compilers and environments:

- gcc and clang on 64-bit (x64) Ubuntu
- gcc on 32-bit (i386) Ubuntu
- gcc on arm64 Ubuntu (broken?)
- MSVC 2022 on 32 and 64-bit Windows 
- MacOS 11 with AppleClang 13
- MacOS 12 with AppleClang 14

- OSX 10.15 with Xcode 12 should still work, but GitHub Actions removed support and so I can't test it any more.
- OSX 10.14 with Boost::filesystem should still work, but I've since changed CI providers and can't test on 10.14 any more.

#### Relevant CMake flags

Pass these during the first call to CMake (next to `-DCMAKE_BUILD_TYPE=Release`) to configure `msync`'s behavior. Boolean options can either be set to `ON` or `OFF` like this: `-DMSYNC_FILE_LOG=OFF`

| Command             |   Type    | Default | Description |
|:-------------------:|:---------:|:-------:|--------------
| `MSYNC_BUILD_TESTS` | boolean   |  `ON`   | If `ON`, download Catch2 and build two test executables, `tests` and `net_tests`, will also be built. |
| `MSYNC_FILE_LOG`    | boolean   |  `ON`   | If `ON`, `msync` will create an `msync.log` file in the current directory whenever it runs with a record of what it did. | 
| `MSYNC_USER_CONFIG` | boolean   |  `OFF`  | If `ON`, `msync` will store account information in the default location for your system. On Windows, this is something like `C:\Users\username\AppData\Local`. On Linux and OSX, this is the `XDG_CONFIG_HOME` environment variable, if set, and `~/.config` otherwise. If this is `OFF`, `msync` will store information in the same directory as the executable.  |
|`MSYNC_DOWNLOAD_ZLIB`| boolean   |  `ON`   | If `ON` AND you're on Windows, CMake will download a built copy of zlib and statically link it to curl for compression. No effect on other platforms. |
| `USE_SYSTEM_CURL`   | boolean   |  `ON`   | If `ON` AND CMake can find `libcurl` on your system, `msync` will use that to perform network requests. If this is `OFF` OR CMake couldn't find `libcurl`, it will download, build, and statically link `libcurl` for you. |

#### Testing your build

To ensure that `msync` found and compiled its network dependencies correctly, run the CMake commands above without `-DMSYNC_BUILD_TESTS=OFF` (or, equivalently, `-DMSYNC_BUILD_TESTS=ON`). Then, run `./tests/net_tests`. This will determine whether `msync` can correctly make authenticated HTTPS requests and will print warnings if it cannot request and recieve compressed responses.

### Next steps

Once you have `msync` compiled, check out [MANUAL.md](MANUAL.md#msync-manual) for installation and usage information.

### Features (that are implemented or coming soon)

- [X] Multiple account support.
- [X] Queue posts, boosts, and favs.
- [X] Queue up threads before sending.
- [X] Send posts, boosts, and favs.
- [X] Download posts from home timeline.
- [ ] Download direct messages.
- [X] Download notifications.
- [ ] Download posts from lists.
- [ ] Download posts from your profile.
- [X] Post template file generation.
- [X] Attachment support.
- [ ] Following support.
- [X] Ability to exclude notifications by type.
- [X] Allow choosing a default account.
- [ ] Support for adding and removing filters.
- [X] Respecting rate limits.
- [X] Bookmark support.

### Roadmap (things for later)

- [ ] Nicknames for accounts.
- [ ] Dropping filtered posts.
- [ ] Access to moderation tools.
- [ ] Queue and send profile updates.
- [X] Download individual posts and threads by ID.
- [X] Poll support (polls download and render properly, voting not implemented yet).
- [ ] Nicer UI, like a minimal webpage to show downloaded posts in.
- [ ] Save sync progress when msync is killed with ctrl-C.
- [ ] [A proper manual for the weirder ins and outs of msync.](MANUAL.md)

### Build Dependencies

CMake will download all of these for you, except it'll use your system's libcurl if you have it and can't update your compiler or standard library. I recommend at least having an SSL/TLS library for CMake to find and dynamically link.
You can verify that your setup works by building tests and running `net_tests`.
- CMake 3.12 or newer
- a compiler with C++17 support
- a standard library with std::filesystem support (gcc libstdc++ 8, clang libc++ 7, MSVC 2017 15.7, or Xcode 11.0) or Boost::filesystem.
- [Nlohmann Json](https://github.com/nlohmann/json)
- [clipp](https://github.com/muellan/clipp)
- [CPR](https://github.com/whoshuu/cpr)
- [curl](https://github.com/curl/curl)
- [whereami](https://github.com/gpakosz/whereami) if `MSYNC_USER_CONFIG` is OFF.
- [zlib](https://www.zlib.net/) on Windows if `MSYNC_DOWNLOAD_ZLIB` is ON (defaults to ON).
- [catch2](https://github.com/catchorg/Catch2) if `MSYNC_BUILD_TESTS` is ON (defaults to ON).

### License

GPLv3 for now, also fascists fuck off.
