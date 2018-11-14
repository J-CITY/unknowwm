#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <map>
#include "utils.h"
#include <memory>
#include <algorithm>
#include "toml.h"

class WindowManager;

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
		                                 H_STACK_DOWN, MONOCLE, GRID, FIBONACCI, FLOAT};
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

	Config(std::string configPath="") {
		Parse(configPath);


		//Autostart
		std::vector<char*> autos;
		autos.push_back("feh");
		autos.push_back("--bg-scale");
		autos.push_back("/home/daniil/WM/1.jpg");
		autos.push_back(nullptr);
		autostart.push_back(Argument(autos));
		
		autos.clear();
		autos.push_back("conky");
		autos.push_back(nullptr);
		autostart.push_back(Argument(autos));
/*
		autos.clear();
		autos.push_back("python");
		autos.push_back("/home/daniil/WM/dock.py");
		autos.push_back(nullptr);
		autostart.push_back(Argument(autos));
		*/
		//Mouse button
		buttons.push_back(Button(MOD1,    Button1,  "MouseMotion",             Argument{.i = MOVE}));
		buttons.push_back(Button(MOD1,    Button3,  "MouseMotion",             Argument{.i = RESIZE}));
		
		//Rules
		rules.push_back(AppRule( "Thunar",     0,       3,    true,   true));

		keys.push_back(Key(MOD1,          XK_b,      "TogglePanel",            Argument{nullptr}));
		keys.push_back(Key(MOD1|CONTROL,  XK_r,      "Quit",                   Argument{.i = 0}));
		keys.push_back(Key(MOD1|CONTROL,  XK_q,      "Quit",                   Argument{.i = 1}));
		std::vector<char*> a;
		a.push_back("xfce4-terminal");
		a.push_back(nullptr);
		keys.push_back(Key(MOD1|SHIFT,    XK_Return, "RunCmd",                 Argument{.com = a}));
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
	}


	void Parse(std::string path) {
		if (path == "") {
			return;
		}
		auto config = cpptoml::parse_file(path);

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
			{ "MODES",MODES }
		};
		auto defModeKey = config->get_qualified_as<std::string>("main.DEFAULT_MODE").value_or("V_STACK_LEFT");
		this->DEFAULT_MODE          = defMode[defModeKey];//Default layout for desktop

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
		

	}
};



























#endif
