if #arg == 0 then
	print("Usage: luajit patmatch.lua pattern < in.file")
	os.exit(1)
end

for l in io.lines() do
	if l:find(arg[1]) then print(l) end
end
