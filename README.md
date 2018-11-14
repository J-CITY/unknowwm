
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

See config.h to get information about wn shortcuts.

# Features

* Multiple desktops
* Resizing and movement with mouse and keyboard
* Borders and optional decorate borders
* Title bars with custom position (left, right, top ,bottom)
* Tailing modes stack (left, right, top, bottom), grid, fibonacci, fullscreen, floating
* Autostart rules
* Rules for special apps

# TODO

* Custom user config
* Bar panel
* Change font for title

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



