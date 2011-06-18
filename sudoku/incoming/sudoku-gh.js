/*
 * Implementation of Knuth's Dancing Links technique
 * for Algorithm X (exact cover).
 */

function dlx_cover(c)
{
    c.right.left = c.left;
    c.left.right = c.right;
    for (var i = c.down; i != c; i = i.down) {
        for (var j = i.right; j != i; j = j.right) {
            j.down.up = j.up;
            j.up.down = j.down;
            j.column.size--;
        }
    }
}

function dlx_uncover(c)
{
    for (var i = c.up; i != c; i = i.up) {
        for (var j = i.left; j != i; j = j.left) {
            j.column.size++;
            j.down.up = j;
            j.up.down = j;
        }
    }
    c.right.left = c;
    c.left.right = c;
}

function dlx_search(head, solution, k, solutions, maxsolutions)
{
    if (head.right == head) {
        solutions.push(solution.slice(0));
        if (solutions.length >= maxsolutions) {
            return solutions;
        }
        return null;
    }
    var c = null;
    var s = 99999;
    for (var j = head.right; j != head; j = j.right) {
        if (j.size == 0) {
            return null;
        }
        if (j.size < s) {
            s = j.size;
            c = j;
        }
    }
    dlx_cover(c);
    for (var r = c.down; r != c; r = r.down) {
        solution[k] = r.row;
        for (var j = r.right; j != r; j = j.right) {
            dlx_cover(j.column);
        }
        var s = dlx_search(head, solution, k+1, solutions, maxsolutions);
        if (s != null) {
            return s;
        }
        for (var j = r.left; j != r; j = j.left) {
            dlx_uncover(j.column);
        }
    }
    dlx_uncover(c);
    return null;
}

function dlx_solve(matrix, skip, maxsolutions)
{
    var columns = new Array(matrix[0].length);
    for (var i = 0; i < columns.length; i++) {
        columns[i] = new Object;
    }
    for (var i = 0; i < columns.length; i++) {
        columns[i].index = i;
        columns[i].up = columns[i];
        columns[i].down = columns[i];
        if (i >= skip) {
            if (i-1 >= skip) {
                columns[i].left = columns[i-1];
            }
            if (i+1 < columns.length) {
                columns[i].right = columns[i+1];
            }
        } else {
            columns[i].left = columns[i];
            columns[i].right = columns[i];
        }
        columns[i].size = 0;
    }
    for (var i = 0; i < matrix.length; i++) {
        var last = null;
        for (var j = 0; j < matrix[i].length; j++) {
            if (matrix[i][j]) {
                var node = new Object;
                node.row = i;
                node.column = columns[j];
                node.up = columns[j].up;
                node.down = columns[j];
                if (last) {
                    node.left = last;
                    node.right = last.right;
                    last.right.left = node;
                    last.right = node;
                } else {
                    node.left = node;
                    node.right = node;
                }
                columns[j].up.down = node;
                columns[j].up = node;
                columns[j].size++;
                last = node;
            }
        }
    }
    var head = new Object;
    head.right = columns[skip];
    head.left = columns[columns.length-1];
    columns[skip].left = head;
    columns[columns.length-1].right = head;
    solutions = [];
    dlx_search(head, [], 0, solutions, maxsolutions);
    return solutions;
}

function solve_sudoku(grid)
{
    var mat = [];
    var rinfo = [];
    for (var i = 0; i < 9; i++) {
        for (var j = 0; j < 9; j++) {
            var g = grid[i][j] - 1;
            if (g >= 0) {
                var row = new Array(324);
                row[i*9+j] = 1;
                row[9*9+i*9+g] = 1;
                row[9*9*2+j*9+g] = 1;
                row[9*9*3+(Math.floor(i/3)*3+Math.floor(j/3))*9+g] = 1;
                mat.push(row);
                rinfo.push({'row': i, 'col': j, 'n': g+1});
            } else {
                for (var n = 0; n < 9; n++) {
                    var row = new Array(324);
                    row[i*9+j] = 1;
                    row[9*9+i*9+n] = 1;
                    row[9*9*2+j*9+n] = 1;
                    row[9*9*3+(Math.floor(i/3)*3+Math.floor(j/3))*9+n] = 1;
                    mat.push(row);
                    rinfo.push({'row': i, 'col': j, 'n': n+1});
                }
            }
        }
    }
    var solutions = dlx_solve(mat, 0, 2);
    if (solutions.length > 0) {
        var r = solutions[0];
        for (var i = 0; i < r.length; i++) {
            grid[rinfo[r[i]]['row']][rinfo[r[i]]['col']] = rinfo[r[i]]['n'];
        }
        return solutions.length;
    }
    return 0;
}

while ((l = readline()) != null) {
	if (l.length >= 81) {
		var g = []
		for (var i = 0; i < 9; ++i) {
			var r = []
			for (var j = 0; j < 9; ++j) {
				var x = i * 9 + j
				r.push(l.charAt(x) >= '1' && l.charAt(x) <= '9'? l.charCodeAt(x) - 48 : 0)
			}
			g.push(r)
		}
		var r = solve_sudoku(g) // this is the number of solutions
		if (r > 0) {
			var s = ''
			for (i = 0; i < 81; ++i) s += g[Math.floor(i/9)][i%9]
			print(s+"\n")
		}
	}
}
