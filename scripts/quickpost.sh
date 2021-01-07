#!/usr/bin/sh

qpost()
{
	postfile=$(mktemp /tmp/msync_post.XXXXXXXX)
	$EDITOR "$postfile"
	if [ -s "$postfile" ]
	then
		msync queue post "$postfile"
	else
		echo "File empty. Not enqueueing."
	fi
	rm "$postfile"
}
