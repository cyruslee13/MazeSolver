# MazeSolver
A maze solving program written for EECE 2118: Microcontrollers.

The program control the Polul 3Pi robot, which has an AtMega328P microcontroller and 5 IR reflectance sensors. The program uses the reflectance to follow a line and detect maze intersections. The decision the robot made at each intersection is stored. After the maze is traversed, the robot simplifies the path so that it does not take any wrong turns the second time the maze is traversed.
