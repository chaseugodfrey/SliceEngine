# Slice Engine                                                                         

Module Code: CSD3451/UXG3450

Milestone: 4
Team: SR3C
Engine Name: Slice Engine

------------------------------------------------------------------------------------------------------

## Team				
### Programmers
Gideon Nicholas FRANCIS (RTIS)
Role: Technical Co-Lead, Engine Architecture

Rainne WON (RTIS)
Role: Graphics Co-Lead

Elton LEOSANTOSA (RTIS)
Role: Graphics Co-Lead

LEE Yong Yee (RTIS)
Role: Audio Lead, Scripting

MUHAMMAD Rayan Bin Mohd Ramlan (RTIS)
Role: Networking Lead

KOH Crystal Qiao Wei (RTIS)
Role: AI Lead

Ian Godfrey RODRIGUES (IMGD)
Role: Technical Co-Lead

LAI Nicholas (IMGD)
Role: ImGui Lead

TEO Kok Chin Aloysius (IMGD)
Role: Physics Lead

MUHAMMAD Hafiz Bin Onn (IMGD)		
Role: Serialisation Lead

LIM Sheng Yao Micah (IMGD)
Role: Project Manager, Input Lead

### Designers
WANG Jiale (UXGD)
Role: AVFX Lead & Technical Designer

Dylan NG Shao Wei (UXGD)
Role: Design Lead & 3D Artist

Ezekiel NG Fu Xiang (UXGD)
Role: Systems Designer & Technical Designer			
		
------------------------------------------------------------------------------------------------------

This file will contain the guide to the Slice Engine. Enjoy your time here(or not).
This document provides an overview of the setup and controls for efficient utilization of our game engine.
It also details major changes to the engine since Milestone 3.

## Setup
The game engine solution was developed in Visual Studio 2022.
our engine consists of 3 projects:
- SliceEditor (Editor)
- SliceEngine (Engine)
- SliceScript (Scripting)

When first setting up the project, the external dependencies must be downloaded using a batch file. 

### Prerequisites: 
Ensure Premake is installed on your device, and that your Visual Studio has the latest C++ module installed. If not, you can download the Premake command-line-executable from Github (https://premake.github.io/).

Run the BAT file "PremakeProj.bat" by double-clicking it in "./SliceEngine/Slice/" to download the dependencies into the "./?/" folder. 
if the Visual Studio solution does not appear after running the BAT file the first time, run "PremakeProj.bat" again.

Please build the engine in Release.

When running the engine for the first time, the engine will take a while to start up as all the assets need to be compiled.

------------------------------------------------------------------------------------------------------

## Changes Since Milestone 3
### Graphics
- Added a render queue system
- Added cascading shadow map
- Added functionality where materials update based on custom shaders

### Animation
- Stabilised animation & FSM systems
- Added functionality for animation system to have script events
- Animator now has states available and more types of states to encompass player animations

### Navigation (Recast & Detour)
- Implemented navigation/pathfinding around obstacles
- Implemented off mesh navigation

### ImGui, Inspector, Windows, Animator
- General debugging
- Added support for Game Object script variables
- Able to save preferred scene to load on startup
- Loads last opened scene if no preferred scene was saved
- Added functionality to add scenes to the build version
- Added functionality to displays states and transitions

### Physics
- Implemented raycasting, impulse physics, layermask and mesh collider.
- Scripting functionality added for ray and raycastinfo.

### Serialization
- Added linking and saving specific navmeshes to specific scenes

### Particle System
- Able to create 3D particles
- Mesh rendering supports all shapes that is available
- Cone and Spherical area shape support
- Orbits, size, rotation, velocity and colour over time
- Added Burst support
- Added physics interaction upport (Stickiness, Bounciness, Friction, Gravity)
- Added Collision support

### Scene System
- Loads scene into queue and by index
- String in-app by accessing scene handles from build settings

### UI
- Added UI font/text rendering, text alignment and wrapping
- Implemented canvas raycast with alpha threshold

### Editor Gameplay
- Created a playable start-to-end level
- Updated player logic in C# for movement and attacks.
- Improved spawner and enemy behaviour stability.
- Added support for animation driven attacks via collider activation.

------------------------------------------------------------------------------------------------------

## Editor
If the windows squished or positioned in ways that are difficult to interact with, 
users can drag and resize the editor and the windows in it. 

### Shortcut Controls
Here are the controls you can use within the game engine:

#### Scene Window
When the scene window is in focus (when the scene tab header is lit up):
WHILE holding down the right mouse button, 
        - Move the mouse around to move the camera
        - Press the WASD keys to move the camera forward, left, back or right
        - Press the Q and E keys to move up and down respectively

To move game objects around, left click on specified game object, the game's 3D-axis will appear:
        - Hold & drag the white center circle to move object around freely
        - Hold & drag green arrow to move object along Y-axis
        - Hold & drag red or blue arrow to move object along X-axis and Z-axis

Undo/Redo/Save/Duplicate
        - Press Ctrl + Z to undo
        - Press Ctrl + Y to redo
        - Press Ctrl + S to save
        - Press Ctrl + D to duplicate after clicking on game object in hierarchy window

WHILE holding the scrollwheel,
        - Scroll up to increase the speed of camera movement
        - Scroll down to decrease the speed of camera movement

Using W for Translate, E for Rotate and R for Scale changes the gizmos as well.

#### Inspector Window
Contains the Component Window
Allows users to edit and adjust game object Transform, Renderer and Script values
        - Transform: Key in new X, Y & Z-coordinates for position, rotation and scale
        - Renderer: Type in type of mesh desired. E.g. cube, sphere, pyramid, player
        - Renderer: Type in type of material desired
        - Script: Type in which script you'd like object to subscribe to and adjust script speed

        - Click Add Component button to add Rigidbody, Object Colliders, Audio & Light Sources 
          Script, Renderer, Nav Agent, AudioListener, Lightsource, Particle System and Animator components.

#### Navigation
        - Click selected desired object and key in new values to edit Rasterization, Agent, Region
          and Polygonization values.
        - Click bake and clear buttons to compile a new navmesh object file or clear previous values 
          of objects 

#### Profiler Window
        - Logger window logs all Info, Error, Debug and Warning messages. 
        - Click the box on the top to toggle on/off auto-scroll.
        - Profiler window details which system is generating the most load on the editor 
          and their speeds.
        - SceneGraphView is a parent-child debugging tool. 

#### Content Browser
Contains an asset browser and allows users to drag and drop assets into scene window to use.
Assets include audio, materials, shaders, textures, tests and default scenes, editors, materials, models, navmeshes, prefabs
        - If a new asset is added via the file explorer, press the "Refresh" 
          button to refresh the files shown in the Assets Browser window.
        - Left click with mouse on selected asset and drag into the scene window for use
        - Right click anywhere to create folder, material, animation clip, 
          animation controller or shader.

#### Game Window
This window is used primarily for camera game objects
        - Create a camera GO in the hierarchy window.
        - Click on it in the hierarchy window to adjust its transform, rotate or scale components
          in order to achieve the viewpoint desired.
        - Use WASD Keys to move the camera around.

#### Animator
Users can use the animator to create sequences of movement for characters, creatures, and objects.
        - Right click mouse to create a node in the animator window.
        - Left click on a node and drag to move it around, click on its side nodules to create a link 
          that can connect nodes to each other.

#### Animation
Users can play, stop, loop and add animation clips/events from the animation resource.
        - Drag & drop an .fbx asset from the content browser, then click on the root node to play,
          pause and stop the animations in the editor.

#### Hierarchy Window
        - Left click on a GO (game object) and drag it on top of another GO to make it a child of 
          that GO.
        - Right click on a game object to open the option to either unparent it or remove it.
        - Right click anywhere to create camera, 3D object or UI components.
        - Right click anywhere to add a gameobject.
        - Click on the triangle to the left of the name of any parent objects to hide/show 
          their children in the hierarchy.
                
#### Ribbon Tabs
##### "File" Options:
        - New Scene:  
                        Opens an empty scene with a single directional light and Third Person Camera
        - Save Scene: 
                        Saves all active scenes 
        - Save Scene as:
                        Save the current scene with another name
        - Project Settings:
                        Build:
                                - Selected scenes will be loaded in the game build
                                - Press button to add current scene to game build
                        Audio: 
                                - Adjust master volume in editor
                                - Open SFX List to choose and play loaded SFX
                                - Click "+" & "-" buttons to add/delete SFX
                        Physics:
                                - Broad Phase Layer Optimization allows user to determine whether 
                                  objects like UI, Player, Environment & Enemies can be affected by
                                  physics
                                - Layer Collision Matrix allows users to determine whether objects
                                  can collide with each other. Uncheck/boxes to toggle on/off

        - Preferences:
                        Theme:
                                - Use to adjust the theme of the editor to light, dark or Microsoft standard
                        Scene:
                                - Use to choose which scene will be the startup scene
        - Exit:
                        Closes the "File" options

##### "Window" Options:
        - Click to create a window if it does not exist.
        - Click "Undo History" to clear history of actions done in editor

##### "Debug" Options:
        - Click "Show Entity IDs" to show Entity IDs in Hierarchy

##### "GameObject" Options:
        - Create Objects: Click to create camera, 3D object or UI components.
        - Click "Empty" to replicate latest created GameObject

------------------------------------------------------------------------------------------------------

## Known Limitations:
1. Cannot modify animations (not edit animation grpahs nor can create animation controllers)
------------------------------------------------------------------------------------------------------