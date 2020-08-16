# UNKNOWWM Client

Simplae app to sent commands to UNKNOWWM.

## Build

```
g++ -o client logger.cpp main.cpp -lX11
```

## Commands

```
change_desktop - change_desktop <int>
change_monitor - change_monitor <int>
switch_mode - switch_mode <int> 
    0 - V stack left 
    1 - V stack right 
    2 - H stack up 
    3 - H stack down 
    4 - monocle 
    5 - grid 
    6 - float 
    7 - fibbonaci 
    8 - double stack
quit - quit
toggle_panel - toggle_panel
next_win - next_win
prev_win - prev_win
next_decktop - next_decktop
prev_decktop - prev_decktop
toggle_fullscreen - toggle_fullscreen
toggle_float - toggle_float
next_layout - next_layout
prev_layout - prev_layout
restart - restart
restart_monitors - restart_monitors
```