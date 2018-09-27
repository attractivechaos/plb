import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

public final class State {

	private static int total;

	private int depth;

	private Board board;

	private State parent;

	private List<State> children;

	public State(State parent, Board board) {
		total++;
		this.parent = parent;
		if (parent != null) {
			this.depth = parent.getDepth() + 1;
		}
		this.board = board;
	}

	public static int getTotal() {
		return total;
	}

	public State getParent() {
		return parent;
	}

	public int getDepth() {
		return depth;
	}

	public Board getBoard() {
		return board;
	}

	public boolean addChild(State state) {
		if (children == null) {
			children = new LinkedList<State>();
		}
		return children.add(state);
	}

	public List<State> getChildren() {
		return Collections.unmodifiableList(children);
	}

	public boolean isComplete() {
		try {
			board.getNextAvailablePosition();
			return false;
		} catch (Exception e) {
			// e.printStackTrace();
			return true;
		}
	}

	public String toString() {
		return board.toString();
	}

}
