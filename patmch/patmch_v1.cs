// Reference: http://support.microsoft.com/kb/308252

using System;
using System.Text.RegularExpressions;

class patmch_v1 {
	static void Main(String[] args) {
		string l;
		if (args.Length < 1) {
			Console.WriteLine("Usage: mono patmch.exe pattern < in.file");
			return;
		}
		Regex re = new Regex(args[0]);
		while ((l = Console.ReadLine()) != null) {
			Match m = re.Match(l);
			if (m.Success) Console.WriteLine(l);
		}
	}
}
