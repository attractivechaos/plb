import sys, string

def sd_genmat():
	r, C = 0, []
	for i in range(9):
		for j in range(9):
			for k in range(9):
				C.append([ 9*i+j, i//3*27+j//3*9+k+81, 9*i+k+162, 9*j+k+243 ])
	R = [[] for c in range(324)]
	for r in range(729):
		for c2 in range(4):
			R[C[r][c2]].append(r)
	return R, C

def sd_update(R, C, sr, sc, r, v):
	m, m_c = 10, 0
	for c2 in range(4): sc[C[r][c2]] += v<<7
	for c2 in range(4):
		c = C[r][c2]
		if v > 0:
			for r2 in range(9):
				rr = R[c][r2]
				sr[rr] += 1
				if sr[rr] == 1:
					for cc2 in range(4):
						cc = C[rr][cc2]
						sc[cc] -= 1
						if sc[cc] < m: m, m_c = sc[cc], cc
		else:
			for r2 in range(9):
				rr = R[c][r2]
				sr[rr] -= 1
				if sr[rr] == 0:
					p = C[rr]
					sc[p[0]] += 1; sc[p[1]] += 1; sc[p[2]] += 1; sc[p[3]] += 1;
	return m, m_c

def sd_solve(R, C, s):
	ret, out, hints = [], [], 0
	sr = [0 for r in range(729)]
	sc = [9 for c in range(324)]
	cr = [-1 for i in range(81)]
	cc = [-1 for i in range(81)]
	for i in range(81):
		if ord(s[i]) >= 49 and ord(s[i]) <= 57: a = ord(s[i]) - 49
		else: a = -1
		if a >= 0:
			sd_update(R, C, sr, sc, i * 9 + a, 1)
			hints += 1
		out.append(a + 1)
	i, m, d = 0, 10, 1
	while True:
		while i >= 0 and i < 81 - hints:
			if d == 1:
				if m > 1:
					for c in range(324):
						if sc[c] < m:
							m, cc[i] = sc[c], c
							if m < 2: break
				if m == 0 or m == 10:
					cr[i], d, i = -1, -1, i - 1
			c = cc[i]
			if d == -1 and cr[i] >= 0: sd_update(R, C, sr, sc, R[c][cr[i]], -1)
			r2_ = 9
			for r2 in range(cr[i] + 1, 9):
				if sr[R[c][r2]] == 0: r2_ = r2; break
			if r2_ < 9:
				m, cc[i+1] = sd_update(R, C, sr, sc, R[c][r2], 1)
				cr[i], d, i = r2, 1, i + 1
			else: cr[i], d, i = -1, -1, i - 1
		if i < 0: break
		y = []
		for j in range(81): y.append(out[j])
		for j in range(i):
			r = R[cc[j]][cr[j]]
			y[r//9] = r%9 + 1
		ret.append(y)
		i, d = i - 1, -1
	return ret

R, C = sd_genmat()
for line in sys.stdin:
	if len(line) >= 81:
		ret = sd_solve(R, C, line)
		for j in range(len(ret)):
			print(''.join(map(str, ret[j])))
		print('')
