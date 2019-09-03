# msync
## A low bandwidth store and forward Mastodon api client

`msync` is a command line client for Mastodon (and anything else that implements the same API) that works a little different. `msync` doesn't stay connected to the internet all the time and constantly pull new posts. Instead, it only connects when you use the `msync sync` command. Every other change is stored on your computer until then. You can queue up posts to send, boost, and fav when you're online, download posts, DMs and notifications to look at offline, and do the same for as many accounts as you want.

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

### Roadmap (things for later)
- [ ] Queue and send profile updates