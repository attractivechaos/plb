import re
import sys
import operator
import itertools

if len(sys.argv) != 1:
    sys.stderr.write("Usage: patmatch.py in.pattern < file.in\n")
    sys.exit(1)

strip_newline = operator.itemgetter(slice(0,-1))

def match(regex, lines_with_cr):
    search = re.compile(regex).search
    return itertools.ifilter(search, itertools.imap(strip_newline, lines_with_cr))

for line in match(sys.argv[1], sys.stdin):
    print line
