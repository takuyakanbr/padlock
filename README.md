# Padlock
A Windows utility to block keyboard and mouse inputs.

## Features
- Restrict or block keyboard and mouse inputs until an unlock sequence is detected
- Ability to customize the unlock, restrict, and lock sequences

## Using
When Padlock is run, a small box (the *status box*) will appear at the bottom right corner of the screen.

##### Controls
- Left click the status box to access the settings
- Right click the status box to close Padlock
- Type the restrict sequence (default: **[Alt+R]**) to enter *Restricted mode*
- Type the lock sequence (default: **[Alt+L]**) to enter *Locked mode*
- Type the unlock sequence (default: **asdf**) to exit the two modes

##### Modes
- Default - all input allowed
- Restricted - all input blocked, except: 0-9, A-Z, Shift, Space, Page Up, Page Down, End, Home, and arrow keys
- Locked - all input blocked

##### Notes
- Padlock is not able to block [Ctrl-Alt-Del].
- To ensure reliability, Padlock should be run as administrator. (Otherwise, it will not be able to detect and block inputs on windows whose processes have elevated privileges.)

## Modifying
You are recommended to use Microsoft Visual Studio 2017.
To begin, just open ```padlock.sln``` using Visual Studio.

## License
Padlock is licensed under the [3-Clause BSD License](https://opensource.org/licenses/BSD-3-Clause).