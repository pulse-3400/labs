// December 1, 2018

class Stack {
  int stack[81]; // stores directions to closest unvisited node
  
  public: 
    // TODO: constructor
    Stack() {
      clearStack(0);
    }

    // clears stack but puts current location as first position
    void clearStack(int location) {
      stack[0] = location;
      for (int k = 1; k < 81; k++) {
        stack[k] = -1;
      }
    }

    void updateLocation(int current) {
      checkUnvisited();
      for (int k = 0; k < 81; k++) {
        if (k == -1) {
          stack[k] = current;
        }
      }
    }

    // NOTE the direction the robot decides to go should be passed in as false
    void checkUnvisited(int currentLocation, int n, int e, int s, int w) {
      if (n+e+s+w >= 2) { //if there is more than one unvisited node
        clearStack(currentLocation);
      }
    } 
};
