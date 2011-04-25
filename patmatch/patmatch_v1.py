import re, sys

if (len(sys.argv) < 2):
    sys.stderr.write("Usage: patmatch.py in.pattern < file.in\n")
    sys.exit(1)

# the following is not correct. we have to trim off the tailing '\n'

r = re.compile(sys.argv[1])
for line in sys.stdin:
    if r.search(line[0:-1]):
		sys.stdout.write(line)
