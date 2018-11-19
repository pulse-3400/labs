
// Locations will be stored as 8b numbers
class MazeWalls {
  int mazeSizeX = 5;
  int mazeSizeY = 5;
  int maze[5][5];
  int neighbors[4]; // neighbors of current location

  public:
    /* Constructor: Initialize maze to all unvisited */
    MazeWalls() {
      for (int r = 0; r < mazeSizeX; r++) {
        for (int c = 0; c < mazeSizeY; c++) {
          maze[r][c] = 0;
        }
      }

      Serial.begin(9600);

    }

    void dfs(int currentLocation) {
      printMaze(currentLocation);
      Serial.println();

      
      int xcurr = getX(currentLocation);
      int ycurr = getY(currentLocation);

      // check for walls and set last 4 bits accordingly
      maze[xcurr][ycurr] = markVisited(maze[xcurr][ycurr]);
      //maze[xcurr][ycurr] = 1;

      setNeighbors(currentLocation);
      for (int k = 0; k < 4; k++) {
        int current_neighbor = neighbors[k];
        if (current_neighbor != -1) { // if current_neighbor is valid
          int xn = getX(current_neighbor);
          int yn = getY(current_neighbor);

          if (maze[xn][yn] == 0)  {  // if you haven't visited the neighbor
            delay(1000);
            /* Here, have the robot travel to (xn,yn)*/
            dfs(current_neighbor);

            delay(1000);
            printMaze(currentLocation);
            Serial.println();
          }
        }
      }
    }

    int getX(int l) {
      return l >> 4;
    }

    int getY(int l) {
      return l & 15;
    }

    void setNeighbors(int location) {
    
      int x = getX(location);
      int y = getY(location);
      
      // check for neighbors, else put -1
      
      // east
      if (x-1 >= 0 && x-1 <= mazeSizeX - 1)
        neighbors[0] = ((x-1) << 4) + y;
      else
        neighbors[0] = -1;
      
      // west  
      if (x+1 <= mazeSizeX - 1)
        neighbors[1] = ((x+1) << 4) + y;
      else
        neighbors[1] = -1;
      
      // south
      if (y+1 >= 0 && y+1 <= mazeSizeY - 1)
        neighbors[2] = (x << 4) + (y + 1);
      else
        neighbors[2] = -1;
      
      // north
      if (y-1 >= 0 && y-1 <= mazeSizeY - 1)
        neighbors[3] = (x << 4) + (y - 1);
      else
        neighbors[3] = -1;
            
    }

    int markVisited(int v) {
      if (! (v&1))
        return v + 1;
      return v;
    }

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
