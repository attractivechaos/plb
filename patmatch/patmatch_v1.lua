if #arg == 0 then
	print("Usage: luajit patmatch.lua pattern < in.file")
	os.exit(1)
end

local lineno = 0
for l in io.lines() do
	lineno = lineno + 1
	if l:find(arg[1]) then
		print(lineno .. ':' .. l)
	end
end
