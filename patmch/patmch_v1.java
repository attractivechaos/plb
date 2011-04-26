import java.util.regex.*;
import java.io.*;

class patmch_v1 {
	public static void main(String[] args) throws Exception {
		BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
		String l;
		if (args.length < 1) {
			System.out.println("Usage: java -cp . patmch_v1 pattern < in.file");
			return;
		}
		Pattern re = Pattern.compile(args[0]);
		while ((l = stdin.readLine()) != null) {
			Matcher m = re.matcher(l);
			if (m.find()) System.out.println(l);
		}
	}
}
