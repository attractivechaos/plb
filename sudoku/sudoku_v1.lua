-- NB: LuaJIT-2.0.0-beta6 is buggy and unable to give the correct result
local bit = require('bit')
local sudoku = {}

sudoku.setbit = function(z, x)
	local y = bit.rshift(x, 5) + 1;
	z[y] = bit.bor(z[y], bit.lshift(1, bit.band(x, 31)))
end

sudoku.enumerate9 = function()
	local e, pos2box, mask_box, mask_col, p = {}, {}, {}, {}, {}
	for i = 0, 80 do pos2box[i] = math.floor(i/27) * 3 + math.floor(math.fmod(i, 9) / 3) end
	for i = 0, 8 do mask_box[i], mask_col[i], p[i] = 0, 0, -1 end
	local i_ = 0
	while true do
		while i_ >= 0 and i_ < 9 do
			local i9, j_ = 9 * i_, 9
			if p[i_] >= 0 then mask_col[p[i_]], mask_box[pos2box[i9+p[i_]]] = 0, 0 end
			for j = p[i_] + 1, 8 do
				if mask_col[j] == 0 and mask_box[pos2box[i9 + j]] == 0 then j_ = j; break end
			end
			if j_ < 9 then mask_col[j_], mask_box[pos2box[i9 + j_]], p[i_], i_ = 1, 1, j_, i_ + 1
			else p[i_], i_ = -1, i_ - 1 end
		end
		if i_ < 0 then break end
		e[#e + 1] = {0, 0, 0}
		for i = 0, 8 do sudoku.setbit(e[#e], 9 * i + p[i]) end
		i_ = i_ - 1
	end
	return e
end

sudoku.solve = function(e, s)
	local z, x, y, n, p, ret, i_ = {}, {}, {}, {}, {}, {}, 0
	for i = 1, 9 do
		local c = 0
		x, y, z[i] = {0, 0, 0}, {0, 0, 0}, {}
		for j = 0, 80 do
			if s:byte(j+1) >= 49 and s:byte(j+1) <= 57 then
				if s:byte(j+1) - 48 == i then sudoku.setbit(x, j)
				else sudoku.setbit(y, j) end
			end
		end
		for j = 1, 46656 do
			local ej = e[j]
			if bit.band(ej[1], x[1]) == x[1] and bit.band(ej[2], x[2]) == x[2] and bit.band(ej[3], x[3]) == x[3]
					and bit.band(ej[1], y[1]) == 0 and bit.band(ej[2], y[2]) == 0 and bit.band(ej[3], y[3]) == 0 
			then z[i][c], z[i][c+1], z[i][c+2], c = e[j][1], e[j][2], e[j][3], c + 3 end
		end
		n[i] = bit.bor(bit.lshift(c/3, 8), i)
	end
	table.sort(n)
	for i = 1, 9 do p[i], n[i] = -3, bit.band(n[i], 0xff) end
	x, i_ = {0, 0, 0}, 1
	while true do
		while i_ >= 1 and i_ <= 9 do
			local zi = z[n[i_]]
			if p[i_] >= 0 then
				local t = p[i_]
				x[1], x[2], x[3] = bit.band(x[1], bit.bnot(zi[t])), bit.band(x[2], bit.bnot(zi[t+1])), bit.band(x[3], bit.bnot(zi[t+2]))
			end
			local j_ = #zi + 2
			for j = p[i_] + 3, #zi, 3 do
				if bit.band(x[1], zi[j]) == 0 and bit.band(x[2], zi[j+1]) == 0 and bit.band(x[3], zi[j+2]) == 0 then
					j_ = j; break
				end
			end
			if j_ <= #zi then
				x[1], x[2], x[3], p[i_], i_ = bit.bor(x[1], zi[j_]), bit.bor(x[2], zi[j_+1]), bit.bor(x[3], zi[j_+2]), j_, i_ + 1
			else p[i_], i_ = -3, i_ - 1 end
		end
		if i_ < 1 then break end
		local y = {}
		for j = 1, 9 do
			for k = 0, 31 do if bit.band(bit.rshift(z[n[j]][p[j]+0], k), 1) == 1 then y[k+1]  = n[j] end end
			for k = 0, 31 do if bit.band(bit.rshift(z[n[j]][p[j]+1], k), 1) == 1 then y[k+33] = n[j] end end
			for k = 0, 16 do if bit.band(bit.rshift(z[n[j]][p[j]+2], k), 1) == 1 then y[k+65] = n[j] end end
		end
		ret[#ret+1] = y
		i_ = i_ - 1
	end
	return ret
end

local e = sudoku.enumerate9()
for l in io.lines() do
	if #l >= 81 then
		local ret = sudoku.solve(e, l)
		for _, v in ipairs(ret) do print(table.concat(v)) end
		print()
	end
end
