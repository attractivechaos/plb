#include <utility>
#include <iostream>
#include <unordered_map>

using namespace std;

int main(int argc, char *argv[])
{
	unordered_map<string, int> h;
	string s;
	int max = 1;
	while (getline(cin, s).good()) {
		unordered_map<string, int>::iterator p = h.find(s);
		if (p == h.end()) h[s] = 1;
		else {
			++p->second;
			if (max < p->second) max = p->second;
		}
	}
	cout<<h.size()<<'\t'<<max<<'\n';
	return 0;
}
