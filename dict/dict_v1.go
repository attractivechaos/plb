package main;
import("fmt"; "bufio"; "os")

func main() {
	r := bufio.NewReader(os.Stdin);
	h := make(map[string]int);
	max := 1;
	for {
		if l, e := r.ReadString('\n'); e == nil {
			if v, ok := h[l]; ok {
				if max < v + 1 { max = v + 1; }
				h[l] = v + 1;
			} else {
				h[l] = 1;
			}
		} else {
			break;
		}
	}
	fmt.Printf("%d\t%d\n", len(h), max);
}
