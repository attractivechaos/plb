function sd_genmat()
	C = Array(Int,(729,4))
	R = Array(Int,(324,9))
	r = 1
	for i = 0:8, j = 0:8, k = 0:8
		C[r,1] = 9 * i + j + 1
		C[r,2] = (floor(i/3)*3 + floor(j/3)) * 9 + k + 82
		C[r,3] = 9 * i + k + 163
		C[r,4] = 9 * j + k + 244
		r += 1
	end
	nr = ones(Int,324)
	for r = 1:729, c = 1:4
		k = C[r,c]
		R[k,nr[k]] = r
		nr[k] += 1
	end
	return R,C
end
function sd_update(R,C,sr,sc,r)
	m = 10
	m_c = 0
	for c2 = 1:4
		sc[C[r,c2]] += 128
	end
 	for c2 = 1:4
 		c = C[r,c2]
		for r2 = 1:9
			rr = R[c,r2] #10
			t = sr[rr] #11
			sr[rr] += 1
			t != 0 && continue
			for cc2 = 1:4
				cc = C[rr,cc2] #15
				if (sc[cc] -= 1) < m #16
					m = sc[cc]
					m_c = cc-1
				end
			end
		end
 	end
 	return m<<16|m_c
end
function revert(R,C,sr,sc,r)
	for c2 = 1:4
		sc[C[r,c2]] -= 128
	end
 	for c2 = 1:4
 		c = C[r,c2]
		for r2 = 1:9 
			rr = R[c,r2]
			(sr[rr] -= 1) != 0 && continue #9
			for i = 1:4
				sc[C[rr,i]] += 1 #11
			end
		end
	end
end
function sd_solve(R,C,_s)
	hints = 0
	out = ref(Int)
	sr = zeros(Int,729)
	sc = Array(Int,324)
	fill!(sc,9)
	cr = zeros(Int,81)
	cc = zeros(Int,81)
	for i = 1:81
		a = isdigit(_s[i]) ? _s[i]-'1' : -1
		if a >= 0 
			sd_update(R,C,sr,sc,(i-1)*9+a+1)
			hints += 1
		end
		push!(out,a+1)
	end
	i, d, cand = 1, 1, 10<<16|0
	while true
		while i >= 1 && i < 82 - hints
			if d == 1 
				m, cc[i] = cand>>16, cand&0xffff+1
				if m > 1
					for c = 1:324
						if sc[c] < m
							m, cc[i] = sc[c], c
							m < 2 && break
						end
					end
				end
				if m == 0 || m == 10
					cr[i] = d = 0
					i -= 1
				end
			end
			c = cc[i]
			r2 = cr[i]+1
			d == 0 && cr[i] >= 1 && revert(R,C,sr,sc,R[c,r2-1])
			for rr = r2:9
				sr[R[c,rr]] == 0 && break
				r2 += 1
			end
			if r2 < 10
				cand = sd_update(R,C,sr,sc,R[c,r2])
				cr[i], d = r2, 1
				i += 1
			else
				cr[i] = d = 0
				i -= 1
			end
		end
		i < 1 && break
		for j = 1:(i-1)
			r = R[cc[j],cr[j]] - 1
			out[floor(r/9)+1] = r%9+1
		end
		i -= 1
		d = 0
	end
	return out
end
#R,C = sd_genmat()
#file = readlines(open("sudoku.txt"))
#_s = file[1]
#sd_solve(R,C,file[3])
#@time for i in file r = sd_solve(R,C,file[1]); end
function main(f,n)
	R,C = sd_genmat()
	file = readlines(open(f))
	for i = 1:n, x = 1:20
		gc_disable()
		r = sd_solve(R,C,file[x])
		#println(r)
		gc_enable()
	end
end
@time main("sudoku.txt",500)