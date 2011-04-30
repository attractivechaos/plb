// -*- mode: c++ -*-
// $Id: sieve.g++,v 1.6 2001/07/11 17:45:46 doug Exp $
// http://www.bagley.org/~doug/shootout/
// From Bill Lear
// with help from Stephane Lajoie

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[]) {
    size_t NUM = (argc == 2 ? (atoi(argv[1]) < 1 ? 1 : atoi(argv[1])): 1);

    vector<char> primes(8192 + 1);
    vector<char>::iterator pbegin = primes.begin();
    vector<char>::iterator begin = pbegin + 2;
    vector<char>::iterator end = primes.end();

    while (NUM--) {
        fill(begin, end, 1);
        for (vector<char>::iterator i = begin; i < end; ++i) {
            if (*i) {
                const size_t p = i - pbegin;
                for (vector<char>::iterator k = i + p; k < end; k += p) {
                    *k = 0;
                }
            }
        }
    }

    cout << "Count: " << count(begin, end, 1) << endl;
}
