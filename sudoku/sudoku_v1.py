import sys, string

def sd_genmat():
	r, C = 0, []
	for i in range(9):
		for j in range(9):
			for k in range(9):
				C.append([ 9*i+j, i/3*27+j/3*9+k+81, 9*i+k+162, 9*j+k+243 ])
	R = [[] for c in range(324)]
	for r in range(729):
		for c2 in range(4):
			R[C[r][c2]].append(r)
	return R, C

def sd_update(R, C, sr, sc, r, v):
	for c2 in range(4):
		c = C[r][c2]
		sc[c] += v
		for x in R[c]: sr[x] += v

def sd_solve(R, C, s):
	ret, out, hints = [], [], 0
	sr = [0 for r in range(729)]
	sc = [0 for c in range(324)]
	cr = [-1 for i in range(81)]
	cc = [-1 for i in range(81)]
	for i in range(81):
		if ord(s[i]) >= 49 and ord(s[i]) <= 57: a = ord(s[i]) - 49
		else: a = -1
		if a >= 0:
			sd_update(R, C, sr, sc, i * 9 + a, 1)
			hints += 1
		out.append(a + 1)
	i, c0, d = 0, 0, 1
	while True:
		while i >= 0 and i < 81 - hints:
			if d == 1:
				m = 10
				for j in range(324):
					c = (j + c0) % 324
					if sc[c] != 0: continue
					n = 0
					for r in R[c]:
						if sr[r] == 0: n += 1
					if n < m: m, cc[i], c0 = n, c, c + 1
					if m <= 1: break
				if m == 0 or m == 10:
					cr[i], d, i = -1, -1, i - 1
			c = cc[i]
			if d == -1 and cr[i] >= 0: sd_update(R, C, sr, sc, R[c][cr[i]], -1)
			r2_ = 9
			for r2 in range(cr[i] + 1, 9):
				if sr[R[c][r2]] == 0: r2_ = r2; break
			if r2_ < 9:
				sd_update(R, C, sr, sc, R[c][r2], 1)
				cr[i], d, i = r2, 1, i + 1
			else: cr[i], d, i = -1, -1, i - 1
		if i < 0: break
		y = []
		for j in range(81): y.append(out[j])
		for j in range(i):
			r = R[cc[j]][cr[j]]
			y[r/9] = r%9 + 1
		ret.append(y)
		i, d = i - 1, -1
	return ret

R, C = sd_genmat()
for line in sys.stdin:
	if len(line) >= 81:
		ret = sd_solve(R, C, line)
		for j in range(len(ret)):
			print(''.join(map(str, ret[j])))
		print
