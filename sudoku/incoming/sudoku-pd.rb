# Preetam D'Souza
# 6.4.2011
# sudoku_cp.rb

$SIZE = 9
$COLORS = (1..$SIZE).to_a
$neighbors = Array.new($SIZE*$SIZE){[]}         # num -> neighbors
$units = Array.new($SIZE*$SIZE){[[],[],[]]}     # num -> [ROW, COL, SQUARE]

def build_neighbors
  ### Build adjacency list based on row, col, and square constraints ###
  (0...$SIZE).each do |r|
    (0...$SIZE).each do |c|
      v = $SIZE*r + c # node ID in [0, $SIZE*$SIZE)
      f = -> x { $neighbors[x] << v unless (x == v || $neighbors[x].include?(v)) }
      ($SIZE*r...$SIZE*(r+1)).each {|x| f.call x; $units[v][0] << x } #ROWS
      (0...$SIZE).map {|i| i*$SIZE+c}.each { |x| f.call x; $units[v][1] << x } #COLS
      lr, lc = (r/3)*3, (c/3)*3
      (0...3).each do |i|
        (0...3).each { |j| f.call lr*$SIZE+lc+j+9*i; $units[v][2] << lr*$SIZE+lc+j+9*i } # SQUARES
      end
    end
  end
end

def assign(choices, node, color)
  erased = choices[node] - [color]
  erased.each { |x|  return false unless eliminate(choices, node, x) }
end

def eliminate(choices, node, color)
  return choices unless choices[node].include?(color)
  choices[node].delete(color)
  return false if choices[node].empty?
  if choices[node].size == 1  # only one color left -> eliminate it from neighbors
    $neighbors[node].each do |n|
      return false unless eliminate(choices, n, choices[node][0])
    end
  end
  $units[node].each do |u|
    slots = []
    u.each { |x| slots << x if choices[x].include?(color) }
    return false if slots.empty? # no place for this color
    if slots.size == 1           # only one place for this color -> assign it
      return false unless assign(choices, slots[0], color)
    end
  end
  choices
end

def check(choices, neighbors)
  choices.each_with_index do |c, i|
    neighbors[i].each { |n| return false if choices[n] == c }
  end
  choices
end

def search(nodes, choices, neighbors, depth)
  return check(choices.flatten, neighbors) if depth == $SIZE*$SIZE # SOLVED
  n = nodes.min_by { |i| choices[i].size } # node with fewest color choices
  choices[n].each do |c|
    new_choices = choices.map { |x| x*1 }  # deep copy hack
    next unless assign(new_choices, n, c)
    if potential = search(nodes - [n], new_choices, neighbors, depth+1)
      return potential
    end
  end
  false
end

def solve(grid)
  choices = Array.new($SIZE*$SIZE){$COLORS.clone} # num -> available colors
  ### Assign given cells and propagate constraints ###
  grid.each_with_index { |c, i| assign(choices, i, c.to_i) unless c =~ /[0.]/ }
  ### Search for a solution ###
  search((0...$SIZE*$SIZE).to_a, choices, $neighbors, 0)
end

def test(file, sep="\n")
  grids = IO.read(file).chomp.split(sep)
  grids.map! { |x| x.chomp.split("").select {|c| c =~ /[0-9.]/ } }
  grids.each { |x| ret = solve(x); puts ret.join; puts }
end

build_neighbors

test("/dev/stdin")
