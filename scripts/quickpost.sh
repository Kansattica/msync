#source this in your .bashrc or .zshrc

qpost()
{
	postfile=$(mktemp /tmp/msync_post.XXXXXXXX)
	msync generate "${@}" -o "$postfile"
	$EDITOR "$postfile"
	if [ -s "$postfile" ]
	then
	postout="$(msync queue post "$postfile")"
	if [ -z "$postout" ]
		then
			echo "$postfile enqueued."
		else
			echo "$postout"
		fi
	else
		echo "$postfile empty. Not enqueueing."
	fi
	rm "$postfile"
}

if [ -n "$ZSH_VERSION" ]; then autoload -U +X bashcompinit && bashcompinit; fi

complete -W '-d --description -f --file --attach --attachment -r --reply-to -i --reply-id -c --content-warning --cw -b --body --content -p --privacy --visibility' qpost
