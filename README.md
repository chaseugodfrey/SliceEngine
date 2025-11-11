# Slice Engine                                                                         

Module Code: CSD3401/UXG3450

Milestone: 2
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

This file will contain the guide to the Slice Engine. Enjoy your time here.
This document provides an overview of the setup and controls for efficient utilization of our game engine.
It also details major changes to the engine since Milestone 1.

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

## Changes Since Milestone 1
1. Scene System
Has been set up, added functionality to save and load scenes.

2. Navmesh Building & Mesh Management
Added NavigationWindow to edit and bake the NavMesh.

3. Materials
Functionality added to define how surfaces appear visually(color, texture, shininess, flexibility). Can create and edit settings of materials, system can save/load material files and apply them to meshes at runtime.

## System Specific Changes
### Graphics
Lighting with shadows(point & direction) implemented. Added functionality for object picking.

### Animation
Functionality for skinned mesh animations. When creating model entities, scene graph can be recreated in ECS with the engine's scene graph as long as the model has a scene graph in it. Added functionality for playing animation loops, State machine & finite state machine works together with animator system.

### Audio
Reduced the number of loops that system required to work. Added functionality so that audio sources have pointes to a channel instead.

### Scripting
Added functionality for hot reloading, able to recompile on button press.

### Navigation
Navmesh generation(Recast) has been setup, navmeshes can be built. Debugger for grid added. Navmesh runtime (Detour) framework setup.

### ImGui
Undo/Redo functionality added.
Added AnimationWindow to test and look at animations from GameObjects
Added Inspector for Materials

### Physics
Added functionality for API creation for C# scripts

### Serialization
Changed serialization system to a templated structure, simplifying ways of adding new types to support. Increased flexibility to add more layers through the completetion of data side implementation.

### Input
Added functionality for Action Mapping.

### Networking
Despite no plans for foreseeable future use of this feature, connection between clients has been setup. Clients can send packets to each other.

### Editor Gameplay
        - Created a usable level "Level: Populated".
        - Controls: press WASD to move & R to attack.
        - Implemented Waypoint system for payload to automatically move from point to point.
        - Implemented Spawner that creates enemies that chase and damage the player.

------------------------------------------------------------------------------------------------------

## Editor
If the windows squished or positioned in ways that are difficult to interact with, 
users can drag and resize the editor and the windows in it. 

### Shortcut Controls
Here are the controls you can use within the game engine:

#### Scene Window
When the scene window is in focus (when the scene tab header is lit up and dark blue):
WHILE holding down the right mouse button, 
        - Move the mouse around to move the camera
        - Press the WASD keys to move the camera forward, left, back or right
        - Press the Q and E keys to move up and down respectively

To move game objects around, left click on specified game object, the game's 3D-axis will appear:
        - Hold & drag the white center circle to move object around freely
        - Hold & drag green arrow to move object along Y-axis
        - Hold & drag red or blue arrow to move object along X-axis and Z-axis

Undo/Redo/Save
        - Press Ctrl + Z to undo
        - Press Ctrl + Y to redo
        - Press Ctrl + S to save

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
          and Animator components.

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
Assets include audio, materials, shaders, textures, tests and default scenes.
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

#### Animation
Users can choose and play animation clips from the animation resource but cannot edit as of now
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
                        Selected scenes will be loaded in the game build
        - Preferences:
                        Use to adjust the theme of the editor to light, dark or Microsoft standard
        - Exit:
                        Closes the "File" options

##### "Window" Options:
        - Click to create a window if it does not exist.

##### "GameObject" Options:
        - Create Objects:
                        Click to create camera, 3D object or UI components.

------------------------------------------------------------------------------------------------------

## Known Limitations:
1. Undo/redo doesn't work for everything in the scene, namely works on only some inspector values, changes made using gizmo movements and undoing of adding of gameobjects/components.

2. UI components within the hierarchy window do not exist as of yet.

------------------------------------------------------------------------------------------------------

## Design Changes since M1
        - Updated level design
        - Addition of ultimate ability
        - Addition of 2 new enemies
        - Split up ember nodes into two versions
        - Updated HUD
        - Updated models
