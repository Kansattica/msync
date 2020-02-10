
# msync Manual
## A low bandwidth store and forward Mastodon api client

Hello! Welcome to the manual for `msync`, a store and forward command line Mastodon API client. `msync` is a piece of software with some quirks, but also some (in my opinion) really cool and powerful functionality, so a manual can be a real help. You dont have to read this from top to bottom- feel free to skim the table of contents or ctrl-F for a word or phrase related to what you want to do. Or, feel free to get ahold of me on Mastodon at [@BestGirlGrace@my.hypnovir.us](https://my.hypnovir.us/@BestGirlGrace).

### How to start using msync

#### Installing

Once you've compiled msync (see README.md), you have a few options:

- leave it where it is and use it in place, or copy it to a folder in your home directory and use it from there.
- copy the compiled `msync` executable to somewhere in your `$PATH`. 

Either option works. The only thing to be aware of is that `msync` will create a `msync_accounts` folder in the same directory as its executable. For the purposes of this guide, commands will be written `msync sync`, but anyone using the first option should type `./msync sync` from the correct folder instead.

#### Setting up your account

`msync` isn't very useful until you have an account. To do this, make sure you can access the internet and run:

`msync new --account yourusername@instance.egg`

This will register msync with your instance and give you a URL to visit in your browser. Go to that URL, log in if needed, click 'authorize', and copy the authorization code it gives you. `msync` will give you onscreen instructions as to what to do next. Notice that the `--account` (or `-a`, for short) switch always goes at the end. Putting it earlier won't work!

Once you see `Done! You're ready to start using this account`, you're ready to go. Notice that `msync` has created an `msync_accounts` directory in the same directory as itself. Keep this somewhere safe, because it now contains a password-equivalent access token! This `msync_accounts` directory contains everything msync knows: your account name, your settings, anything you've queued but haven't sent, and, once you synchronize with the server, it will contain local copies of your notifications and home timeline. To move or copy your `msync` installation to another computer, simply copy that `msync_accounts` folder.

#### A note on multiple accounts.

If you want to add another account, `msync` can handle as many accounts as you're willing to throw at it. Run `msync new --account anotherusername@some.instance` and follow the on-screen instructions for each account you'd like to add. Most of `msync`'s commands only operate on one account at a time, so you'll have to specify an account with the `--account` or `-a` switch if you have more than one. Only `msync sync` and the account-listing part of `msync config showall` will work without specifying an account.

To remove an account from msync, simply delete its folder from `msync_accounts`.

### How to use msync

When I say `msync` is a store and forward client. What this means is that msync lets you queue stuff up while you're not connected to the internet, and then sync up later. `msync` will only connect to the internet when you run `msync new` or `msync sync`. Everything else simply manipulates settings or queues that are kept locally on your machine. `msync` is for everyone, but it's designed for:

- Unreliable, slow, or not always-on internet connections.
- Computers or connections that can't handle the Mastodon web frontend.
- Systems without a graphical without a graphical front end.