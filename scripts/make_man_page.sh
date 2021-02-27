#/usr/bin/bash

# adapted from https://www.howtogeek.com/682871/how-to-create-a-man-page-on-linux/
# assumes msync is in your path. if it isn't, provide its path as the first argument to this script.

msync_location="${1:-msync}"
filename='msync.1.md'
version="$msync_location version" | head -n1 | cut -d' ' -f3
echo "% msync(1) msync $version" > $filename
echo "% Grace Lovelace" >> $filename
echo "% $(date "+%B %Y")" >> $filename
echo "" >> $filename
$msync_location help | sed 's/^\([A-Z]\+$\)/# \1/' >> $filename
pandoc $filename -s -t man -o msync.1
gzip -f msync.1
rm $filename

printf "To install this, you probably want to do something like:\nsudo mkdir /usr/local/man/man1\nsudo mv msync.1.gz /usr/local/man/man1\n"
