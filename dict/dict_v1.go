// Contributed by Pat
package main

import (
	"fmt"
	"bufio"
	"os"
)

func main() {
	r, _ := bufio.NewReaderSize(os.Stdin, 1e6)
	h := make(map[string]int, 1e6)
	max := 1
	for {
		b, e := r.ReadSlice('\n')
		if e != nil {
			break
		}
		l := string(b)
		v := h[l] + 1
		h[l] = v
		if v > max {
			max = v
		}
	}
	fmt.Printf("%d\t%d\n", len(h), max)
}
