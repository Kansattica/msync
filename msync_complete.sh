# should work for bash or zsh
# source this somewhere if you want it to stick
# i think the traditional place is 
# ~/.local/share/bash-completion/completions/ for bash
# or you can just put it in your .bashrc or .zshrc

if [ -n "$ZSH_VERSION" ]; then autoload -U +X bashcompinit && bashcompinit; fi

function _msync() {
	local cmd="${1}"
	local word=${COMP_WORDS[COMP_CWORD]}
	local prev=${COMP_WORDS[COMP_CWORD-1]}
	local line=${COMP_LINE}

	local accountverbose='-a --account -v --verbose';

	case "$prev" in
		$cmd)
			COMPREPLY=($( compgen -W 'new config sync gen generate q queue yeehaw location license version help' -- $word ))
			return 0;
			;;
		'config')
			COMPREPLY=($( compgen -W 'showall default sync access_token auth_code account_name instance_url client_id client_secret exclude_boosts exclude_favs exclude_follows exclude_mentions exclude_polls' -- $word ))
			return 0;
			;;
		'sync')
			# this is a weird one because it could be a config sync or a normal sync
			if [[ "$line" == "*config*" ]]; then
				COMPREPLY=($( compgen -W 'home notifications' -- $word ));
			else
				COMPREPLY=($( compgen -W "-r --retries -p --posts -m --max-requests -s --send-only -g --get-only --recv-only $accountverbose" -- $word ));
			fi
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
			COMPREPLY=($( compgen -W '-r --remove -c --clear fav boost post print' -- $word ));
			return 0;
			;;
		'-r' | '--remove' | '-c' | '--clear')
			COMPREPLY=($( compgen -W 'fav boost post print' -- $word ));
			return 0;
			;;
		'post' | '-f' | '--file' | '--attach' | '--attachment')
			COMPREPLY=($( compgen -f -- $word ));
			return 0;
			;;
		'-a' | '--account')
			local accounts=$($cmd location)
			COMPREPLY=($( compgen -C "ls $accounts" -- $word ));
			return 0;
			;;
	esac

	# if nothing else, suggest --account and --verbose
	COMPREPLY=($( compgen -W "$accountverbose" -- $word ))

}

complete -F _msync msync
