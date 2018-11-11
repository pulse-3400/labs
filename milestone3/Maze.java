import java.util.concurrent.TimeUnit;

/*
 * Maze object that can run DFS algorithm given with a user-inputed maze size, and start coordinates
 */
public class Maze {

	private int mazeSizeX;
	private int mazeSizeY;
	int visited[][];
	
	public Maze(int x, int y) {
		mazeSizeX = x;
		mazeSizeY = y;
		visited = new int[x][y]; // all initialized to 0;
	}

	/*
	 * Depth first search main logic
	 * Travels the entire maze and then returns to starting location
	 * 
	 * To stop traveling once the entire maze has been visited, call stopIfCompleted() method
	 * then stop robot
	 */
	public void dfs(int currentLocation) {
		
		printMaze(currentLocation); // print for debugging
		System.out.println();
		
		
		int xcurr = getX(currentLocation);
		int ycurr = getY(currentLocation);
		
		visited[xcurr][ycurr] = 1;
		
		int[] neighbors = getNeighbors(currentLocation); 
		
		
		for (int k = 0; k < 4; k++) {
			
			int current_n = neighbors[k];
			if (current_n != -1) {
				int xn = getX(current_n);
				int yn = getY(current_n);
				
				if (visited[xn][yn] == 0) {
					
					/* Here, have the robot travel to (xn,yn)*/
					
					// pause (for debugging, you can comment out)
					try {
						TimeUnit.SECONDS.sleep(1);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					
					
					dfs(current_n); // recursive call to visit neighbor
					
					// pause (for debugging, you can comment out)
					try {
						TimeUnit.SECONDS.sleep(1);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					
					printMaze(currentLocation);  // more debugging
					System.out.println();
				}
			}
		}
	}	
	
	/*
	 * Returns an array of all of a maze location's neighbors
	 * Each neighbor is an 8 bit number 
	 */
	public int[] getNeighbors(int location) {
		int[] neighbors = new int[4];
		
		int x = getX(location);
		int y = getY(location);
		
		// check for neighbors, else put -1
		
		// east
		if (x-1 >= 0 && x-1 <= this.mazeSizeX - 1)
			neighbors[0] = ((x-1) << 4) + y;
		else
			neighbors[0] = -1;
		
		// west  
		if (x+1 <= this.mazeSizeX - 1)
			neighbors[1] = ((x+1) << 4) + y;
		else
			neighbors[1] = -1;
		
		// south
		if (y+1 >= 0 && y+1 <= this.mazeSizeY - 1)
			neighbors[2] = (x << 4) + (y + 1);
		else
			neighbors[2] = -1;
		
		// north
		if (y-1 >= 0 && y-1 <= this.mazeSizeY - 1)
			neighbors[3] = (x << 4) + (y - 1);
		else
			neighbors[3] = -1;
		
		return neighbors;
	}
	
	/*
	 * Checks entire maze to see if all spots have been visited
	 * Returns true if entire map is visited, false otherwise
	 */
	private boolean stopIfCompleted() {
		for(int r = 0; r < mazeSizeX; r++) {
			for(int c = 0; c < mazeSizeY; c++) {
					if (visited[r][c] != 1)
						return false;
			}
		}
		return true;
	}
	
	/*
	 * Gets x location for given 8b location 
	 */
	public int getX(int currentLocation) {
		return currentLocation >> 4;
	}

	/*
	 * Gets y location for given 8b location 
	 */
	public int getY(int currentLocation) {
		return (currentLocation & 15);
	}
	
	/*
	 * Debugging method, used for testing DFS
	 *  Prints maze with current location of robot **/
	public void printMaze(int current) {
		for(int r = 0; r < mazeSizeX; r++) {
			for(int c = 0; c < mazeSizeY; c++) {
				if (r == getX(current) && c == getY(current))
					System.out.print("*\t");
				else
					System.out.print(visited[r][c] + "\t");
			}
			System.out.println();
		}
	}
	
	/*
	 * Debugging method to ensure neighbors are correct
	 *  Prints neighbors of a maze location (decimal) **/
	public void printNeighbors(int current) {
		int[] n = this.getNeighbors(current);
		for (int k = 0; k < 4; k++) {
			System.out.print(n[k] + "\t");
		}
		System.out.println();
	}

	/*
	 * Main method to run
	 */
	public static void main(String[] args) {
		// does DFS on an unvisited 6x6 maze, starting at (0,0)
		Maze maze = new Maze(6,6);
		int startingLoc = 0; // decimal
		maze.dfs(startingLoc);
	}
	
}
