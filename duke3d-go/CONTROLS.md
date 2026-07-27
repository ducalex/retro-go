# Retro-Go Duke3D Controls

This document outlines the gamepad mapping and the specialized hotkey system implemented for Duke3D on Retro-Go compatible devices.

## Primary Controls (Game Mode)

| Retro-Go Button | Duke3D Action |
| :--- | :--- |
| **D-Pad UP** | Move Forward |
| **D-Pad DOWN** | Move Backward |
| **D-Pad LEFT** | Turn Left |
| **D-Pad RIGHT** | Turn Right |
| **A** | Fire Weapon |
| **B** | Jump / Swim Down / Fly Down |
| **X** | Crouch / Swim Up / Fly Up |
| **Y** | Toggle Jetpack |
| **L (Shoulder)** | Strafe Left |
| **R (Shoulder)** | Strafe Right |
| **SELECT** | Cycle Weapon |
| **START** | **Use / Open / Flip Switch** (Triggered on short-press release) |
| **MENU** | Toggle Game Menu |
| **OPTION** | Crouch / Swim Up / Fly Up |

---

## Shift Mode (Hold START for 500ms)

Holding the **START** button transforms the other gamepad controls into hotkeys. Movement is preserved during the initial hold to ensure smooth gameplay.

| START + Button | Duke3D Action |
| :--- | :--- |
| **START + UP** | **Use Inventory Item** (Medkit, Steroids, etc.) |
| **START + DOWN** | **Toggle Jetpack** |
| **START + LEFT** | **Previous Inventory Item** |
| **START + RIGHT** | **Next Inventory Item** |
| **START + B** | **Look Down** |
| **START + X / OPTION** | **Look Up** |

---

## How the Hotkey System Works

### 1. Transparent Hold
To ensure movement feels fluid, the game **does not stop** your current action the moment you press START. You can continue running or turning while preparing to use a hotkey. The controls only switch to their hotkey actions once the 500ms timer has expired and Shift Mode becomes active.

### 2. Reliable "Sticky" Use (Short Press)
Activating doors or switches in Duke3D requires the "Use" key to be held for at least one game tick. 
- If you tap START and release it in **less than 500ms**, the system triggers a pulse.
- This pulse is **"Sticky"**: it automatically holds the Use action for **100ms** in the background. This ensures that every interaction is registered reliably by the game engine, even with very quick taps.

### 3. Input Snapshotting
The controller state is snapshotted at the beginning of every frame. This eliminates input "jitter" and ensures that complex combinations (like strafe-jumping while cycling inventory) remain perfectly stable across all ESP32 targets.
