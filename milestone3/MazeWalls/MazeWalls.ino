#include "MazeWalls.h"

MazeWalls m = MazeWalls();
int startingLoc = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  m.dfs(startingLoc);
  //Serial.println(m.stopIfCompleted());
  delay(1000);
}
