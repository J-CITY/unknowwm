
Unknown WM


Tiny window manager written in C++ with the XLib library.

![Screenshot](https://github.com/J-CITY/unknowwm/blob/master/screens/0.png)

![Screenshot](https://github.com/J-CITY/unknowwm/blob/master/screens/1.png)

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

If you are using a display manager add to your `.xinitrc`

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

You can configurate with .toml config (See `config` file):

 1. Keyboard shortcuts
 2. Gaps and borders
 3. Borders colors
 4. Windows title
 5. Autostarts commands
 6. Rules for special app
 7. Desktops count and it`s layouts

Also in `/client` folder - simple client for send events to WM.

# Features

* Multiple desktops
* Support multiple monitors
* Resizing and movement with mouse and keyboard
* Borders and optional decorate borders
* Gaps
* Title bars with custom position (left, right, top, bottom)
* Tailing modes
* Autostart rules
* Rules for special apps
* Actions /close/hide/fullscreen on title press
* Customization config with TOML
* Send WM status by script (Script set no config file)

## Support tiling modes:

 1. Vertical stack left
 2. Vertical steck right 
 3. Horizontal stack up
 4. Horizontal stack down 
 5. Monocle 
 6. Double stack
 7. Floating
 8. Fibonacci

# TODO

* Add reload config func and reinit monitors and desktops (test it)
* Check client`s monitor when it move (test it)

## Also

Unknow Dock is a simple status bar for [UnknowDock](https://github.com/J-CITY/unknowdock).

# Thanks

* Tudurom [windowchef](https://github.com/tudurom/windowchef)
* nnoell [neurowm](https://github.com/nnoell/neurowm)
* c00kiemon5ter [monsterwm](https://github.com/c00kiemon5ter/monsterwm)
