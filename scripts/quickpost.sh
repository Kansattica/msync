#!/usr/bin/sh

qpost()
{
	postfile=$(mktemp /tmp/msync_post.XXXXXXXX)
	quoted_args="$(printf "%q " "${@}")"
	msync generate --body "${quoted_args}" -o "$postfile"
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
