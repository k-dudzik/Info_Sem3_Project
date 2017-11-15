// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "time.h"
#include "math.h"
#include "string.h"
#include "Shlobj.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>

//#include "drone.h"
#include "Game.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define s8 int8_t
#define s16 int16_t
#define s32 int32_t

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

using namespace std;
using namespace Gosu;

Background::Background(string bg_data_path)
{
	bg_image = new Image(bg_data_path);
	cout << bg_data_path << endl;
	y_pos = 4400;
	x_pos = 4400;
}

Background::~Background()
{
	delete bg_image;
}

Drone::Drone(string drone_data_path, string name_base, uint8_t w, uint16_t n, uint8_t v, string missile_data_path, string missile_name_base, string sprite_data_path) : n(n), v(v)
{
	//Drone
	stringstream filename;
	drone_array = new Image*[n * v];
	for (volatile int i = 0; i < (n * v); i++)
	{
		filename << drone_data_path << name_base << setfill('0') << setw(w) << i << ".png";
		drone_array[i] = new Image(filename.str());
		cout << filename.str() << endl;
		filename.str(std::string());
		filename.clear();
		//Bitmap buf(900, 600);
		//memcpy(buf.data(), u8_drone_array[i], 4 * 900 * 600);
		//drone_array[i] = new Image(buf);
	}

	for (int i = 0; i < (n * v); i++)
		drone_array[i]->height(); //first operation on element takes ages...no idea why

	//Missile		
	missile_left = new Missile(missile_data_path, missile_name_base, "l.png");
	missile_right = new Missile(missile_data_path, missile_name_base, "r.png");

	//Sprite
	missile_sprite = new Missile_Sprite(sprite_data_path);
}

Drone::~Drone()
{
	for (int i = 0; i < (n * v); i++)
	{
		delete drone_array[i];
	}
	delete[] drone_array;
	delete missile_left;
	delete missile_right;
	delete missile_sprite;
}

double Drone::speed_eval(bool w_key, bool s_key, double update_interval)
{
	if (w_key != s_key)
	{
		if (w_key)
			speed_integrator += update_interval / 1000.0;
		else
			speed_integrator -= update_interval / 1000.0;
	}
	else
		speed_integrator += 0.0005 * update_interval * ((speed_integrator < 0) - (speed_integrator > 0));

	if (speed_integrator > 3)
	{
		speed_integrator = 3;
		speed = 40;
	}
	else if (speed_integrator < -3)
	{
		speed_integrator = -3;
		speed = 10;
	}

	if (speed_integrator > 0)
		speed = 20 * (1 - 2.0 / 27.0 * pow(speed_integrator, 3) + 1.0 / 3.0 * pow(speed_integrator, 2));
	else
		speed = 20 / (1 - 2.0 / 27.0 * pow(-speed_integrator, 3) + 1.0 / 3.0 * pow(speed_integrator, 2));

	return speed;
}

double Drone::heading_eval(bool a_key, bool d_key, double update_interval)
{
	if (a_key != d_key)
	{
		if (a_key)
			heading_integrator += update_interval / 1000.0;
		else
			heading_integrator -= update_interval / 1000.0;
	}
	else
	{
		if (abs(heading_integrator) < 0.017) //rot_speed < 1
			heading_integrator = 0;
		heading_integrator += 0.0005 * update_interval * ((heading_integrator < 0) - (heading_integrator > 0));
	}

	if (heading_integrator > 1)
	{
		heading_integrator = 1;
		rot_speed = 30;
	}
	else if (heading_integrator < -1)
	{
		heading_integrator = -1;
		rot_speed = -30;
	}

	if (heading_integrator > 0)
		rot_speed = -30 * pow(heading_integrator, 2) + 60 * heading_integrator;
	else
		rot_speed = 30 * pow(heading_integrator, 2) + 60 * heading_integrator;

	heading += rot_speed * update_interval / 1000.0;
	return heading;
}

void Drone::update_missiles(double time) {
	if (missile_left->missile_launch) {
		missile_left->update(time);
	}
	if (missile_right->missile_launch) {
		missile_right->update(time);
	}
}

void Drone::draw_missiles() {
	if (missile_left->missile_launch)
		missile_left->draw();
	if (missile_right->missile_launch)
		missile_right->draw();
}

void Drone::setup() {
	missile_left->x_pos = WINDOW_WIDTH / 2 - 92;
	missile_left->y_pos = WINDOW_HEIGHT / 2 + 38;
	missile_right->x_pos = WINDOW_WIDTH / 2 + 74;
	missile_right->y_pos = WINDOW_HEIGHT / 2 + 38;
}

Missile::Missile(string missile_data_path, string name_base, string name_suffix)
{
	//Missile
	stringstream filename;
	filename << missile_data_path << name_base << name_suffix;
	missile_image = new Image(filename.str());
}

Missile::~Missile()
{
	delete missile_image;
}

void Missile::draw() {
	missile_image->draw(x_pos, y_pos, 4);
}

void Missile::update(double time) {
	this->speed_eval(time);
	this->y_pos -= this->speed;
}

double Missile::speed_eval(double time)
{
	if (missile_launch)
	{
		missile_integrator++;
		if (y_pos < -107)
		{
			missile_integrator = 0;
			if (!delay_timestamp) {
				delay_timestamp = time; // reload timer starten
			}
			if ((time - delay_timestamp) > 5000) { // 5000ms, dann nachladen
				missile_launch = FALSE;
				y_pos = WINDOW_HEIGHT / 2 + 38;
				delay_timestamp = 0;
			}
		}
		speed = speed_fac * missile_integrator;
	}

	return speed;
}

Missile_Sprite::Missile_Sprite(string sprite_data_path)
{
	stringstream filename;
	for (int i = 0; i <= 103; i++)
	{
		filename << sprite_data_path << i << ".png";
		sprite_array[i] = new Image(filename.str());
		filename.str(std::string());
		filename.clear();
	}
}

Missile_Sprite::~Missile_Sprite()
{
	for (int i = 0; i <= 103; i++)
		delete sprite_array[i];
}

GameWindow::GameWindow(uint16_t w = WINDOW_WIDTH, uint16_t h = WINDOW_HEIGHT, bool fullscreen = true) : Window(w, h, fullscreen)
{
	set_caption("Gosu Game");

	SHGetSpecialFolderPath(NULL, szFolderPath, CSIDL_PROFILE, FALSE);
	wstring buf(szFolderPath);
	string graphics_path(buf.begin(), buf.end());
	graphics_path.append("/Source/Repos/graphics");

	//Background
	stringstream bg_data_path;
	bg_data_path << graphics_path << "/zoom.earth-ber.jpg";
	background = new Background(bg_data_path.str());

	//Drone
	//Missiles
	//Missile Sprites
	stringstream drone_data_path;
	drone_data_path << graphics_path << "/drone/resize/reduced_size/";
	stringstream missile_data_path;
	missile_data_path << graphics_path << "/drone/missiles/";
	stringstream sprite0_data_path;
	sprite0_data_path << graphics_path << "/rocket/edited/flame";

	drone = new Drone(drone_data_path.str(), "render", 4, 121, 4, missile_data_path.str(), "missile_", sprite0_data_path.str());
	drone->setup();

	background->y_pos = 4400;
	background->x_pos = 4400;
	t = 0; //ms

	obj_handler = new Objective_Handler();
}

GameWindow::~GameWindow()
{
	delete background;
	delete drone;
}

void GameWindow::input_eval()
{
	w_key = input().down(KB_W);
	s_key = input().down(KB_S);
	a_key = input().down(KB_A);
	d_key = input().down(KB_D);
}

void GameWindow::update()
{
	input_eval();

	drone->speed_eval(w_key, s_key, update_interval());
	drone->heading_eval(a_key, d_key, update_interval());
	
	drone->drone_offset = 121 * (drone->missile_left->missile_launch | drone->missile_right->missile_launch << 1) + 60 - drone->heading_integrator * 60;
	background->y_pos += offset_y(-drone->heading, drone->speed * update_interval() / 1000.0);
	background->x_pos += offset_x(-drone->heading, drone->speed * update_interval() / 1000.0);

	drone->update_missiles(t);

	drone->missile_sprite->sprite_cnt_l += drone->missile_sprite->sprite_l * 1.75;
	drone->missile_sprite->sprite_cnt_r += drone->missile_sprite->sprite_r * 1.75;
	if (drone->missile_sprite->sprite_cnt_l > 103)
	{
		drone->missile_sprite->sprite_cnt_l = 0;
		drone->missile_sprite->sprite_l = FALSE;
	}
	if (drone->missile_sprite->sprite_cnt_r > 103)
	{
		drone->missile_sprite->sprite_cnt_r = 0;
		drone->missile_sprite->sprite_r = 0;
	}

	obj_handler->update();

	t += this->update_interval();
}

void GameWindow::draw()
{
	background->bg_image->draw_rot(
		WINDOW_WIDTH / 2, 
		WINDOW_HEIGHT / 2 + 100,
		0, 
		drone->heading, 
		background->x_pos / background->bg_image->width(), 
		background->y_pos / background->bg_image->height(),
		background->scale,
		background->scale);
	drone->drone_array[drone->drone_offset]->draw(WINDOW_WIDTH / 2 - 450, WINDOW_HEIGHT / 2 - 200, 10);

	drone->draw_missiles();

	//if (drone->missile_sprite->sprite_l)
		//sprite0_array[sprite0l_cnt]->draw(WINDOW_WIDTH / 2 - 186, 0, 2);
	//if (drone->missile_sprite->sprite_r)
		//sprite0_array[sprite0r_cnt]->draw(WINDOW_WIDTH / 2 - 20, 0, 3);

	obj_handler->draw();
}

void GameWindow::button_down(Button button)
{
	switch (button.id())
	{
	case KB_F: 
		if (drone->heading_integrator == 0)
		{
			drone->missile_left->missile_launch = TRUE;
			drone->missile_sprite->sprite_l = TRUE;
		}
		break;
	case KB_G:
		if (drone->heading_integrator == 0)
		{
			drone->missile_right->missile_launch = TRUE;
			drone->missile_sprite->sprite_r = TRUE;
		}
		break;
	default:break;
	}
}

void GameWindow::button_up(Button button)
{
	switch (button.id())
	{
	default:break;
	}
}

Objective::Objective(double x, double y, string t, string m):x_pos(x),y_pos(y),title(t),mission(m) {

}

Objective::~Objective() {

}

void Objective::draw() {
	if (cleared) {
		//Bild von Krater oder abgehakt oder so
	}
	else if (active) {
		//Zielmarkierung
	}
	else {
		//Pfeil oder Nadel wie bei google maps?
	}
}

bool Objective::update() {
	return cleared;
}

bool Objective::destroy() {
	//Animation für Zerstörung?!
	cleared = TRUE;
	return cleared;
}

bool Objective::operator== (const Objective& o1) {
	if (this->x_pos == o1.x_pos && this->y_pos == o1.y_pos) {
		return TRUE;
	}
	return FALSE;
}

Objective_Handler::Objective_Handler():selected_objective(Objective(0, 0, "","")) {
	add_objective(4000, 4000, "Erste Mission", "Zerstöre den feindlichen Truck im Mittelpunkt der Karte");
	selected_objective = objective_list.at(0);
}

void Objective_Handler::select_new_objective() {
	for (auto i = objective_list.begin(); i != objective_list.end(); i++) {
		if (!i->cleared) {
			selected_objective = *i;
		}
	}
}

void Objective_Handler::add_objective(double x, double y, string t, string m) {
	auto o = Objective(x, y, t, m);
	objective_list.push_back(o);
}

void Objective_Handler::update() {
	for (auto i = objective_list.begin(); i != objective_list.end(); i++) {
		if (i->update() && selected_objective == *i) {
			select_new_objective();
		}
	}
}

void Objective_Handler::draw() {
	for (auto i = objective_list.begin(); i != objective_list.end(); i++) {
		i->draw();
	}
}

int main()
{
	GameWindow window;
	window.show();
	//Gosu::Song song("C:/Users/Public/Music/Sample Music/Kalimba.mp3");
	//song.play();
	return 0;
}

