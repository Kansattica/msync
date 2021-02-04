#!/usr/bin/sh

qpost()
{
	postfile=$(mktemp /tmp/msync_post.XXXXXXXX)
	quoted_args="$(printf "%q " "${@}")"
	msync generate --body "${quoted_args}" -o "$postfile"
	$EDITOR "$postfile"
	if [ -s "$postfile" ]
	then
		msync queue post "$postfile"
	else
		echo "File empty. Not enqueueing."
	fi
	rm "$postfile"
}
