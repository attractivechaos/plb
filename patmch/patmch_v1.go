package main

import (
	"fmt"
	"bufio"
	"os"
	"regexp"
	"flag"
)

func main() {
	flag.Parse()
	if flag.NArg() == 0 {
		fmt.Printf("Usage: patmch pattern < in.file\n")
		return
	}
	re := regexp.MustCompile(flag.Arg(0))
	r := bufio.NewReader(os.Stdin)
	w := bufio.NewWriter(os.Stdout)
	for {
		l, e := r.ReadBytes('\n')
		if e != nil {
			break
		}
		if re.Match(l[:len(l)-1]) {
			w.Write(l)
		}
	}
	w.Flush()
}
