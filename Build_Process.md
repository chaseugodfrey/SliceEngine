# Slice Engine                                                                         

Module Code: CSD3451/UXG3450

Milestone: 4
Team: SR3C
Engine Name: Slice Engine

This file will contain the guide to building Slice Engine. Enjoy your time here(or not).

## Setup
The game engine solution was developed in Visual Studio 2022.
Our engine consists of 2 solutions:
- Slice
This is our core editor and engine solution.

- WeightOfTheSky
This is our game application solution.

Slice consist of 3 projects:
- SliceEditor (Editor)
- SliceEngine (Engine)
- SliceScript (Scripting)

WeightOfTheSky consist of 1 project:
- WeightOfTheSky

When first setting up the project, the external dependencies must be downloaded using a batch file. 

### Prerequisites for Slice: 
Ensure Premake is installed on your device, and that your Visual Studio has the latest C++ module installed. If not, you can download the Premake command-line-executable from Github (https://premake.github.io/).

Run the BAT file "PremakeProj.bat" by double-clicking it in "./SliceEngine/Slice/" to download the dependencies into the "./?/" folder. 
if the Visual Studio solution does not appear after running the BAT file the first time, run "PremakeProj.bat" again.

Please build the engine in Release.

When running the engine for the first time, the engine will take a while to start up as all the assets need to be compiled.

### Prerequisites for WeightOfTheSky:
Run the BAT file "PremakeProj.bat" by double-clicking it in "./SliceEngine/WeightOfTheSky/" to download the dependencies into the "./?/" folder. 
if the Visual Studio solution does not appear after running the BAT file the first time, run "PremakeProj.bat" again.

Please build the game in Release.


