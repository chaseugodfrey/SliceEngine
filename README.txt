# Slice Engine                                                                         

Module Code: CSD3401/UXG3450

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

This file will contain the guide to the Slice Engine. Enjoy your time here.
This document provides an overview of the setup and controls for efficient utilization of our game engine.
It also details major changes to the engine since Milestone 2.

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

## Changes Since Milestone 2
### Graphics
- Unified Shader File Strucutre. Converted all shader pairs to consistent naming scheme. Simplifying shader reloading and external tooling integration.
- Improved Shader Loading System.
- Skybox & Skybox lighting. Skybox is code generated, and provides global illumination.
- Added more Post-post processing effects

### Animation
- Animator Window overhaul. Rebuilt using ImNodes. Improvements include safer state editing, clearer visual presentation and better drag & drop support.
- New FSM structures. Expanded FSM data, cleaner state naming conventions and better control transitions.
- Scene Graph & Bone System updated. Improved bone transform handling & better support for FPS style animation logic.
- Prefab compatibility with Animations. Animation data now serializes better.

### Audio
- Attack, walk and BGM sounds included in default scenes for testing.

### Scripting
- Hot reload stability fixes. Reload no longer resets the entire scripting context incorrectly.
- Added new rotation utilities, vec2 & vec3 helpers and templated clamp and general math extensions.
- Improved rotation clamping and improved movement logic in Camera controller.
- Script based sound triggering as scripts can now play SFX & BGM directly from C#.

### Navigation (Recast & Detour)
- Recast Intregration stabilised. Fixed rasterization and region settigs, enabling agent visualisation.
- Detour runtime functions added. Runtime structures added to engine, enabling NPC movement and path queries.
- Inspector updates now expose more parameters and allow baking directly from editor.

### ImGui, Inspector, Windows
- Updated console window, showing logs, warnings and script outputs.
- Scene graph updated. Debug tool improvements for visualising parent-child relationships.
- Added CTRL+D for duplication of game objects.
- Better drag-&-drop behaviour for animation controllers and prefabs.

### Physics
- Transform & Rigidbody fixes. Overhauled sync logic to keep ECS transforms and physics bodies aligned.
- Collider Runtime enhancements. Collder now de/activates correctly during gameplay events.
- API Extensions for C#. Added missing constructors, helper functions and scripting hooks for rigidbody and collider manipulation.

### Serialization
- Core serialisation converted to a templated system, improving type extensibility.
- Added support for serialising animation controllers and state machine data.

### Input
- Better prefab saving ensures Input components persist correctly.
- Full Action Mapping functionality exposed to C# scripts, including 1D and multiple action types.

### Networking
- No changes as there are no plans to include multiplayer in the final product as of yet.

### Editor Gameplay
- Created a usable level "Level: Populated".
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
When the scene window is in focus (when the scene tab header is lit up and dark blue):
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

#### Animator
Users can use the animtor to create sequences of movement for characters, creatures, and objects.
        - Right click mouse to create a node in the animator window.
        - Left click on a node and drag to move it around, click on its side nodules to create a link 
          that can connect nodes to each other.

#### Animation
Users can choose and play animation clips from the animation resource.
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
        - Create Objects: Click to create camera, 3D object or UI components.

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
