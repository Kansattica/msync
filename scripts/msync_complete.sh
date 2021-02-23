# should work for bash or zsh
# source this somewhere if you want it to stick
# i think the traditional place is 
# ~/.local/share/bash-completion/completions/ for bash
# or you can just put it in your .bashrc or .zshrc
# long story short, it works after doing:
# source msync_completion.sh

if [ -n "$ZSH_VERSION" ]; then autoload -U +X bashcompinit && bashcompinit; fi

function _msync() {
	local cmd="${1}"
	local subcmd="${COMP_WORDS[1]}"
	local word=${COMP_WORDS[COMP_CWORD]}
	local prev=${COMP_WORDS[COMP_CWORD-1]}
	local line=${COMP_LINE}

	local accountverbose='-a --account -v --verbose';

	# look at the last word to see what to propose next. This usually works, but not if the last thing was a command line option.
	case "$prev" in
		$cmd)
			COMPREPLY=($( compgen -W 'new config sync gen generate queue yeehaw location license version help' -- $word ))
			return 0;
			;;
		'config')
			COMPREPLY=($( compgen -W 'showall default sync access_token auth_code account_name instance_url client_id client_secret exclude_boosts exclude_favs exclude_follows exclude_mentions exclude_polls' -- $word ))
			return 0;
			;;
		'sync')
			# this is a weird one because it could be a config sync or a normal sync
			if [[ "$line" == *"config"* ]]; then
				COMPREPLY=($( compgen -W 'home notifications bookmarks' -- $word ));
			else
				COMPREPLY=($( compgen -W "-r --retries -p --posts -m --max-requests -s --send-only -g --get-only --recv-only $accountverbose" -- $word ));
			fi
			return 0;
			;;
		'home' | 'notifications')
			COMPREPLY=($( compgen -W 'newest oldest off' -- $word ));
			return 0;
			;;
		'gen' | 'generate')
			COMPREPLY=($( compgen -W '-d --description -f --file --attach --attachment -o --output -r --reply-to -i --reply-id -c --content-warning --cw -b --body --content -p --privacy --visibility' -- $word ));
			return 0;
			;;
		'-p' | '--privacy' | '--visibility')
			COMPREPLY=($( compgen -W 'default public private followersonly unlisted dm direct' -- $word ));
			return 0;
			;;
		'queue' | 'q')
			COMPREPLY=($( compgen -W 'remove -r --remove clear -c --clear fav boost bookmark post print context' -- $word ));
			return 0;
			;;
		'-r' | '--remove' | '-c' | '--clear' | 'remove' | 'r' | 'c' | 'clear')
			COMPREPLY=($( compgen -W 'fav boost bookmark post context' -- $word ));
			return 0;
			;;
		'post' | '-f' | '--file' | '--attach' | '--attachment')
			COMPREPLY=($( compgen -o filenames -A file -- $word ));
			return 0;
			;;
		'-a' | '--account')
			local accounts=$(ls "$(${cmd} location)")
			local lowercased=$(echo "$accounts" | awk '{print tolower($0)}')
			COMPREPLY=($( compgen -W "$accounts $lowercased" -- $word ));
			return 0;
			;;
	esac

	# if the first one failed, check the second argument- the 'subcommand'
	# right now, there's some duplication of effort- if this came first, we could get rid of the weird if statement in the sync for the last one
	# but the $last switch statement provides autocompletion for the visibility settings, and so that has to come first.
	case "$subcmd" in
		'gen' | 'generate')
			COMPREPLY=($( compgen -W '-d --description -f --file --attach --attachment -o --output -r --reply-to -i --reply-id -c --content-warning --cw -b --body --content -p --privacy --visibility' -- $word ));
			return 0;
			;;
		'sync')
			COMPREPLY=($( compgen -W "-r --retries -p --posts -m --max-requests -s --send-only -g --get-only --recv-only $accountverbose" -- $word ));
			return 0;
			;;
	esac

	# if nothing else, suggest --account and --verbose
	COMPREPLY=($( compgen -W "$accountverbose" -- $word ))

}

complete -F _msync msync
