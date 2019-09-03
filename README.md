# msync
## A low bandwidth store and forward Mastodon api client

`msync` is a command line client for Mastodon (and anything else that implements the same API) that works a little different. `msync` doesn't stay connected to the internet all the time and constantly pull new posts. Instead, it only connects when you use the `msync sync` command. Every other change is stored on your computer until then. You can queue up posts to send, boost, and fav when you're online, download posts, DMs and notifications to look at offline, and do the same for as many accounts as you want.

`msync` doesn't really do much yet, so if you need a command line mastodon client right now, my last project [Mastodial](https://github.com/kansattica/mastodial) can send posts, boosts, and favs and kind of implements the same queueing functionality.

### Get msync
Once `msync` has features, I'll have releases available for download. For now, it has to be built with cmake. If you have a fairly recent version of cmake (3.12 or later), you can simply clone the repo, make a `build` directory, run cmake inside, and cmake will download and build `msync` and its dependencies automatically. 

### Features (that I'm going to implement soon)
- [ ] Multiple account support
- [ ] Queue posts, boosts, and favs
- [ ] Send posts, boosts, and favs
- [ ] Download posts from home timeline
- [ ] Download direct messages
- [ ] Download notifications
- [ ] Download posts from lists
- [ ] Download posts from your profile
- [ ] Post template file generation
- [ ] Attachment support

### Roadmap (things for later)
- [ ] Queue and send profile updates
- [ ] Nicer UI, like a minimal webpage to show downloaded posts in.

### Build Dependencies
- C++ 17 std::filesystem support
- [Nlohmann Json](https://github.com/nlohmann/json)
- [Frozen](https://github.com/serge-sans-paille/frozen)
- [whereami](https://github.com/gpakosz/whereami.git)
- [TCLAP](http://tclap.sourceforge.net/)
- [CPR](https://github.com/whoshuu/cpr)
- [curl](https://github.com/curl/curl)