for l in io.lines() do
	local pr, s, pa = l:match('([a-zA-Z][a-zA-Z0-9]*)://([^ /]+)(/[^ ]*)?')
	if pr then print(pr, s, pa) end
end
