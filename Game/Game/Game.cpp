// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Gosu/Gosu.hpp"
#include "Gosu/AutoLink.hpp"
#include "time.h"
#include "math.h"
#include "string.h"
#include "Shlobj.h"
#include <sstream>
#include <iomanip>
#include <iostream>

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

class GameWindow : public Window
{
public:
	TCHAR szFolderPath[MAX_PATH];
	Bitmap bg_data;
	Image *bg_image;
	Image *drone_array[484] = { NULL };
	Image *missile_array[2] = { NULL };
	Image *sprite0_array[104] = { NULL };
	uint8_t missile_launch = 0;
	uint16_t sprite_mask = 0;
	uint8_t sprite0l_cnt = 0;
	uint8_t sprite0r_cnt = 0;
	uint16_t drone_offset;

	double missile_pos[4] = { 0 }; //lx, ly, rx, ry
	double y_pos, x_pos;
	double t, scale;

	double speed_integrator = 0;
	double heading = 0;
	double heading_integrator = 0;
	double missile_integrator[2] = { 0 };


	GameWindow(uint16_t w = WINDOW_WIDTH, uint16_t h = WINDOW_HEIGHT, bool fullscreen = true) : Window(w, h, fullscreen)
	{
		set_caption("Gosu Game");
		
		SHGetSpecialFolderPath(NULL, szFolderPath, CSIDL_PROFILE, FALSE);
		wstring buf = szFolderPath;
		string graphics_path(buf.begin(), buf.end());
		graphics_path.append("/Source/Repos/graphics");

		//Background
		stringstream bg_data_path;
		bg_data_path << graphics_path << "/zoom.earth-ber.jpg";
		//load_image_file(bg_data, bg_data_path.str());
		bg_image = new Image(bg_data_path.str());

		//Drone
		stringstream drone_data_path;
		for (int i = 0; i <= 483; i++)
		{
			drone_data_path << graphics_path << "/drone/resize/reduced_size/render" << setfill('0') << setw(4) << i << ".png";
			drone_array[i] = new Image(drone_data_path.str());
			cout << drone_data_path.str() << endl;
			drone_data_path.str(std::string());
			drone_data_path.clear();
		}

		for (int i = 0; i <= 483; i++)
		{
			if (drone_array[i])
			{
				drone_array[i]->height(); //first operation on element takes ages...no idea why
				cout << i << endl;
			}
		}

		//Missiles
		stringstream missile_data_path;
		missile_data_path << graphics_path << "/drone/missiles/missile_l.png";
		missile_array[0] = new Image(missile_data_path.str());
		missile_data_path.str(std::string());
		missile_data_path.clear();
		missile_data_path << graphics_path << "/drone/missiles/missile_r.png";
		missile_array[1] = new Image(missile_data_path.str());

		//Missile Sprites
		stringstream sprite0_data_path;
		for (int i = 0; i <= 103; i++)
		{
			sprite0_data_path << graphics_path << "/rocket/edited/flame" << i << ".png";
			sprite0_array[i] = new Image(sprite0_data_path.str());
			sprite0_data_path.str(std::string());
			sprite0_data_path.clear();
		}

		missile_pos[0] = WINDOW_WIDTH / 2 - 92;
		missile_pos[1] = WINDOW_HEIGHT / 2 + 38;
		missile_pos[2] = WINDOW_WIDTH / 2 + 74;
		missile_pos[3] = WINDOW_HEIGHT / 2 + 38;

		y_pos = 4400;
		x_pos = 4400;
		t = 0; //ms
		scale = 1;
	}

	~GameWindow()
	{
		delete bg_image;
		for(int i = 0; i <= 483; i++)
			delete drone_array[i];
		delete missile_array[0];
		delete missile_array[1];
		for (int i = 0; i <= 103; i++)
			delete sprite0_array[i];
	}

	void update() override;

	void draw() override;

	void button_down(Button button) override;

	void button_up(Button button) override;

	double speed_eval();

	double heading_eval();

	vector<double> missile_eval();
};

double GameWindow::speed_eval()
{
	bool w_key = input().down(KB_W);
	bool s_key = input().down(KB_S);
	double speed = 20;
	if (w_key != s_key)
	{
		if (w_key)
			speed_integrator += update_interval() / 1000.0;
		else
			speed_integrator -= update_interval() / 1000.0;
	}
	else
		speed_integrator += 0.0005 * update_interval() * ((speed_integrator < 0) - (speed_integrator > 0));

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

double GameWindow::heading_eval()
{
	bool a_key = input().down(KB_A);
	bool d_key = input().down(KB_D);
	double rot_speed = 0;

	if (a_key != d_key)
	{
		if (a_key)
			heading_integrator += update_interval() / 1000.0;
		else
			heading_integrator -= update_interval() / 1000.0;
	}
	else
	{
		if (abs(heading_integrator) < 0.017) //rot_speed < 1
			heading_integrator = 0;
		heading_integrator += 0.0005 * update_interval() * ((heading_integrator < 0) - (heading_integrator > 0));
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

	return rot_speed;
}

vector<double> GameWindow::missile_eval()
{
	vector<double> missile_speed = { 0, 0 };
	double missile_speed_fac = 1;
	if (missile_launch & 0x01)
	{
		missile_integrator[0]++;
		if (missile_pos[1] < -107)
		{
			missile_launch &= ~0x01;
			missile_integrator[0] = 0;
			missile_pos[1] = WINDOW_HEIGHT / 2 + 38;
		}
		missile_speed.at(0) = missile_speed_fac * missile_integrator[0];
	}
	if (missile_launch & 0x02)
	{
		missile_integrator[1]++;
		if (missile_pos[3] < -107)
		{
			missile_launch &= ~0x02;
			missile_integrator[1] = 0;
			missile_pos[3] = WINDOW_HEIGHT / 2 + 38;
		}
		missile_speed.at(1) = missile_speed_fac * missile_integrator[1];
	}

	return missile_speed;
}

void GameWindow::update()
{
	double speed = speed_eval();
	heading += heading_eval() * update_interval() / 1000.0;
	
	drone_offset = 121 * (missile_launch) + 60 - heading_integrator * 60;

	y_pos += offset_y(-heading, speed * update_interval() / 1000.0);
	x_pos += offset_x(-heading, speed * update_interval() / 1000.0);

	if (missile_launch)
	{
		vector<double> missile_speed = missile_eval();
		missile_pos[1] -= missile_speed.at(0);
		missile_pos[3] -= missile_speed.at(1);
	}

	sprite0l_cnt += !!(sprite_mask & 0x01) * 1.75;
	sprite0r_cnt += !!(sprite_mask & 0x02) * 1.75;
	if (sprite0l_cnt > 103)
	{
		sprite0l_cnt = 0;
		sprite_mask &= ~0x01;
	}
	if (sprite0r_cnt > 103)
	{
		sprite0r_cnt = 0;
		sprite_mask &= ~0x02;
	}

	t += this->update_interval();
}

void GameWindow::draw()
{
	bg_image->draw_rot(
		WINDOW_WIDTH / 2, 
		WINDOW_HEIGHT / 2 + 100,
		0, 
		heading, 
		x_pos / bg_image->width(), 
		y_pos / bg_image->height(),
		scale,
		scale);
	drone_array[drone_offset]->draw(WINDOW_WIDTH / 2 - 450, WINDOW_HEIGHT / 2 - 200, 10);

	if (missile_launch & 0x01)
		missile_array[0]->draw(missile_pos[0], missile_pos[1], 4);
	if (missile_launch & 0x02)
		missile_array[1]->draw(missile_pos[2], missile_pos[3], 5);

	if (sprite_mask & 0x01)
		//sprite0_array[sprite0l_cnt]->draw(WINDOW_WIDTH / 2 - 186, 0, 2);
	if (sprite_mask & 0x02)
		//sprite0_array[sprite0r_cnt]->draw(WINDOW_WIDTH / 2 - 20, 0, 3);
}

void GameWindow::button_down(Button button)
{
	switch (button.id())
	{
	case KB_F: 
		if (heading_integrator == 0)
		{
			missile_launch |= 0x01;
			sprite_mask |= 0x01;
		}
		break;
	case KB_G:
		if (heading_integrator == 0)
		{
			missile_launch |= 0x02;
			sprite_mask |= 0x02;
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

int main()
{
	GameWindow window;
	window.show();
	//Gosu::Song song("C:/Users/Public/Music/Sample Music/Kalimba.mp3");
	//song.play();
	return 0;
}

