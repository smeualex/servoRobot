Servo Motor Arm
===============
Simple Arduino based arm using 4 micro servo motors with 2 main purposes:
- self-learning
- fun

The project was inspired by:
===============

The main idea is from [robotshop.com](http://www.robotshop.com/letsmakerobots/micro-servo-robot) by [Pinaut](http://www.robotshop.com/letsmakerobots/pinaut)

The design is from [thingiverse.com](http://www.thingiverse.com/thing:1684471) by [Heartmam](http://www.thingiverse.com/Heartman/about)

Description
==============

The arm is controlled by an Arduino Micro Pro which receives serial commands to move the indivitual motors.

In the original project the arm was controlled by a "master" arm via 4 potentiometers. I had some troubles (bad 3d printed stuff + not having the right pots) and the "master" controll is done by an xBox controller from the PC using a Python script.

## Flow
xBox Controller ---> PC --- Python Script ----> serial commands over USB ---> Arduino board ---> motors

Button functions:
- test movement
- reset to start position
- reset Arduino _microPro does not have a reset button_

The robot is made from the following:
- rotating base
- shoulder and elbow articulations
- grip

## Parts List
- Arduino Micro Pro
- 4 9g micro Servo Motors
- 3 push buttons
- cables & wires
- small breadboard
- custom made board to power the motors from micro USB
- 3D printed robot parts


## To Do:
0. Check [fork of logging library](https://github.com/joscha/Arduino-Log)
1. Implement playback
2. Use EEPROM to store data
 - implement write
 - implement read
 - implement some integrity check
3. A display would be nice to have

