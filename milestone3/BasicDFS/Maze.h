/* 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *                        ECE 3400 - TEAM PULSE                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * 
 * This Maze class is a basic implementation of DFS with wall avoidance
 * 
 * A Maze object:
 * - represents maze as 2D array
 * - stores each entry of the 2D array as a 1b int (visited/not visited)
 * - returns location of next entry to travel to
 * 
 * Last updated: 11/17/18
 */
 
class Maze {
  
  /* INSTANCE VARIABLES */
  int mazeSizeX = 9;
  int mazeSizeY = 9;

  int maze[9][9]; // main data structure, representing maze

  public:

    /* CONSTRUCTOR */
    Maze() {
      // initialize maze to be unvisited
      for (int r = 0; r < mazeSizeX; r++) {
        for (int c = 0; c < mazeSizeY; c++) {
          maze[r][c] = 0;
        }
      }

      Serial.begin(9600); // debugging
    }
    
    /**********************/
    /* Main DFS algorithm */
    /**********************/
    int dfs(int currentLocation, boolean n, boolean e, boolean s, boolean w) {
      int xcurr = getX(currentLocation);
      int ycurr = getY(currentLocation);

      maze[xcurr][ycurr] = 1; // mark intersection as visited

      // getting neighbors based off wall logic
      int temp[4];    //  | N | E | S | W |
      
      if (!n) 
        temp[3] = (xcurr << 4) + (ycurr - 1); // location of north neighbor;
      else
        temp[3] = -1;
      if (!e)
        temp[2] = ((xcurr - 1) << 4) + ycurr;
      else
        temp[2] = -1;
      if (!s)
        temp[1] = (xcurr << 4) + (ycurr + 1);
      else
        temp[1] = -1;
      if (!w)
        temp[0] = ((xcurr + 1) << 4) + ycurr;
      else
        temp[0] = -1;

      for (int k = 0; k < 4; k++) { // iterate through neighbors
        int current_neighbor = temp[k];
        
        if (current_neighbor != -1) { // if neighbor is not a wall
          int xn = getX(current_neighbor);
          int yn = getY(current_neighbor);

          if (maze[xn][yn] == 0)  {  // prioritize unvisited nodes first
            return maze[xn][yn];
          }
        }
      }
      
      // all neighbors have been visited, so pick a random (valid) neighbor
      // eventually put most recently visited in a stack and base logic off of that
      while (true) {
        int current_neighbor = temp[random(0,3)]; // pls check syntax
        if (current_neighbor != -1)
          return maze[getX(current_neighbor)][getY(current_neighbor)];
      }
      return -1;
    }
    
    /******************/
    /* GETTER METHODS */
    /******************/
    int getX(int l) {
      return l >> 4;
    }

    int getY(int l) {
      return l & 15;
    }

    /*********************/
    /* DEBUGGING METHODS */
    /*********************/

    /* Prints maze to Serial monitor for easy testing */
    void printMaze(int current) {
    for(int r = 0; r < mazeSizeX; r++) {
      for(int c = 0; c < mazeSizeY; c++) {
        if (r == getX(current) && c == getY(current))
          Serial.print("*\t");
        else {
          String temp = String(maze[r][c], DEC);
          Serial.print(temp + "\t");
        }
      }
      Serial.println();
     } 
    }
  
  
};
