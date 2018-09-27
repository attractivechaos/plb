import java.util.*;
import static java.lang.System.*;

public class Sudoku {

	// 1
	// private static final String BOARD =
	// "304009600006541000820000009203005704050208060107300905700000096000953200002600103";

	// 2
	// private static final String BOARD =
	// "048000700002971005170004600900030208600050001804010003005300094400192300003000120";

	// 87
	// private static final String BOARD =
	// "005603090800000006073040050054007000008010600000400580010080320500000008080201700";

	// 211
	//private static final String BOARD = "020600105810000009000013080000002090400090001050100000080320000200000043605007010";

	private static final String BOARD = "104060000058000009706000100500806000000501000000902001007000208400000390000010604";
	
	public static void main(String[] args) {
		char[] numbers = BOARD.toCharArray();
		int[] grid = new int[numbers.length];
		for (int i = 0; i < grid.length; i++) {
			grid[i] = Integer.parseInt(Character.toString(numbers[i]));
		}
		State initialState = new State(null, new Board(grid));

		Sudoku s = new Sudoku();
		LinkedList<State> completeStates = new LinkedList<State>();
		long start = System.currentTimeMillis();
		s.rsolve(initialState, completeStates);
		out.println("solved in " + (System.currentTimeMillis() - start)
				+ " milliseconds.");
		out.println("computed states: " + State.getTotal());
		out.println("solutions: " + completeStates.size());
		out.println();
		for (State state : completeStates) {
			out.println(state);
		}
	}

	public void rsolve(State state, List<State> completeStates) {
		if (state.isComplete()) {
			completeStates.add(state);
		}
		List<Integer> possibles = null;
		try {
			Board board = state.getBoard();
			possibles = board.getPossiableNumbersForNextPlacement();
			// out.println("possibles:[" + possibles + "]");
			if (!possibles.isEmpty()) {
				for (int number : possibles) {
					// out.println("\n"+board);
					Board newBoard = board.place(number);
					// out.println("placeed:[" + number + "]");
					// out.println("\n"+board);
					State state2 = new State(state, newBoard);
					state.addChild(state2);
					rsolve(state2, completeStates);
				}
			} else {
				// out.println("***********************");
			}
		} catch (Exception e) {
			// e.printStackTrace();
		}

	}

}
