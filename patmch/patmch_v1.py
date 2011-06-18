import re, sys

if (len(sys.argv) < 2):
    sys.stderr.write("Usage: patmatch.py in.pattern < file.in\n")
    sys.exit(1)

# the following is not correct. we have to trim off the tailing '\n'

repattern = sys.argv[1][:-1] if sys.argv[1][-1]=='\n' else sys.argv[1]
r = re.compile(repattern)
for line in sys.stdin:
    line = line[:-1]
    if line and r.search(line):
        sys.stdout.write(line)
