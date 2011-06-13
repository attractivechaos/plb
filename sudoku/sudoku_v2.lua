local bit = require('bit')
local sudoku = {}

sudoku.genaux = function()
	local a = {}
	for i = 0, 8 do
		for j = 0, 8 do
			for k = 0, 8 do if k ~= i then a[#a+1] = 9 * k + j end end
			for k = 0, 8 do if k ~= j then a[#a+1] = 9 * i + k end end
			local iN, jN = math.floor(i/3) * 3, math.floor(j/3) * 3
			for ki = iN, iN+2 do
				for kj = jN, jN+2 do
					if ki ~= i and kj ~= j then a[#a+1] = 9 * ki + kj end
				end
			end
		end
	end
	return a
end

sudoku.first = function(z)
	for i = 0, 8 do
		if bit.band(bit.rshift(z, i), 1) == 1 then return i end
	end
end

sudoku.update = function(a, z, p, x)
	local stack = { bit.bor(bit.lshift(p, 8), x) }
	while #stack >= 1 do
		local t = table.remove(stack)
		local q, y = bit.rshift(t, 8), bit.lshift(1, bit.band(t, 0xff))
		z[q] = 512 + y
		for l = 1, 20 do
			local aql = a[20 * q + l]
			if bit.band(z[aql], y) ~= 0 then
				if bit.rshift(z[aql], 9) == 1 then return -1 end -- confliction!
				z[aql] = bit.band(z[aql], bit.bnot(y)) - 512 -- 512=1<<9
				if bit.rshift(z[aql], 9) == 1 then
					stack[#stack+1] = bit.bor(bit.lshift(aql, 8), sudoku.first(z[aql]))
				end
			end
		end
	end
	return 0
end

sudoku.solve = function(a, s)
	local stack, ret = {{}}, {}
	for i = 0, 80 do
		local x = (s:byte(i+1) >= 48 and s:byte(i+1) <= 57) and s:byte(i+1) - 49 or -1
		stack[1][i] = x < 0 and 0x13ff or 512 + bit.lshift(1, x)
	end
	for i = 0, 80 do
		if bit.rshift(stack[1][i], 9) == 1 then sudoku.update(a, stack[1], i, sudoku.first(stack[1][i])) end
	end
	while #stack >= 1 do
		local min, min_i, max, z = 10, -1, 0, table.remove(stack)
		for i = 0, 80 do
			local c = bit.rshift(z[i], 9)
			if c > 1 and c < min then min, min_i = c, i end
			max = c > max and c or max
		end
		if max > 1 then
			local swap = {}
			for i = 0, 80 do swap[i] = z[i] end
			for i = 0, 8 do
				if bit.band(bit.lshift(1, i), swap[min_i]) ~= 0 then
					local t = {}
					for i = 0, 80 do t[i] = swap[i] end
					if sudoku.update(a, t, min_i, i) >= 0 then stack[#stack+1] = t end
				end
			end
		else
			local y = {}
			for i = 0, 80 do y[#y+1] = sudoku.first(z[i]) + 1 end
			ret[#ret+1] = y
		end
	end
	return ret
end

local aux = sudoku.genaux()
for l in io.lines() do
	if #l >= 81 then
		local ret = sudoku.solve(aux, l)
		for _, v in ipairs(ret) do print(table.concat(v)) end
		print()
	end
end
