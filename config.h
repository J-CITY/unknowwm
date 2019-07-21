#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <map>
#include "utils.h"
#include <memory>
#include <set>
#include <algorithm>
#include "toml.h"
#include <iostream>
class WindowManager;

const std::string ACTION_TOGGLE_PANEL = "TogglePanel";
const std::string ACTION_SWAP_MASTER = "SwapMaster";
const std::string ACTION_QUIT = "Quit";
const std::string ACTION_RUN_CMD = "RunCmd";
const std::string ACTION_KILL_CILENT = "KillClient";
const std::string ACTION_NEXT_WIN = "NextWin";
const std::string ACTION_PREW_WIN = "PrevWin";
const std::string ACTION_MOVE_RESIZE = "MoveResize";
const std::string ACTION_SWITCH_MODE = "SwitchMode";
const std::string ACTION_RESIZE_MASTER = "ResizeMaster";
const std::string ACTION_RESIZE_STACK = "ResizeStack";
const std::string ACTION_MOVE_DOWN = "MoveDown";
const std::string ACTION_MOVE_UP = "MoveUp";
const std::string ACTION_NEXT_DESKTOP = "NextDesktop";
const std::string ACTION_NEXT_FILLED_DESKTOP = "NextFilledDesktop";
const std::string ACTION_PREV_DESKTOP = "PrevDesktop";
const std::string ACTION_CLIENT_TO_DESKTOP = "ClientToDesktop";
const std::string ACTION_TOGGLE_FLOAT_CLIENT = "ToggleFloatClient";
const std::string ACTION_TOGGLE_FULLSCREEN_CLIENT = "ToggleFullscreenClient";
const std::string ACTION_CHANGE_DECORATE_BORDER = "ChangeDecorateBorder";
const std::string ACTION_CHANGE_BORDER = "ChangeBorder";
const std::string ACTION_CHANGE_GAP = "ChangeGap";
const std::string ACTION_ADD_MASTER = "AddMaster";
const std::string ACTION_HIDE_CUR_CLIENT = "HideCurClient";
const std::string ACTION_HIDE_ALL_CLIENT_ON_DESKTOP = "HideAllClientOnDescktop";
const std::string ACTION_CHANGE_DESKTOP = "ChangeDesktop";
const std::string ACTION_CHANGE_LAYOUT = "ChangeLayout";
const std::string ACTION_CHANGE_MONITOR = "ChangeMonitor";
const std::string ACTION_CLIENT_TO_MONITOR = "ClientToMonitor";
const std::string ACTION_RESTART_MONITORS = "RestartMonitors";
const std::string ACTION_RESTART = "Restart";

class Config {
public:
	int PANEL_HEIGHT_HORIZONTAL_UP    = 22;
	int PANEL_HEIGHT_HORIZONTAL_DOWN  = 0;
	int PANEL_HEIGHT_VERTICAL_LEFT    = 0;
	int PANEL_HEIGHT_VERTICAL_RIGHT   = 0;
	int USELESSGAP                    = 10;

	int TITLE_POSITION                = TITLE_LEFT;// TITLE_LEFT TITLE_RIGHT TITLE_UP TITLE_DOWN
	int TITLE_HEIGHT                  = 20;
	bool SHOW_TITLE                   = true;
	//bool TITLE_IN_BEGIN = true;

	bool SHOW_DECORATE                = true;
	int DECORATE_BORDER_WIDTH         = 3;

	int DESKTOPS                      = 4;
	int BORDER_WIDTH                  = 3;

	// mod keys
	static const auto MOD1            = Mod1Mask;//ALT
	static const auto MOD4            = Mod4Mask;//Super
	static const auto CONTROL         = ControlMask;//Control
	static const auto SHIFT           = ShiftMask;//Shift

	bool USE_TITLE_BUTTON_ACTIONS     = true;
	float       MASTER_SIZE           = 0.52;
	bool        SHOW_PANEL            = true;//Show bar panel
	int         DEFAULT_MODE          = V_STACK_LEFT;//Default layout for desktop
	bool        ATTACH_ASIDE          = true;// False means new window is master
	bool        FOLLOW_WINDOW         = false;//follow the window when moved to a different desktop
	bool        FOLLOW_MONITOR        = false;//follow the window when moved to a different monitor
	bool        FOLLOW_MOUSE          = false;//focus the window the mouse just entered
	bool        CLICK_TO_FOCUS        = true;//focus an unfocused window when clicked
	int         FOCUS_BUTTON          = Button3;// mouse button to be used along with CLICK_TO_FOCUS
	std::string FOCUS_COLOR           = "#4A724E";
	std::string UNFOCUS_COLOR         = "#3B6B71";
	std::string INFOCUS_COLOR         = "#654D47";
	std::string DECORATE_FOCUS_COLOR  = "#3B6B71";
	std::string DECORATE_UNFOCUS_COLOR= "#213846";
	std::string DECORATE_INFOCUS_COLOR= "#213846";
	std::string TITLE_TEXT_COLOR      = "#ffffff";//"#303030";

	int TITLE_DX                      = 5;
	int TITLE_DY                      = 14;
	std::string FONT                  = "Verdana:size=11";

	int MINWSZ                        = 50;//minimum window size
	int DEFAULT_MONITOR               = 0;
	int DEFAULT_DESKTOP               = 0;
	
	bool AUTOFLOATING                 = false;
	int NMASTER                       = 1;

	std::vector<int> initLayout       = {V_STACK_LEFT, V_STACK_RIGHT, GRID, -1, -1};
	std::vector<int> layouts          = {V_STACK_LEFT, V_STACK_RIGHT, H_STACK_UP, 
		                                 H_STACK_DOWN, MONOCLE, GRID, FIBONACCI, FLOAT, DOUBLE_STACK_VERTICAL};
	std::map<int, std::vector<int>> desktopLayouts = {
		{0, {V_STACK_LEFT, H_STACK_UP, FIBONACCI}},
		{2, {GRID}},
	};

	std::vector<Ml> init;
	std::vector<AppRule> rules;
	std::unique_ptr<WindowManager> wm;
	std::vector<Key> keys;
	std::vector<Button> buttons;
	std::vector<Argument> autostart;
	void DesktopChange(int key, int desk) {
		keys.push_back(Key(MOD1,       key, "ChangeDesktop",  Argument(desk)));
		keys.push_back(Key(MOD1|SHIFT, key, "ClientToDesktop",Argument(desk)));
	}
	void MonitorChange(int key, int mon) {
		keys.push_back(Key(MOD4,       key, "ChangeMonitor",  Argument(mon)));
		keys.push_back(Key(MOD4|SHIFT, key, "ClientToMonitor",Argument(mon)));
	}

	std::string PIPE_WM_INFO = "/home/daniil/wminfo";
	std::string PIPE_DOCK_INFO = "/home/daniil/dockinfo";

	std::string CONFIG_PATH = "";

	Config(std::string configPath="") {
		CONFIG_PATH = configPath;
		Parse(configPath);
		if (configPath == "") {
			InitDefault();
		}
	}
	
	void InitDefault() {
		//Autostart
		/*std::vector<char*> autos;
		autos.push_back("feh");
		autos.push_back("--bg-scale");
		autos.push_back("/home/daniil/unknowwm/1.jpg");
		autos.push_back(nullptr);
		autostart.push_back(Argument(autos));
		
		autos.clear();
		autos.push_back("/home/daniil/unknowwm/autorandr.sh");		
		//autos.push_back("conky");
		autos.push_back(nullptr);
		autostart.push_back(Argument(autos));*/
/*
		autos.clear();
		autos.push_back("python");
		autos.push_back("/home/daniil/WM/dock.py");
		autos.push_back(nullptr);
		autostart.push_back(Argument(autos));
		*/
		//Mouse button
		//buttons.push_back(Button(MOD1,    Button1,  "MouseMotion",             Argument{.i = MOVE}));
		//buttons.push_back(Button(MOD1,    Button3,  "MouseMotion",             Argument{.i = RESIZE}));
		
		//Rules
		//rules.push_back(AppRule( "Thunar",     0,       3,    true,   true));

		keys.push_back(Key(MOD1,          XK_b,      "TogglePanel",            Argument{nullptr}));
		keys.push_back(Key(MOD1|CONTROL,  XK_r,      "Quit",                   Argument{.i = 0}));
		keys.push_back(Key(MOD1|CONTROL,  XK_q,      "Quit",                   Argument{.i = 1}));
		std::vector<std::string> a;
		a.push_back("xfce4-terminal");
		//a.push_back(nullptr);
		keys.push_back(Key(MOD1|SHIFT,    XK_Return, "RunCmd",                 Argument(a)));
		keys.push_back(Key(MOD1|SHIFT,    XK_c,      "KillClient",             Argument{nullptr}));
		keys.push_back(Key(MOD1,          XK_j,      "NextWin",                Argument{nullptr}));
		keys.push_back(Key(MOD1,          XK_k,      "PrevWin",                Argument{nullptr}));
		
		std::vector<int> mv1 = { 0,  25,   0,   0};
		keys.push_back(Key(MOD4,          XK_j,      "MoveResize",             Argument(mv1)));
		std::vector<int> mv2 = { 0, -25,   0,   0};
		keys.push_back(Key(MOD4,          XK_k,      "MoveResize",             Argument(mv2)));
		std::vector<int> mv3 = { 25,  0,   0,   0};
		keys.push_back(Key(MOD4,          XK_l,      "MoveResize",             Argument(mv3)));
		std::vector<int> mv4 = {-25,  0,   0,   0};
		keys.push_back(Key(MOD4,          XK_h,      "MoveResize",             Argument(mv4)));

		keys.push_back(Key(MOD1,          XK_q,      "SwitchMode",             Argument(V_STACK_LEFT)));
		keys.push_back(Key(MOD1,          XK_w,      "SwitchMode",             Argument(V_STACK_RIGHT)));
		keys.push_back(Key(MOD1,          XK_e,      "SwitchMode",             Argument(H_STACK_UP)));
		keys.push_back(Key(MOD1,          XK_r,      "SwitchMode",             Argument(H_STACK_DOWN)));
		keys.push_back(Key(MOD1,          XK_t,      "SwitchMode",             Argument(MONOCLE)));
		keys.push_back(Key(MOD1,          XK_y,      "SwitchMode",             Argument(GRID)));
		keys.push_back(Key(MOD1,          XK_u,      "SwitchMode",             Argument(FLOAT)));
		keys.push_back(Key(MOD1,          XK_i,      "SwitchMode",             Argument(FIBONACCI)));
		keys.push_back(Key(MOD1,          XK_Return, "SwapMaster",             Argument{nullptr}));
		keys.push_back(Key(MOD1,          XK_h,      "ResizeMaster",           Argument(-10)));
		keys.push_back(Key(MOD1,          XK_l,      "ResizeMaster",           Argument(+10)));
		keys.push_back(Key(MOD1,          XK_o,      "ResizeStack",            Argument(-10)));
		keys.push_back(Key(MOD1,          XK_p,      "ResizeStack",            Argument(+10)));
		keys.push_back(Key(MOD1|SHIFT,    XK_j,      "MoveDown",               Argument{nullptr}));
		keys.push_back(Key(MOD1|SHIFT,    XK_k,      "MoveUp",                 Argument{nullptr}));
		keys.push_back(Key(MOD1|CONTROL,  XK_h,     "NextDesktop",             Argument(-1)));
		keys.push_back(Key(MOD1|CONTROL,  XK_l,     "NextDesktop",             Argument(1)));
		keys.push_back(Key(MOD1|SHIFT,    XK_h,     "NextFilledDesktop",       Argument(-1)));
		keys.push_back(Key(MOD1|SHIFT,    XK_l,     "NextFilledDesktop",       Argument(1)));
		keys.push_back(Key(MOD1,          XK_Tab,   "PrevDesktop",             Argument{nullptr}));
		keys.push_back(Key(CONTROL,       XK_F2,    "ClientToDesktop",         Argument(2)));
		keys.push_back(Key(CONTROL,       XK_m,     "ToggleFullscreenClient",  Argument{nullptr}));
		keys.push_back(Key(CONTROL,       XK_f,     "ToggleFloatClient",       Argument{nullptr}));
		keys.push_back(Key(MOD1,          XK_period,"ChangeDecorateBorder",    Argument(+1)));
		keys.push_back(Key(MOD1,          XK_comma, "ChangeDecorateBorder",    Argument(-1)));
		keys.push_back(Key(MOD4|SHIFT,    XK_period,"ChangeBorder",            Argument(+1)));
		keys.push_back(Key(MOD4|SHIFT,    XK_comma, "ChangeBorder",            Argument(-1)));
		keys.push_back(Key(MOD1|SHIFT,    XK_period,"ChangeGap",               Argument(+1)));
		keys.push_back(Key(MOD1|SHIFT,    XK_comma, "ChangeGap",               Argument(-1)));
		keys.push_back(Key(MOD1,          XK_z,     "AddMaster",               Argument(-1)));
		keys.push_back(Key(MOD1,          XK_x,     "AddMaster",               Argument(+1)));
		keys.push_back(Key(MOD1,          XK_c,     "HideCurClient",           Argument{nullptr}));
		keys.push_back(Key(MOD1,          XK_v,     "HideAllClientOnDescktop", Argument{nullptr}));
		keys.push_back(Key(MOD1,          XK_f,     "ChangeLayout",            Argument(+1)));
		keys.push_back(Key(MOD1,          XK_g,     "ChangeLayout",            Argument(-1)));

		keys.push_back(Key(MOD4,          XK_g,     "Restart",                 Argument(nullptr)));
		keys.push_back(Key(MOD4,          XK_f,     "RestartMonitors",         Argument(nullptr)));

		std::vector<int> mv5 = {0,  0,   0,   25};
		keys.push_back(Key(MOD4|SHIFT,    XK_j,     "MoveResize",              Argument(mv5)));
		std::vector<int> mv6 = {0,  0,   0,   -25};
		keys.push_back(Key(MOD4|SHIFT,    XK_k,     "MoveResize",              Argument(mv6)));
		std::vector<int> mv7 = {0,  0,   25,   0};
		keys.push_back(Key(MOD4|SHIFT,    XK_l,     "MoveResize",              Argument(mv7)));
		std::vector<int> mv8 = {0,  0,   -25,   0};
		keys.push_back(Key(MOD4|SHIFT,    XK_h,     "MoveResize",              Argument(mv8)));
		
		DesktopChange(XK_F1, 0);
		DesktopChange(XK_F2, 1);
		DesktopChange(XK_F3, 2);
		DesktopChange(XK_F4, 3);
		MonitorChange(XK_F1, 0);
		MonitorChange(XK_F2, 1);
	}

	unsigned int GetMask(std::string mask) {
		std::map <std::string, int> maskMap = {
			{ "SHIFT", ShiftMask },
			{ "CONTROL", ControlMask },
			{ "MOD1", Mod1Mask },
			{ "MOD4", Mod4Mask },
			{ "NULL", 0 }
		};

		std::vector<std::string> masks;
		std::stringstream test(mask);
		std::string segment;
		while(std::getline(test, segment, '|')) {
			masks.push_back(segment);
		}
		unsigned int resMask = 0;
		for (auto &m : masks) {
			resMask |= maskMap[m];
		}
		return resMask;
	}

	void Parse(std::string path) {
		if (path == "") {
			return;
		}
		auto config = cpptoml::parse_file(path);

		this->PIPE_WM_INFO     = config->get_qualified_as<std::string>("main.PIPE_WM_INFO").value_or("");
		this->PIPE_DOCK_INFO     = config->get_qualified_as<std::string>("main.PIPE_DOCK_INFO").value_or("");

		this->PANEL_HEIGHT_HORIZONTAL_UP    = config->get_qualified_as<int>("main.PANEL_HEIGHT_HORIZONTAL_UP").value_or(22);
		this->PANEL_HEIGHT_HORIZONTAL_DOWN  = config->get_qualified_as<int>("main.PANEL_HEIGHT_HORIZONTAL_DOWN").value_or(0);
		this->PANEL_HEIGHT_VERTICAL_LEFT    = config->get_qualified_as<int>("main.PANEL_HEIGHT_VERTICAL_LEFT").value_or(0);
		this->PANEL_HEIGHT_VERTICAL_RIGHT   = config->get_qualified_as<int>("main.PANEL_HEIGHT_VERTICAL_RIGHT").value_or(0);
		this->USELESSGAP                    = config->get_qualified_as<int>("main.USELESSGAP").value_or(10);
		
		auto titleKey = config->get_qualified_as<std::string>("main.TITLE_POSITION").value_or("TITLE_LEFT");
		std::map <std::string, int> titlePos = {
			{ "TITLE_LEFT", TITLE_LEFT },
			{ "TITLE_RIGHT", TITLE_RIGHT },
			{ "TITLE_UP", TITLE_UP },
			{ "TITLE_DOWN", TITLE_DOWN }
		};
		
		this->TITLE_POSITION               = titlePos[titleKey];// TITLE_LEFT TITLE_RIGHT TITLE_UP TITLE_DOWN
		this->TITLE_HEIGHT                 = config->get_qualified_as<int>("main.TITLE_HEIGHT").value_or(20);
		this->SHOW_TITLE                   = config->get_qualified_as<bool>("main.SHOW_TITLE").value_or(true);
		//bool TITLE_IN_BEGIN = true;

		this->SHOW_DECORATE                = config->get_qualified_as<bool>("main.SHOW_DECORATE").value_or(true);
		this->DECORATE_BORDER_WIDTH        = config->get_qualified_as<int>("main.DECORATE_BORDER_WIDTH").value_or(3);

		this->DESKTOPS                     = config->get_qualified_as<int>("main.DESKTOPS").value_or(4);
		this->BORDER_WIDTH                 = config->get_qualified_as<int>("main.BORDER_WIDTH").value_or(3);

		this->MASTER_SIZE                  = config->get_qualified_as<double>("main.MASTER_SIZE").value_or(0.52);
		this->SHOW_PANEL                   = config->get_qualified_as<bool>("main.SHOW_PANEL").value_or(true);
		
		std::map <std::string, int> defMode = {
			{ "V_STACK_LEFT", V_STACK_LEFT },
			{ "V_STACK_RIGHT", V_STACK_RIGHT },
			{ "H_STACK_UP", H_STACK_UP },
			{ "H_STACK_DOWN", H_STACK_DOWN },
			{ "MONOCLE", MONOCLE },
			{ "GRID", GRID },
			{ "FLOAT", FLOAT },
			{ "FIBONACCI", FIBONACCI },
			{ "MODES",MODES },
			{ "DOUBLE_STACK_VERTICAL",DOUBLE_STACK_VERTICAL }
		};
		auto defModeKey = config->get_qualified_as<std::string>("main.DEFAULT_MODE").value_or("V_STACK_LEFT");
		this->DEFAULT_MODE          = defMode[defModeKey];//Default layout for desktop

		this->USE_TITLE_BUTTON_ACTIONS = config->get_qualified_as<bool>("main.USE_TITLE_BUTTON_ACTIONS").value_or(true);
		this->ATTACH_ASIDE          = config->get_qualified_as<bool>("main.ATTACH_ASIDE").value_or(true);
		this->FOLLOW_WINDOW         = config->get_qualified_as<bool>("main.FOLLOW_WINDOW").value_or(false);
		this->FOLLOW_MONITOR        = config->get_qualified_as<bool>("main.FOLLOW_MONITOR").value_or(false);
		this->FOLLOW_MOUSE          = config->get_qualified_as<bool>("main.FOLLOW_MOUSE").value_or(false);
		this->CLICK_TO_FOCUS        = config->get_qualified_as<bool>("main.CLICK_TO_FOCUS").value_or(true);
		this->FOCUS_BUTTON          = Button3;// mouse button to be used along with CLICK_TO_FOCUS
		
		this->FOCUS_COLOR           = config->get_qualified_as<std::string>("main.FOCUS_COLOR").value_or("#4A724E");
		this->UNFOCUS_COLOR         = config->get_qualified_as<std::string>("main.UNFOCUS_COLOR").value_or("#3B6B71");
		this->INFOCUS_COLOR         = config->get_qualified_as<std::string>("main.INFOCUS_COLOR").value_or("#654D47");
		this->DECORATE_FOCUS_COLOR  = config->get_qualified_as<std::string>("main.DECORATE_FOCUS_COLOR").value_or("#3B6B71");
		this->DECORATE_UNFOCUS_COLOR= config->get_qualified_as<std::string>("main.DECORATE_UNFOCUS_COLOR").value_or("#213846");
		this->DECORATE_INFOCUS_COLOR= config->get_qualified_as<std::string>("main.DECORATE_INFOCUS_COLOR").value_or("#213846");
		this->TITLE_TEXT_COLOR      = config->get_qualified_as<std::string>("main.TITLE_TEXT_COLOR").value_or("#ffffff");//"#303030";

		this->TITLE_DX              = config->get_qualified_as<int>("main.TITLE_DX").value_or(5);
		this->TITLE_DY              = config->get_qualified_as<int>("main.TITLE_DY").value_or(14);
		this->FONT                  = config->get_qualified_as<std::string>("main.FONT").value_or("Verdana:size=11");

		this->MINWSZ                = config->get_qualified_as<int>("main.MINWSZ").value_or(50);//minimum window size
		this->DEFAULT_MONITOR       = config->get_qualified_as<int>("main.DEFAULT_MONITOR").value_or(0);
		this->DEFAULT_DESKTOP       = config->get_qualified_as<int>("main.DEFAULT_DESKTOP").value_or(0);
	
		this->AUTOFLOATING          = config->get_qualified_as<bool>("main.AUTOFLOATING").value_or(false);
		this->NMASTER               = config->get_qualified_as<int>("main.NMASTER").value_or(1);
		
		std::map <std::string, KeySym> keyMap = {
			{ "KEY_A", XK_a },
			{ "KEY_B", XK_b },
			{ "KEY_C", XK_c },
			{ "KEY_D", XK_d },
			{ "KEY_E", XK_e },
			{ "KEY_F", XK_f },
			{ "KEY_G", XK_g },
			{ "KEY_H", XK_h },
			{ "KEY_I", XK_i },
			{ "KEY_J", XK_j },
			{ "KEY_K", XK_k },
			{ "KEY_L", XK_l },
			{ "KEY_M", XK_m },
			{ "KEY_N", XK_n },
			{ "KEY_O", XK_o },
			{ "KEY_P", XK_p },
			{ "KEY_Q", XK_q },
			{ "KEY_R", XK_r },
			{ "KEY_S", XK_s },
			{ "KEY_T", XK_t },
			{ "KEY_U", XK_u },
			{ "KEY_V", XK_v },
			{ "KEY_W", XK_w },
			{ "KEY_X", XK_x },
			{ "KEY_Y", XK_y },
			{ "KEY_Z", XK_z },
			{ "KEY_RETURN", XK_Return },
			{ "KEY_PERIOD", XK_period },
			{ "KEY_COMMA", XK_comma },
			{ "KEY_TAB", XK_Tab },
			{ "KEY_F1", XK_F1 },
			{ "KEY_F2", XK_F2 },
			{ "KEY_F3", XK_F3 },
			{ "KEY_F4", XK_F4 },
			{ "KEY_F5", XK_F5 },
			{ "KEY_F6", XK_F6 },
			{ "KEY_F7", XK_F7 },
			{ "KEY_F8", XK_F8 },
			{ "KEY_F9", XK_F9 },
			{ "KEY_F10", XK_F10 },
			{ "KEY_F11", XK_F11 },
			{ "KEY_F12", XK_F12 },
		};

		auto autostartTable = config->get_table_array("autostart");
		for (const auto& t : *autostartTable) {
			std::cout << "autostart\n";
			auto cmd = t->get_as<std::string>("cmd").value_or("");
			std::vector<std::string> autos;
			std::stringstream test(cmd);
			std::string segment;
			while(std::getline(test, segment, ' ')) {
				autos.push_back(segment);
				std::cout << autos[autos.size()-1] << " ::\n";
			}
			autostart.emplace_back(autos);
			std::cout << autostart[0].com[0] << " @@\n";
		}

		
		std::map <std::string, int> btnMap = {
			{ "LEFT", Button1 },
			{ "RIGHT", Button3 },
			{ "MIDDLE", Button2 }
		};
		std::map <std::string, int> actionMap = {
			{ "MOVE", MOVE },
			{ "RESIZE", RESIZE }
		};
		auto btnTable = config->get_table_array("mouse");
		for (const auto& t : *btnTable) {
			auto mask = t->get_as<std::string>("mask").value_or("");
			auto resMask = GetMask(mask);
			
			auto btn = t->get_as<std::string>("button").value_or("");
			auto resBtn = btnMap[btn];

			auto action = t->get_as<std::string>("action").value_or("");
			auto resAction = actionMap[action];

			buttons.push_back(Button(resMask, resBtn, "MouseMotion", Argument{.i = resAction}));
		
		}

		//rules
		auto rulesTable = config->get_table_array("rules");
		for (const auto& t : *rulesTable) {
			auto resClass = t->get_as<std::string>("class").value_or("");
			auto resMonitor = t->get_as<int>("monitor").value_or(0);
			auto resDesktop = t->get_as<int>("desktop").value_or(0);
			auto resIsfollow = t->get_as<bool>("isfollow").value_or(true);
			auto resIsfloating = t->get_as<bool>("isfloating").value_or(true);
			rules.push_back(AppRule(resClass, resMonitor, resDesktop, resIsfollow, resIsfloating));
		}


		//move resize
		auto moveResizeTable = config->get_table_array("cmd_move_resize");
		for (const auto& t : *moveResizeTable) {
			auto mask = t->get_as<std::string>("mask").value_or("");
			auto resMask = GetMask(mask);
			auto key = t->get_as<std::string>("key").value_or("");
			auto resKey = keyMap[key];
			auto moveresize = t->get_array_of<int64_t>("moveresize");
			std::vector<int> arr;
			for (const auto& val : *moveresize) {
				arr.push_back(val);
			}
			keys.push_back(Key(resMask, resKey, "MoveResize", Argument(arr)));
		}


		//runcmd
		auto runTable = config->get_table_array("cmd_run");
		for (const auto& t : *runTable) {
			auto cmd = t->get_as<std::string>("cmd").value_or("");
			std::vector<std::string> run;
			std::stringstream test(cmd);
			std::string segment;
			while(std::getline(test, segment, ' ')) {
				run.push_back(segment);
			}
			auto mask = t->get_as<std::string>("mask").value_or("");
			auto resMask = GetMask(mask);
			auto key = t->get_as<std::string>("key").value_or("");
			auto resKey = keyMap[key];
			keys.push_back(Key(resMask, resKey, "RunCmd", Argument(run)));
		}

		//layout
		std::map <std::string, int> layoutMap = {
			{ "V_STACK_LEFT", V_STACK_LEFT }, 
			{ "V_STACK_RIGHT", V_STACK_RIGHT }, 
			{ "H_STACK_UP", H_STACK_UP }, 
			{ "H_STACK_DOWN", H_STACK_DOWN }, 
			{ "MONOCLE", MONOCLE }, 
			{ "GRID", GRID }, 
			{ "FLOAT", FLOAT }, 
			{ "FIBONACCI", FIBONACCI }, 
			{ "DOUBLE_STACK_VERTICAL", DOUBLE_STACK_VERTICAL }, 
			{ "MODES", MODES } 
		};
		auto layoutTable = config->get_table_array("cmd_layout");
		for (const auto& t : *layoutTable) {
			auto mask = t->get_as<std::string>("mask").value_or("");
			auto resMask = GetMask(mask);
			auto key = t->get_as<std::string>("key").value_or("");
			auto resKey = keyMap[key];
			auto layout = t->get_as<std::string>("layout").value_or("");
			auto resLayout = layoutMap[layout];
			keys.push_back(Key(resMask, resKey, "SwitchMode", Argument(resLayout)));
		}

		//CMD
		std::set<std::string> nullptrFuncs = {
			"TogglePanel",
			"NextWin",
			"PrevWin",
			"SwapMaster",
			"MoveDown",
			"MoveUp",
			"PrevDesktop",
			"ToggleFullscreenClient",
			"KillClient",
			"ToggleFloatClient",
			"RestartMonitors",
			"HideCurClient",
			"HideAllClientOnDescktop",
			"Restart",
		};
		auto keyTable = config->get_table_array("cmd_key");
		for (const auto& t : *keyTable) {
			auto mask = t->get_as<std::string>("mask").value_or("");
			auto resMask = GetMask(mask);
			auto key = t->get_as<std::string>("key").value_or("");
			auto resKey = keyMap[key];
			auto resFunc = t->get_as<std::string>("func").value_or("");
			auto resArg = t->get_as<int>("arg").value_or(0);
			if (resFunc != "") {
				if (nullptrFuncs.find(resFunc) == nullptrFuncs.end()) {
					keys.push_back(Key(resMask, resKey, resFunc, Argument(resArg)));
				} else {
					keys.push_back(Key(resMask, resKey, resFunc, Argument{nullptr}));
				}
			}
		}

	}
};



#endif
