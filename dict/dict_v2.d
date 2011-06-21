import tango.stdc.stdio: fgets, stdin, printf;

void main() {
	int[char[]] hash;
	char[1024] line;
	int max;
	while (fgets(line.ptr, line.length, stdin) != null) {
		int current = ++hash[line];
		max = (current > max) ? current : max;
	}
	printf("%d\t%d\n", hash.length, max);
}

