# Padlock
A Windows utility to block keyboard and mouse inputs.

## Features
- Restrict or block keyboard and mouse inputs until an unlock sequence is detected
- Ability to customize the unlock, restrict, and lock sequences
- Ability to automatically lock after a period of inactivity

## Using
When Padlock is run, a small box (the *status box*) will appear at the bottom right corner of the screen,
and a tray icon will be added to the taskbar.

#### Controls
- Right clicking the tray icon displays a popup menu, where you can access the settings or close Padlock
- Alternatively, double click the tray icon to access the settings
- Type the restrict sequence (default: **[Alt+R]**) to enter *Restricted mode*
- Type the lock sequence (default: **[Alt+L]**) to enter *Locked mode*
- Type the unlock sequence (default: **asdf**) to exit the two modes

#### Modes
- Default - all input allowed
- Restricted - all input blocked, except: 0-9, A-Z, Shift, Space, Page Up, Page Down, End, Home, and arrow keys
- Locked - all input blocked

#### Settings
- Change the unlock, restrict, and lock sequences
- Option to automatically switch to Locked mode after a period of inactivity
- Option to change when the status box is displayed 

#### Notes
- Padlock is not able to block [Ctrl-Alt-Del].
- To ensure reliability, Padlock should be run as administrator. (Otherwise, it will not be able to detect and block inputs on windows whose processes have elevated privileges.)

## Modifying
You are recommended to use Microsoft Visual Studio 2017.
To begin, just open ```padlock.sln``` using Visual Studio.

## License
Padlock is licensed under the [3-Clause BSD License](https://opensource.org/licenses/BSD-3-Clause).