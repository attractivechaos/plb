package main;
import("fmt"; "bufio"; "os"; "regexp"; "flag")

func main() {
	flag.Parse();
	if flag.NArg() == 0 {
		fmt.Printf("Usage: patmch patter < in.file\n");
		return
	}
	re := regexp.MustCompile(flag.Arg(0))
	r := bufio.NewReader(os.Stdin);
	for {
		if l, e := r.ReadBytes('\n'); e == nil {
			if re.Find(l[0:len(l)-1]) != nil {
				fmt.Printf("%s", l)
			}
		} else {
			break;
		}
	}
}
