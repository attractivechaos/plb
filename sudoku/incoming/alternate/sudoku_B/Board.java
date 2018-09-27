import static java.lang.System.out;

import java.util.*;

public final class Board {

	private static final List<Integer> numbers;

	private int[] subGrid0 = { 0, 1, 2, 9, 10, 11, 18, 19, 20 };
	private int[] subGrid1 = { 3, 4, 5, 12, 13, 14, 21, 22, 23 };
	private int[] subGrid2 = { 6, 7, 8, 15, 16, 17, 24, 25, 26 };
	private int[] subGrid3 = { 27, 28, 29, 36, 37, 38, 45, 46, 47 };
	private int[] subGrid4 = { 30, 31, 32, 39, 40, 41, 48, 49, 50 };
	private int[] subGrid5 = { 33, 34, 35, 42, 43, 44, 51, 52, 53 };
	private int[] subGrid6 = { 54, 55, 56, 63, 64, 65, 72, 73, 74 };
	private int[] subGrid7 = { 57, 58, 59, 66, 67, 68, 75, 76, 77 };
	private int[] subGrid8 = { 60, 61, 62, 69, 70, 71, 78, 79, 80 };

	static {
		numbers = new ArrayList<Integer>();
		for (int i = 0; i < 9; i++) {
			numbers.add(new Integer(i + 1));
		}
	}

	private final int[] grid;

	public Board(int[] grid) {
		this.grid = grid;
	}

	public Board place(int i) throws Exception {
		Board b = new Board(grid.clone());
		b.getGrid()[getNextAvailablePosition()] = i;
		return b;
	}

	public List<Integer> getPossiableNumbersForNextPlacement() throws Exception {

		int idx = getNextAvailablePosition();
		// out.println("idx:[" + idx + "]");

		List<Integer> possiables = new ArrayList<Integer>(numbers);

		// normalize row
		int idxNorm = idx;
		while (idxNorm > 8) {
			idxNorm -= 9;
		}
		int startIdx = idx - idxNorm;
		int endIdx = startIdx + 9;

		// exclude numbers already used in the row
		for (int z = startIdx; z < endIdx; z++) {
			int val = grid[z];
			if (val == 0) {
				continue;
			}
			possiables.remove(Integer.valueOf(val));
			// out.println("excluded(row):[" + val + "](" + z + ")");
		}

		// exclude numbers already used in the column
		for (int n = idxNorm; n < grid.length; n += 9) {
			int val = grid[n];
			if (val == 0) {
				continue;
			}
			possiables.remove(Integer.valueOf(val));
			// out.println("excluded(column):[" + val + "](" + n + ")");
		}

		// exclued subgrid

		List<Integer> subGrid = getSubGrid(idx);
		for (Integer t : subGrid) {
			if (possiables.remove(t)) {
				// out.println("excluded(t):[" + t + "]");
			}
		}

		return possiables;

	}

	private List<Integer> getSubGrid(int idx) {

		int[] subGrid = null;
		switch (idx) {
		case 0:
		case 1:
		case 2:
		case 9:
		case 10:
		case 11:
		case 18:
		case 19:
		case 20:
			subGrid = subGrid0;
			// out.println("subGrid0");
			break;
		case 3:
		case 4:
		case 5:
		case 12:
		case 13:
		case 14:
		case 21:
		case 22:
		case 23:
			subGrid = subGrid1;
			// out.println("subGrid1");
			break;
		case 6:
		case 7:
		case 8:
		case 15:
		case 16:
		case 17:
		case 24:
		case 25:
		case 26:
			subGrid = subGrid2;
			// out.println("subGrid2");
			break;
		case 27:
		case 28:
		case 29:
		case 36:
		case 37:
		case 38:
		case 45:
		case 46:
		case 47:
			subGrid = subGrid3;
			// out.println("subGrid3");
			break;
		case 30:
		case 31:
		case 32:
		case 39:
		case 40:
		case 41:
		case 48:
		case 49:
		case 50:
			subGrid = subGrid4;
			// out.println("subGrid4");
			break;
		case 33:
		case 34:
		case 35:
		case 42:
		case 43:
		case 44:
		case 51:
		case 52:
		case 53:
			subGrid = subGrid5;
			// out.println("subGrid5");
			break;
		case 54:
		case 55:
		case 56:
		case 63:
		case 64:
		case 65:
		case 72:
		case 73:
		case 74:
			subGrid = subGrid6;
			// out.println("subGrid6");
			break;
		case 57:
		case 58:
		case 59:
		case 66:
		case 67:
		case 68:
		case 75:
		case 76:
		case 77:
			subGrid = subGrid7;
			// out.println("subGrid7");
			break;
		case 60:
		case 61:
		case 62:
		case 69:
		case 70:
		case 71:
		case 78:
		case 79:
		case 80:
			subGrid = subGrid8;
			// out.println("subGrid8");
			break;
		}

		List<Integer> used = new ArrayList<Integer>();
		for (int i = 0; i < subGrid.length; i++) {
			int m = grid[subGrid[i]];
			if (m != 0) {
				used.add(Integer.valueOf(m));
			}
		}
		// out.println("excluded(subGrid)(used):" + used);
		return used;
	}

	int getNextAvailablePosition() throws Exception {
		for (int i = 0; i < grid.length; i++) {
			if (grid[i] == 0) {
				return i;
			}
		}
		throw new Exception("No available positions!");
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		int k = 0;
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				sb.append("|" + grid[k++]);
			}
			sb.append("|\n");
		}
		return sb.toString();
	}

	public int[] getGrid() {
		return grid;
	}

}
