# `msync` Manual
## A low bandwidth store and forward Mastodon client

Hello! Welcome to the manual for `msync`, a store and forward command line Mastodon API client. `msync` is a piece of software with some quirks, but also some (in my opinion) really cool and powerful functionality, so a manual can be a real help. You dont have to read this from top to bottom- feel free to skim the table of contents or ctrl-F for a word or phrase related to what you want to do. Or, feel free to get ahold of me on Mastodon at [@BestGirlGrace@my.hypnovir.us](https://my.hypnovir.us/@BestGirlGrace).

The rest of this document will say "Mastodon", but Pleroma should work just as well. I do some cursory testing against Pleroma, but not quite as much as I do Mastodon. They implement much the same API, and msync tries its best not to rely on Mastodonisms.

### How to start using `msync`

#### Installing

Once you've compiled, [ordered](https://princess.software/order/), or [downloaded](https://github.com/Kansattica/msync/releases) `msync` (see [README.md](./README.md#which-release-should-i-download)), you have a few options:

- leave it where it is and use it in place, or copy it to a folder in your home directory and use it from there.
- copy the compiled `msync` executable to somewhere in your `$PATH`. 
- if you're on a Debian-like system, run `sudo dpkg -i msync-<version>-Linux.deb`, which will install msync into `/usr/bin`.

Any of these options work, but please read the note about `msync_accounts` below. If you downloaded or purchased a prebuilt release, your executable will be called something like `msync-v0.9.5-install-linux-x64-gcc`. Download the release for your operating system and architecture, and see below for the difference between `portable` and `install` builds. Feel free to rename it to simply `msync` or whatever else you like. The rest of this manual will assume that your executable can be invoked simply by typing `msync` at the command line, and you can add a leading `./` or trailing `.exe` as appropriate.

#### A note about `msync_accounts`

`msync` stores its information (account settings, downloaded posts, queued posts, etc.) in a directory named `msync_accounts`. By default, `msync` creates and looks for this directory in the same directory as the executable. If you downloaded a release with `install` in the name, installed `msync` from the .deb package, or configured `msync` with `-DMSYNC_USER_CONFIG=ON`, then msync will create `msync_accounts` in the conventional place for your platform:

- On Windows, this is something like `C:\Users\username\AppData\Local`.
- On Linux and OSX, this is the `XDG_CONFIG_HOME` environment variable, if set, and `~/.config` otherwise.

I recommend this "install" approach over the same-directory-as-executable approach.

You can see where your `msync_accounts` directory is by running `msync location`. You can see what flags your `msync` was compiled with, including `MSYNC_USER_CONFIG`, by running `msync version`.

#### Setting up your account

`msync` isn't very useful until you have an account. To do this, make sure you can access the internet and run:

`msync new --account yourusername@instance.egg`

This will register `msync` with your instance and give you a URL to visit in your browser. Go to that URL, log in if needed, click 'authorize', and copy the authorization code it gives you. `msync` will give you onscreen instructions as to what to do next. Notice that the `--account` (or `-a`, for short) switch always goes at the end. Putting it earlier won't work!

Once you see `Done! You're ready to start using this account`, you're ready to go. `msync` has just created an `msync_accounts` directory, either in the same directory as itself or in a system-specific location. You can find out which by running `msync location`. Keep this somewhere safe, because it now contains a password-equivalent access token! This `msync_accounts` directory contains everything `msync` knows: your account name, your settings, anything you've queued but haven't sent, and, once you synchronize with the server, it will contain local copies of your notifications and home timeline. To move, copy, or back up your `msync` installation to another computer, simply copy that `msync_accounts` folder.

#### A note on multiple accounts

If you want to add another account, `msync` can handle as many accounts as you're willing to throw at it. Run `msync new --account anotherusername@some.instance` and follow the on-screen instructions for each account you'd like to add. Most of `msync`'s commands only operate on one account at a time, so you'll have to specify an account with the `--account` or `-a` switch if you have more than one. Only `msync sync` and the account-listing part of `msync config showall` will work without specifying an account.

When running with multiple accounts, you only have to specify the shortest unique prefix of an account to select it. For example, if you have two accounts, `SomeCoolGirl@crime.egg` and `HeyAnotherPerson@illegal.egg`, registered with msync, you only have to specify `-a s` or `-a h` to choose the account. Note that the search is also case-insensitive. So, to queue a post for the `SomeCoolGirl` account, use:

```
msync queue post new_post -a s
```

And to show all settings for the `HeyAnotherPerson` account, use:

```
msync config showall -a h
```

However, if your accounts were `SomeCoolGirl@crime.egg` and `SomebodyOnce@Toldme.egg`, you would have to specify:

```
msync queue post new_post -a somec
```

or

```
msync sync --max-requests 6 -a someb
```

In `msync` versions 0.9.9 and later, you can use `msync config default --account accountname` to set that account as the default. This uses the same prefix rule as every other command, so `accountname` can be reduced to any unambiguous prefix. If an account is set as the default, it will be used as the argument for all commands that would normally throw an error if no account is specified, such as `queue` and `config`. Note that `sync` will always interpret a missing account flag as "sync all accounts". Running `msync config default` with no account specified will ensure no accounts are marked as default.

Note that the `-a` flag must always go last, after all other flags and arguments.

Note also that `msync sync` doesn't have to take an `--account` flag. You can use `msync sync` with an account to sync only that account, or omit the account flag to sync all your accounts.

Tab completion, described below, can help by autocompleting account names.

To remove an account from msync, simply delete its folder from `msync_accounts`.

### How to use `msync`

When I say `msync` is a store and forward client. What this means is that `msync` lets you queue stuff up while you're not connected to the internet, and then sync up later. `msync` will only connect to the internet when you run `msync new` or `msync sync`. Everything else simply manipulates settings or queues that are kept locally on your machine. `msync` is for everyone, but it's designed for:

- Unreliable, slow, or not always-on internet connections.
- Computers or connections that can't handle the Mastodon web frontend.
- Systems that run "headless" or otherwise can't use a graphical Mastodon client.
- Anyone who prefers to use a keyboard.
- Use in automated scripts.

#### Tab completion

`msync` comes with tab completion for bash and zsh. To use it, simply `source msync_completion.sh` in your .bashrc, .zshrc, or equivalent. There's more advice in the file itself, at `scripts/msync_completion.sh`.

#### Man page

Running `msync` without any arguments or `msync help` will print msync's help information in `man` page format.

#### The home timeline and notifications

After your account is set up, running `msync sync --verbose` will connect to your instance and, if all is well, start downloading notifications and statuses. The `--verbose` is optional (and can be shortened to `-v`), but it will make `msync` tell you where to find the downloaded timeline and notifications. These will be stored at `msync_accounts/[username@instance.url]/home.list` and `msync_accounts/[username@instance.url]/notifications.list`, respectively. You can locate your `msync_accounts` folder at any time by running `msync location`. There's a lot of ways you can look at these files, but the important things to know the process are:

- `msync` does not care about the contents of these files. It simply appends posts and notifications to them. You can delete these files, edit them, move them elsewhere, `msync` doesn't care.
- When you first sync up, `msync` will get five chunks of statuses or notifications. On subsequent updates, `msync` will default to downloading until it's "caught up", and has downloaded everything since the last post it saw. To change this behavior, use the ` --max-requests <integer>` option when calling `msync sync`. 
- Especially when using `--max-requests`, tell `msync` whether you want it to get the newest posts first or the oldest by using `msync config sync (home|notifications) (newest|oldest|off)`
- If you plan on always syncing every message every time, instead of using `--max-requests`, I suggest using `oldest` instead of `newest`. When syncing oldest-first, `msync` can write the messages to disk as they come in, letting you see the files update immediately AND not having to store every message in memory until the end. In addition, due to limitations on the Mastodon API, newest-first will only ever download the most recent 400 or so posts. For this reason, oldest-first is the default for syncing both the home timeline and notifications.
- Note that you can also not sync a timeline at all with `msync config sync home off`
- If you don't care about a specific type of notification, you can stop `msync` from retrieving them when you sync with `msync config exclude_boosts true`, and same for `favs`, `follows`, `mentions`, and `polls`. `msync` treats anything starting with a `t`, `T`, `y`, or `Y` as truthy, and everything else as falsy. So `exclude_favs true`, `exclude_favs YES`, and `exclude_favs Yeehaw` are equivalent.
- I'll write more about configuration later, but for now, you can see all your settings and registered accounts with `msync config showall`.

#### Reading the home timeline, notifications, and bookmarks

Your home timeline, notifications, and bookmarks are saved into files named `home.list`, `notifications.list`, and `bookmarks.list` in your `msync_accounts` folder under the appropriate user account. You can find where these files are located by running `msync location` or `msync sync --verbose`. `msync` doesn't provide a built-in way to look at these files. It's designed so that you can use whatever tool you prefer for reading text files. Here's a few ways that work for me to get you started.

##### vim

I usually use `msync` while ssh'd into a Linux server. When I do, my program of choice is vim. You can open up every home timeline and notification list `msync` has in separate tabs like this:

```
vim -p `msync location`/**/*.list
```

If you're using Bash, you may have to run `shopt -s globstar` first. 

After starting vim, I suggest you run `:bufdo set autoread` to quiet the annoying errors that pop up when you let `msync` modify the files while vim is running. Doing this might wind up putting one buffer over the one you currently have open, so consider doing this in an empty `:tabe` and running `:q` afterwards. So, when I start vim with my `msync` data in it, I usually start with:

```
:tabe
:bufdo set autoread
:q
```

Alternatively, you can simply add this line to your `.vimrc`.

```
set autoread
```

`gt` and `gT` move you forward and backwards through the tabs, respectively. Run `msync` commands like this:

```
:!msync sync
```

Or, to generate a new post and open it in its own tab:

```
:!msync gen
:tabe new_post
```

This is especially useful when you want to copy reply ids, users, and content warnings from an existing post into a reply.

You can move through each list with the usual vim movement keys. Page up and page down work fine, for example. To move one post or notification at a time,  I either type `/--------` (eight is the minimum number of dashes to not match the same separator twice) or simply position my cursor on one of the separators and press `*`. Once you do this, you can use `n` to go to the next post, and `N` to go to the previous post.

If you accidentally edit a file, use `:e!` to read it again from disk. When you're done with your session, `:qa!` will close all the tabs. And, of course, you can use `ctrl-Z` and `fg` to background and foreground vim if you prefer.

#### Downloading attachments

`msync` cannot display attachments on its own, but it will provide you with the URLs your mastodon instance stores attachments at. You can use a tool such as `wget`, `aria2`, or, on Windows, `Invoke-WebRequest`.

If you'd also like to download images, you can run a little shell script like the following after syncing. I recommend `aria2`, available from your Linux distribution's package manager or as a Windows binary, because it supports parallel downloads and doesn't leave the query string (that annoying `?1582789450` thing) in the filenames like `wget` does.

```
grep "^attached: " msync_accounts/username@instance.egg/home.list | sed 's/attached: //' | sort | uniq | aria2c -c -d downloaded_images -i -
```
Or, if you prefer `wget`:

```
grep "^attached: " msync_accounts/username@instance.egg/home.list | sed 's/attached: //' | sort | uniq | wget -c --content-disposition -P downloaded_images -i -
```

The `--content-disposition` flag here ensures `wget` fixes up the filename in some cases, but still leaves the query strings in the file name.

On Windows, you can use a pure Powershell solution like this:

```
New-Item -Type Directory downloaded_images;  Get-Content .\home.list | Select-String -Pattern "^attached: " | ForEach-Object { $_ -replace "^attached: ", "" } | Where-Object { !(Test-Path -Path "downloaded_images/$((Split-Path -Leaf $_).Split('?')[0])" -PathType Leaf) } | Sort-Object -Unique | ForEach-Object { Invoke-WebRequest -UseBasicParsing -Uri $_ -OutFile "downloaded_images/$((Split-Path -Leaf $_).Split('?')[0])" }
```

`Invoke-WebRequest` is pretty slow, though, so you should try using BITS if you have it:

```
New-Item -Type Directory downloaded_images;  Get-Content .\home.list | Select-String -Pattern "^attached: " | ForEach-Object { $_ -replace "^attached: ", "" } | Where-Object { !(Test-Path -Path "downloaded_images/$((Split-Path -Leaf $_).Split('?')[0])" -PathType Leaf) } | Sort-Object -Unique | ForEach-Object { New-Object psobject -Property @{Source = $_;Destination = "downloaded_images/$((Split-Path -Leaf $_).Split('?')[0])" } } | Start-BitsTransfer -TransferType Download
```

Or you can replace the last step in either of these chains with `wget` or `aria2c`, if you're willing to install those on Windows.

All of these skip downloading files you've already downloaded, so feel free to always run them after syncing- perhaps by saving them into a shell script and running `msync sync; ./get_images.sh` or whatever.

#### Queueing

To do other useful stuff with `msync`, you'll have to use queues. The `msync queue` command allows you to queue up favorites, boosts, bookmarks, and posts to be sent when you're back online. 

To queue up favorites, run `msync queue fav <any number of status IDs>`. You can get these status IDs from downloaded posts and notifications. They're always on a line that starts with `status id:` in `home.list` and `notifications.list`- don't put any other ID numbers into this! Boosts are the same: `msync queue boost <any number of status ids>`

Note that for boosted posts, you typically want to interact with the `original id` instead of the `status id`. Boosted posts get a new ID number in the database, but the Mastodon API generally wants to work with the original ID. If your msync doesn't have an `original id` field, you'll want to update to the newest release.

If you accidentally favorited or boosted a toot you'd rather not have, run `msync queue remove fav <any number of status IDs>`. This works for favorites and boosts that you haven't sent yet (which will be removed immediately) and for ones that have already been sent (which will be removed the next time you `msync sync`).

You can check the status of your queues and see what will be sent next time you sync up with `msync queue print`.

If you want to just clear that queue, possibly because there's a typo'd ID in there- you can run `msync queue clear fav` (or `boost` or `post` or `context`).

Posts are a little different. You still queue them up to be sent when you next `sync` up, but there's an extra step involved. `msync queue post <any number of file paths>` takes, well, a number of file paths. The contents of these will be interpreted as text files and sent as posts when you `msync sync` up next. A few notes on posts:

- I usually use `msync gen` to create files that I then fill with my posts. Run `msync` without any options to see all the switches that `msync gen` takes- these allow you to set privacy, content warnings, replies, and file attachments and descriptions. When I want to make a post with msync, I do this most of the time:

```
    msync gen
    vim new_post
    msync queue post new_post
    msync sync
```

- The scripts folder has a file called `quickpost.sh` that helps automate this process for simple posts. Run `source quickpost.sh` to enable it. After it's been sourced, typing `qpost` at the shell will create a temporary file, open it in your `$EDITOR` and automatically enqueue it.
- `qpost` takes the same command line arguments as `msync gen`. `qpost -b "hello there"` will open a post prefilled with `hello there` in the body. `qpost --cw food` will open a post already content-warned with `food`.
- You need not use `msync gen` to generate post files. `msync` will happily queue and send any text file you pass to `msync queue post`.
- `msync queue post` will copy the files you specify into your `msync_accounts` folder, so don't feel obligated to keep them around after you queue them.
- `msync` does *not* copy attachments when you queue them. Attachment paths are converted to absolute file paths and uploaded in place when you `msync sync` up next.
- `msync` supports image descriptions. The first description goes to the first attachment and so on. Descriptions without an image will generate a warning.
- The `--body` option to `msync gen` can be useful, especially for prefilling someone's handle in the body of a post, but be careful- your shell might do unwanted things with characters like `!` and `$`. 
- If you're replying to someone else's post, make sure you:
    - use the same (or whatever necessary) visibility setting- `msync` will default to whatever your account's default visibility setting is. 
    - include their @ handle in the body
    - include a content warning, if needed
    - all in all, my `msync` command line when generating a reply usually looks like this:

    ```
    msync gen --privacy unlisted --reply-to 12345678 --body "@whoever@crime.egg" --cw "contains content" 
    ```

- If you'd like to queue up a thread of posts all at once, you can use `msync`'s reply ID feature. It works like this:
    - If you'd like to reply to a post, assign it a reply ID. You can do this by passing the `--reply-id <id>` option to `msync gen`. The ID can be any string, but for this to work, you must reproduce it exactly, including any spaces.  I like to use `first`, `second`, `third`, and so on.
    - If you'd like a post to be a reply, pass the `--reply-to <id>` option to `msync gen`. There are two valid values for `reply_to`:
        - The status ID of a post that's already been published.
        - The `reply_id` of a post queued in front of this one. 
    - Basically, when you run `msync sync`, your queued posts are sent as normal. If a post has a `reply_id`, after it's published, any posts with a `reply_to` equal to that `reply_id` will have their `reply_to` set to the actual published ID of that post. This will persist even if the subsequent post fails to send for some reason. 
    - If a post has an invalid `reply_to`, the remote server won't accept it. You can edit the queued version of the post in `msync_accounts/<username@instance.url>/queuedposts` and sync again.
	- Starting with msync v0.9.9.9, posts queued without a reply ID will be automatically assigned one based on the filename.


- If you see a post on your timeline and want to see the rest of the thread, you can queue up a request for context next time you sync. Use `msync queue context <id>` to have `msync` fetch all the posts before and after that one in the thread. Next time you `msync sync`, it'll fetch the post in question, as well as all the posts above and below it in the thread. This doesn't get everything- it won't fetch replies to other posts in the thread, for example- but it's useful for seeing what a reply is to or digging up the rest of a thread. Threads fetched like this are stored in `msync_accounts/<username@instance.url>/threads` as `<status id>.list`, so they get picked up if you use a wildcard to open your timelines like this:

```
vim -p `msync location`/**/*.list
```

- If you fetch context for a the same thread at a later date, `msync` will automatically overwite the existing file to ensure you have the most recent version of the thread.

#### `msync` doesn't like my filename!

The command line parser library `msync` uses has a few edge cases. It seems to have issues parsing filenames that begin with the same prefix as a command msync uses. If you're trying to generate a file that starts with `post` or attach a file named `favicon.png`, the parser might get mad at you. I suggest renaming the file or, in the case of `msync gen`, entering a different name on the command line and updating it to the correct one in the generated file.

#### A note on UTF-8, Unicode, and Emoji

`msync` tries its best not to touch the bytes that it sends to or from the server. This means that posts are sent to the server and written to the local timelines as close to as is as possible. Emojis and other non-ASCII characters should work fine, as long as you send and read UTF-8. Some terminal emulators have a hard time displaying UTF-8, and you may wind up having to change some setting or use a different font. I know that running `[Console]::OutputEncoding = [Text.UTF8Encoding]::UTF8` can help on Powershell, especially when it comes to specifying command line arguments that include non-ASCII characters. Note that characters may show weird on the terminal, but go over the wire fine. 

If you want to test whether your terminal renders UTF-8, running `msync yeehaw` will attempt to print a cowboy emoji (🤠). If it shows up as garbled characters, you'll have to adjust your terminal's encoding settings. If your version of `msync` creates a `msync.log` in the current directory, you can test your file viewer of choice as well. I know that Powershell makes you pass the `-Encoding utf8` option to commands such as `Get-Content`.

`msync` tries its best to do everything in UTF-8, which means that it expects all files that it reads and writes to be UTF-8. It will convert Windows command line arguments to UTF-8 internally, but ensuring that post files (such as those generated by `msync gen`) are UTF-8 (ideally without BOM) is up to you.

Versions of Windows before Windows 10 lack the UTF-8 locale support needed for `msync` to handle correctly turning UTF-8 strings into Windows native file paths. This shouldn't be too much of a problem, but you might run into issues with non-ASCII characters in paths. To know if your system is vulnerable to this, build the tests (don't run CMake with `-DCMAKE_BUILD_TESTS=OFF`), from the build directory, run `.\tests\tests '[locale]'`. If it starts printing garbage and telling you stuff failed, it means that you'll likely have issues with non-ASCII characters in paths. Everything else should work fine regardless, though.

#### I'd like a proper man page for my system

Man pages can be generated by running `make_man_page.sh` in the scripts directory. By default, `make_man_pages.sh` assumes `msync` is in your `$PATH`. If it isn't, provide a path to `msync` as the first argument. This requires pandoc to be installed. Alternatively, you can download a premade man page from the [releases page](https://github.com/Kansattica/msync/releases).
