// acquired from http://pankaj-k.net/sudoku/sudoku.js
// based on Peter Norvig's http://norvig.com/sudoku.html

var test_grid1 = "003020600900305001001806400008102900700000008006708200002609500800203009005010300";
var test_grid2 = "4.....8.5.3..........7......2.....6.....8.4......1.......6.3.7.5..2.....1.4......";

function cross(A, B){
  var C = [];
  for (var a in A) 
    for (var b in B) 
      C.push(A[a] + B[b]);
  return C;
}

function member(item, list){
  for (var i in list)
    if (item == list[i]) return true;
  return false;
}

var rows = ['A','B','C','D','E','F','G','H','I'];
var cols = ['1','2','3','4','5','6','7','8','9'];
var digits = "123456789";
var squares = cross(rows, cols);
var nassigns = 0;
var neliminations = 0;
var nsearches = 0;

var unitlist = [];
for (var c in cols) 
  unitlist.push(cross(rows, [cols[c]]));
for (var r in rows) 
  unitlist.push(cross([rows[r]], cols));
var rrows = [['A','B','C'], ['D','E','F'], ['G','H','I']];
var ccols = [['1','2','3'], ['4','5','6'], ['7','8','9']];
for (var rs in rrows) 
  for (var cs in ccols) 
    unitlist.push(cross(rrows[rs], ccols[cs]));

var units = {};
for (var s in squares){
  units[squares[s]] = [];
  for (var u in unitlist)
    if (member(squares[s], unitlist[u]))
      units[squares[s]].push(unitlist[u]);
}

var peers = {};
for (var s in squares){
  peers[squares[s]] = {};
  for (var u in units[squares[s]]){
    var ul = units[squares[s]][u];
    for (var s2 in ul)
      if (ul[s2] != squares[s])
        peers[squares[s]][ul[s2]] = true;
  }
}

function parse_grid(grid){ // Given a string of 81 digits (or . or 0 or -), return an object os {cell:values}
  nassigns = 0;
  neliminations = 0;
  nsearches = 0;
  var grid2 = "";
  for (var c = 0; c < grid.length; c++)
    if ("0.-123456789".indexOf(grid.charAt(c)) >= 0)
      grid2 += grid.charAt(c);
  var values = {};
  for (var s in squares)
    values[squares[s]] = digits;
  for (var s in squares)
    if (digits.indexOf(grid2.charAt(s)) >= 0 && !assign(values, squares[s], grid2.charAt(s)))
      return false;
  return values;
}

function assign(values, sq, dig){ // Eliminate all the other values (except dig) from values[sq] and propagate.
  ++nassigns;
  var result = true;
  var vals = values[sq];
  for (var d = 0; d < vals.length; d++)
    if (vals.charAt(d) != dig)
      result &= (eliminate(values, sq, vals.charAt(d)) ? true : false);
  return (result ? values : false);
}

function eliminate(values, sq, dig){
  ++neliminations;
  if (values[sq].indexOf(dig) == -1)  // already eliminated.
    return values;
  values[sq] = values[sq].replace(dig, "");
  if (values[sq].length == 0) // invalid input ?
    return false;
  else if (values[sq].length == 1){ // If there is only one value (values[sq]) left in square, remove it from peers
    var result = true;
    for (var s in peers[sq])
      result &= (eliminate(values, s, values[sq]) ? true : false);
    if (!result) return false;
  }
  for (var u in units[sq]){
    var dplaces = [];
    for (var s in units[sq][u]){
      var sq2 = units[sq][u][s];
      if (values[sq2].indexOf(dig) != -1) 
        dplaces.push(sq2);
    }
    if (dplaces.length == 0)
      return false;
    else if (dplaces.length == 1)
      if (!assign(values, dplaces[0], dig))
        return false;
  }
  return values;
}

function dup(obj){
  var d = {};
  for (var f in obj)
    d[f] = obj[f];
  return d;
}

function search(values){
  ++nsearches;
  if (!values)
    return false;
  var min = 10, max = 1, sq = null;
  for (var s in squares){
    if (values[squares[s]].length > max)
      max = values[squares[s]].length;
    if (values[squares[s]].length > 1 && values[squares[s]].length < min){
      min = values[squares[s]].length;
      sq = squares[s];
    }
  }

  if (max == 1)
    return values;
  for (var d = 0; d < values[sq].length; d++){
    var res = search(assign(dup(values), sq, values[sq].charAt(d)));
    if (res)
      return res;
  }
  return false;
}

function center(s, w){
  var excess = w - s.length;
  while (excess > 0){
    if (excess%2) s += " "; else s = " " + s;
    excess -= 1;
  }
  return s;
}
  
function board_string(values){ // Used for debugging
  var width = 0;
  for (var s in squares)
    if (values[squares[s]].length > width)
      width = values[squares[s]].length;
  width += 1;
  var seg = "";
  for (var i = 0; i < width; i++) seg += "---";
  var line = "\n" + [seg, seg, seg].join("+");
  var board = "";
  for (var r in rows){
    for (var c in cols){
      board += center(values[rows[r] + cols[c]], width);
      if (c == 2 || c == 5) board += "|";
    }
    if (r == 2 || r == 5) board += line;
    board += "\n";
  }
  board += "\n";
  return board;
}

while ((l = readline()) != null) {
	if (l.length >= 81) {
		var v = parse_grid(l)
		var r = search(v)
		print(board_string(r))
	}
}
