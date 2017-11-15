#pragma once

#include "Gosu/Gosu.hpp"
#include "Gosu/AutoLink.hpp"
#include <vector>

using namespace std;
using namespace Gosu;

class Background;
class Drone;
class Missile;
class Missile_Sprite;
class GameWindow;
class Objective;
class Objective_Handler;

class Background
{
public:
	Image *bg_image;
	double y_pos, x_pos;
	double scale = 1;
	uint8_t z_pos = 1;

	Background(string bg_data_path);
	~Background();
};

class Drone
{
public:
	uint16_t n = 0;
	uint8_t v = 0;

	Image **drone_array;
	Missile *missile_left = NULL;
	Missile *missile_right = NULL;
	uint16_t drone_offset;
	Missile_Sprite *missile_sprite = NULL;

	double speed = 20;
	double speed_integrator = 0;
	double heading = 0;
	double heading_integrator = 0;
	double rot_speed = 0;
	double scale = 1;
	uint8_t z_pos = 10;

	Drone(string drone_data_path, string name_base, uint8_t w, uint16_t n, uint8_t v, string missile_data_path, string missile_name_base, string sprite_data_path);
	~Drone();

	double speed_eval(bool w_key, bool s_key, double update_interval);
	double heading_eval(bool a_key, bool d_key, double update_interval);
	void update_missiles(double time);
	void draw_missiles();
	void setup();
};

class Missile
{
public:
	Image *missile_image = NULL;
	bool missile_launch = FALSE;
	double y_pos, x_pos;
	uint8_t z_pos = 4;
	double speed = 0;
	double missile_integrator = 0;
	const double speed_fac = 1;
	double delay_timestamp = 0;

	Missile(string missile_data_path, string name_base, string name_suffix);
	~Missile();

	void update(double time);
	void draw();
	double speed_eval(double time);
};

class Missile_Sprite
{
public:
	Image *sprite_array[104] = { NULL };
	bool sprite_l = FALSE;
	bool sprite_r = FALSE;
	uint8_t sprite_cnt_l = 0;
	uint8_t sprite_cnt_r = 0;

	Missile_Sprite(string sprite_data_path);
	~Missile_Sprite();
};

class GameWindow : public Window
{
public:
	Objective_Handler *obj_handler;
	Background *background;
	Drone *drone;
	TCHAR szFolderPath[MAX_PATH];
	double t;
	uint8_t mode = 0;

	//Input (evtl in eigene Klasse)
	bool w_key;
	bool s_key;
	bool a_key;
	bool d_key;

	GameWindow(uint16_t w, uint16_t h, bool fullscreen);

	~GameWindow();

	void update() override;

	void draw() override;

	void button_down(Button button) override;

	void button_up(Button button) override;

	void input_eval();
};

class Objective
{
public:
	double x_pos = 0; //x Position auf Bild der Karte
	double y_pos = 0; //y Position auf Bild der Karte
	bool active = FALSE;
	bool cleared = FALSE;
	string title = "";
	string mission = "";

	Objective(double x, double y, string t, string m);
	~Objective();

	void draw();
	bool update();
	bool destroy();

	bool operator== (const Objective& o1);
};

class Objective_Handler
{
public:
	Objective_Handler();

	vector<Objective> objective_list;
	Objective selected_objective; //Erstes Objective muss beim Start erstellt und ausgewählt werden!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	void select_new_objective();
	void add_objective(double x, double y, string t, string m);
	void update();
	void draw();
};