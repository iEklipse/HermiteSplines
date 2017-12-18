COMP 371 - Computer Graphics
Winter 2016

Tri-Luong Steven Dien
27415281

Programming Assignment 2
=============================================================================================
Features and Functionalities

In this programming assignment, the program takes as input a set of control points and their tangents
entered by the user and creates a cubic Hermite spline interpolating those points.

The application also draws a simple triangular object that follows motion path defined by the spline
and the curve's local orientation.

The program handle different user inputs (keyboard and mouse):

- The window can be resized however the window will always keep the same aspect ration (square form)
- [Left Mouse Button] : Mark control points/tangents in the window
- [Key 'RIGHT', 'LEFT', 'UP', 'DOWN'] : Move the camera right, left, up and down
- [Key 'L'] : Render spline using line strips
- [Key 'P'] : Render spline using points
- [Key 'Enter'] : Compute and draws the spline based on the control points and tangents points entered
- [Key 'Backspace'] : Reset the application
- [Key 'Escape'] : Close the application

To change the speed of the triangle:
// Fast computer, set the animationSpeed variable with a big number (ie. 1000.0)
// Slower computer, set the animationSpeed variable with a small number (ie. 10.0);
==============================================================================================