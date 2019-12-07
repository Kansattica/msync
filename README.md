# msync
## A low bandwidth store and forward Mastodon api client

`msync` is a command line client for Mastodon (and anything else that implements the same API) that works a little different. `msync` doesn't stay connected to the internet all the time and constantly pull new posts. Instead, it only connects when you use the `msync sync` command. Every other change is stored on your computer until then. You can queue up posts to send, boost, and fav when you're online, download posts, DMs and notifications to look at offline, and do the same for as many accounts as you want.

`msync` currently supports queueing and sending posts, boosts, and favorites for any number of accounts. Downloading timelines for later reading is on the list.

### Get msync
Once `msync` has a stable release, I'll have releases available for download. For now, it has to be built with cmake. If you have a fairly recent version of cmake (3.12 or later), you can simply clone the repo, make a `build` directory, run cmake inside, and cmake will download and build `msync` and its dependencies automatically. 

On Linux systems, it works a lot better if it can link in your system's openssl (or whatever other TLS implementation cURL knows how to use) and, if you have it, libcurl. Consider installing these through your package manager.

I currently build and ensure all tests pass on Linux with both clang and gcc, as well as the Microsoft compiler on Windows.

### Features (that are implemented or coming soon)
- [X] Multiple account support
- [X] Queue posts, boosts, and favs
- [X] Queue up threads before sending.
- [X] Send posts, boosts, and favs
- [ ] Download posts from home timeline
- [ ] Download direct messages
- [ ] Download notifications
- [ ] Download posts from lists
- [ ] Download posts from your profile
- [X] Post template file generation
- [X] Attachment support
- [ ] Nicknames for accounts

### Roadmap (things for later)
- [ ] Queue and send profile updates
- [ ] Download individual posts and threads by URL or ID
- [ ] Poll support
- [ ] Nicer UI, like a minimal webpage to show downloaded posts in.

### Build Dependencies
Cmake will download all of these for you, except it'll use your system's libcurl if you have it. I recommend at least having an SSL/TLS library for cmake to find and dynamically link.
You can verify that your setup works by building tests and running `net_tests`.
- [Nlohmann Json](https://github.com/nlohmann/json)
- [whereami](https://github.com/gpakosz/whereami)
- [clipp](https://github.com/muellan/clipp)
- [CPR](https://github.com/whoshuu/cpr)
- [curl](https://github.com/curl/curl)
- [catch2](https://github.com/catchorg/Catch2) if MSYNC_BUILD_TESTS is set.
