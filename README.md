
Unknown WM


Tiny window manager written in C++ with the XLib library.

![Screenshot](https://github.com/J-CITY/unknowwm/blob/master/screens/0.png)

# Description
Unknown WM is a simple window manager.

# Installation

First, install the XLib.

Clone the repository and compile it

Compile WM

``` bash
git clone https://github.com/J-CITY/unknowwm
cd unknowwm
make
```

If you are not using a display manager add to your `.xinitrc`

```bash
exec ~/unknowwm/wm
```

and create a file `unknowwm` in `/usr/share/xsessions/`

```
[Desktop Entry]
Encoding=UTF-8
Name=unknowwm
Comment=Unknow WM - a small window manager
Exec=~/unknowwm/wm
Type=XSession
Name[ru_RU.utf8]=unknowwm
```

# Usage

You can configurate your shortcuts in config file

Support commands:

cmd_move_resize - move or resize window

cmd_run - run shell cmd

cmd_layout - set lyaout mode V_STACK_LEFT | V_STACK_RIGHT | H_STACK_UP | H_STACK_DOWN | MONOCLE | DOUBLE_STACK_VERTICAL | FLOAT | FIBONACCI

cmd_key - other options which you can use with shortcuts
TogglePanel - toggle place for bar

KillClient

NextWin

PrevWin

SwapMaster - swap current client with master
	
MoveUp - move client up in stack
	
MoveDown  - move client down in stack

PrevDesktop

ToggleFullscreenClient - set window in fullscreen mode

ToggleFloatClient - set window in floating mode

HideCurClient

HideAllClientOnDescktop 

Quit

ResizeMaster

ResizeStack

NextDesktop

NextDesktop

NextFilledDesktop

NextFilledDesktop

ClientToDesktop

ChangeDecorateBorder - change 2nd border size

ChangeBorder - change 1st border size

ChangeGap - cahnge gaps

AddMaster - add second master window

ChangeLayout

ChangeDesktop

ClientToDesktop

for more informations about function params see config file



# Features

* Multiple desktops
* Resizing and movement with mouse and keyboard
* Borders and optional decorate borders
* Title bars with custom position (left, right, top ,bottom)
* Tailing modes stack (left, right, top, bottom), grid, fibonacci, fullscreen, floating
* Autostart rules
* Rules for special apps
* Actions /close/hide/fullscreen on title press

# TODO

* Add PIPE and remove write to file
* Add reload config func and reinit monitors and desktops (test it)
* Check client`s monitor when it move (test it)
* Refactor

# Thanks

* Tudurom [windowchef](https://github.com/tudurom/windowchef)
* nnoell [neurowm](https://github.com/nnoell/neurowm)
* c00kiemon5ter [monsterwm](https://github.com/c00kiemon5ter/monsterwm)

# Screenshots

Title:

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/title.png)

Stack top mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/st.png)

Stack bottom mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/sb.png)

Stack right mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/sr.png)

Stack left mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/sl.png)

Grid mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/grid.png)

Fibonacci mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/fib.png)

Float mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/float.png)

Fullscreen mode

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/full.png)

Mono border

![Imgur](https://github.com/J-CITY/unknowwm/blob/master/screens/border.png)



