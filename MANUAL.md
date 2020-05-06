# `msync` Manual
## A low bandwidth store and forward Mastodon api client

Hello! Welcome to the manual for `msync`, a store and forward command line Mastodon API client. `msync` is a piece of software with some quirks, but also some (in my opinion) really cool and powerful functionality, so a manual can be a real help. You dont have to read this from top to bottom- feel free to skim the table of contents or ctrl-F for a word or phrase related to what you want to do. Or, feel free to get ahold of me on Mastodon at [@BestGirlGrace@my.hypnovir.us](https://my.hypnovir.us/@BestGirlGrace).

### How to start using `msync`

#### Installing

Once you've compiled or [downloaded](https://github.com/Kansattica/msync/releases) `msync` (see [README.md](./README.md)), you have a few options:

- leave it where it is and use it in place, or copy it to a folder in your home directory and use it from there.
- copy the compiled `msync` executable to somewhere in your `$PATH`. 
- if you're on a Debian-like system, run `sudo dpkg -i msync-<version>-Linux.deb`, which will install msync into `/usr/bin`.

Any of these options work, but please read the note about `msync_accounts` below. If you downloaded a prebuilt release, your executable will be called something like `msync-v0.9.5-linux-x64`. Feel free to rename it to simply `msync` or whatever else you like. The rest of this manual will assume that your executable can be invoked simply by typing `msync` at the command line, and you can add a leading `./` or trailing `.exe` as appropriate.

#### A note about `msync_accounts`

`msync` stores its information (account settings, downloaded posts, queued posts, etc.) in a directory named `msync_accounts`. By default, `msync` creates and looks for this directory in the same directory as the executable. If you downloaded a release with `install` in the name, installed `msync` from the .deb package, or configured `msync` with `-DMSYNC_USER_CONFIG=ON`, then msync will create `msync_accounts` in the conventional place for your platform:

- On Windows, this is something like `C:\Users\username\AppData\Local`.
- On Linux, this is the `XDG_CONFIG_HOME` environment variable, if set, and `~/.config` otherwise.

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

Note that the `-a` flag must always go last, after all other flags and arguments.

Note also that `msync sync` doesn't have to take an `--account` flag. You can use `msync sync` with an account to sync only that account, or omit the account flag to sync all your accounts.

To remove an account from msync, simply delete its folder from `msync_accounts`.

### How to use `msync`

When I say `msync` is a store and forward client. What this means is that `msync` lets you queue stuff up while you're not connected to the internet, and then sync up later. `msync` will only connect to the internet when you run `msync new` or `msync sync`. Everything else simply manipulates settings or queues that are kept locally on your machine. `msync` is for everyone, but it's designed for:

- Unreliable, slow, or not always-on internet connections.
- Computers or connections that can't handle the Mastodon web frontend.
- Systems that run "headless" or otherwise can't use a graphical Mastodon client.
- Anyone who prefers to use a keyboard.
- Use in automated scripts.

#### Reading the home timeline and notifications

After your account is set up, running `msync sync --verbose` will connect to your instance and, if all is well, start downloading notifications and statuses. The `--verbose` is optional (and can be shortened to `-v`), but it will make `msync` tell you where to find the downloaded timeline and notifications. These will be stored at `msync_accounts/[username@instance.url]/home.list` and `msync_accounts/[username@instance.url]/notifications.list`, respectively. You can locate your `msync_accounts` folder at any time by running `msync location`. There's a lot of ways you can look at these files (in a text editor, IDE or `less` are common for me, I'll write more about this later), but the important things to know the process are:

- `msync` does not care about the contents of these files. It simply appends posts and notifications to them. You can delete these files, edit them, move them elsewhere, `msync` doesn't care.
- When you first sync up, `msync` will get five chunks of statuses or notifications. On subsequent updates, `msync` will default to downloading until it's "caught up", and has downloaded everything since the last post it saw. To change this behavior, use the ` --max-requests <integer>` option when calling `msync sync`. 
- Especially when using `--max-requests`, tell `msync` whether you want it to get the newest posts first or the oldest by using `msync config sync (home|notifications) (newest|oldest|off)`
- If you plan on always syncing every message every time, instead of using `--max-requests`, I suggest using `oldest` instead of `newest`. When syncing oldest-first, `msync` can write the messages to disk as they come in, letting you see the files update immediately AND not having to store every message in memory until the end.
- Note that you can also not sync a timeline at all with `msync config sync home off`
- If you don't care about a specific type of notification, you can stop `msync` from retrieving them when you sync with `msync config exclude_boosts true`, and same for `favs`, `follows`, `mentions`, and `polls`. `msync` treats anything start with a `t`, `T`, `y`, or `Y` as truthy, and everything else as falsy. So `exclude_favs true`, `exclude_favs YES`, and `exclude_favs Yeehaw` are equivalent.
- I'll write more about configuration later, but for now, you can see all your settings and registered accounts with `msync config showall`.

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

To do other useful stuff with `msync`, you'll have to use queues. The `msync queue` command allows you to queue up favorites, boosts, and posts to be sent when you're back online. 

To queue up favorites, run `msync queue fav <any number of status IDs>`. You can get these status IDs from downloaded posts and notifications. They're always on a line that starts with `status id:` in `home.list` and `notifications.list`- don't put any other ID numbers into this! Boosts are the same: `msync queue boost <any number of status ids>`

If you accidentally favorited or boosted a toot you'd rather not have, run `msync queue --remove fav <any number of status IDs>`. This works for favorites and boosts that you haven't sent yet (which will be removed immediately) and for ones that have already been sent (which will be removed the next time you `msync sync`).

You can check the status of your queues and see what will be sent next time you sync up with `msync queue print`.

If you want to just clear that queue, possibly because there's a typo'd ID in there- you can run `msync queue --clear fav` (or `boost` or `post`).

Posts are a little different. You still queue them up to be sent when you next `sync` up, but there's an extra step involved. `msync queue post <any number of file paths>` takes, well, a number of file paths. The contents of these will be interpreted as text files and sent as posts when you `msync sync` up next. A few notes on posts:

- Most of the time, I use `msync gen` to create files that I then fill with my posts. Run `msync` without any options to see all the switches that `msync gen` takes- these allow you to set privacy, content warnings, replies, and file attachments and descriptions. When I want to make a post with msync, I do this most of the time:

```
    msync gen
    vim new_post
    msync queue post new_post
    msync sync
```

- `msync queue post` will copy the files you specify into your `msync_accounts` folder, so don't feel obligated to keep them around after you queue them.
- `msync` does *not* copy attachments when you queue them. Attachment paths are converted to absolute file paths and uploaded in place when you `msync sync` up next.
- The `--body` option to `msync gen` can be useful, especially for prefilling someone's handle in the body of a post, but be careful- your shell might do unwanted things with characters like `!` and `$`. 
- If you're replying to someone else's post, make sure you:
    - use the same (or whatever necessary) visibility setting- `msync` will default to whatever your account's default visibility setting is. 
    - include their @ handle in the body
    - include a CW, if needed
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

#### A note on UTF-8, Unicode, and Emojis

`msync` tries its best not to touch the bytes that it sends to or from the server. This means that posts are sent to the server and written to the local timelines as close to as is as possible. Emojis and other non-ASCII characters should work fine, as long as you send and read UTF-8. Some terminal emulators have a hard time displaying UTF-8, and you may wind up having to change some setting or use a different font. I know that running `[Console]::OutputEncoding = [Text.UTF8Encoding]::UTF8` can help on Powershell. In addition, characters may show weird on the terminal, but go over the wire fine. Alternatively, [Powershell Core](https://github.com/PowerShell/PowerShell) should do UTF-8 everywhere by default.

If you want to test whether your terminal renders UTF-8, running `msync yeehaw` will attempt to print a cowboy emoji (🤠). If it shows up as garbled characters, you'll have to adjust your terminal's encoding settings. If your version of `msync` creates a `msync.log` in the current directory, you can test your file viewer of choice as well. I know that Powershell makes you pass the `-Encoding utf8` option to commands such as `Get-Content`.

`msync` tries its best to do everything in UTF-8, but this can cause issues, especially on Windows, where a string can be UTF-8, UTF-16 (especially paths), or whatever code page your system has. Work is ongoing to fix this, especially since there won't be a good way to interpret UTF-8 strings as paths on Windows until C++20, but things seem to work alright for now. The biggest issue is probably with attachment paths, which might get messed up getting moved between narrow-byte code page encoding and UTF-16.
