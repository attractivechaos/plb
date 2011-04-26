import sys

h, m = {}, 0
for l in sys.stdin:
	if (l in h): h[l] += 1
	else: h[l] = 1
	if (m < h[l]): m = h[l]
print(len(h), m)
