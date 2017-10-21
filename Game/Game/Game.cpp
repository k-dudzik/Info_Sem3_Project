// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Gosu/Gosu.hpp"
#include "Gosu/AutoLink.hpp"
#include "time.h"
#include "math.h"
#include "string.h"
#include "Shlobj.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define TILE_WIDTH 200
#define TILE_HEIGHT 200
using namespace std;

class GameWindow : public Gosu::Window
{
public:
	TCHAR szFolderPath[MAX_PATH];
	Gosu::Bitmap bg_data;
	vector<Gosu::Image> bg_tiles;
	uint16_t row_count;
	double radius = sqrt(pow(WINDOW_WIDTH, 2) + pow(WINDOW_HEIGHT, 2)) / 2.0; //2203
	double y_pos, x_pos;
	double t, alpha, scale;
	uint16_t offset[4] = { 0 };
	uint32_t k = 100;


	GameWindow(uint16_t w = WINDOW_WIDTH, uint16_t h = WINDOW_HEIGHT, bool fullscreen = true) : Window(w, h, fullscreen)
	{
		set_caption("Gosu Tutorial Game");
		
		SHGetSpecialFolderPath(NULL, szFolderPath, CSIDL_PROFILE, FALSE);
		wstring buf = szFolderPath;
		string bg_data_path(buf.begin(), buf.end());
		bg_data_path.append("/Source/Repos/graphics/zoom.earth-ber.png");
		Gosu::load_image_file(bg_data, bg_data_path);
		bg_tiles = Gosu::load_tiles(bg_data, TILE_WIDTH, TILE_HEIGHT); //44 x 44
		row_count = bg_data.width() / TILE_WIDTH;
		y_pos = 4000;
		x_pos = 4000;
		t = 0;
		scale = 1.5;
	}

	void update() override;

	void draw() override;

	void button_down(Gosu::Button button) override;
};

void GameWindow::update()
{
	t += 10.0 / k;
	k++;
	alpha++;
	scale = 1.5 + sin(t) * 0.5;
	y_pos = 4000 + sin(t) * k; //y shift
	x_pos = 4000 + cos(t) * k; //x shift

	offset[0] = (y_pos - radius) / TILE_HEIGHT; //y min
	offset[1] = (y_pos + radius) / TILE_HEIGHT; //y max
	offset[2] = (x_pos - radius) / TILE_WIDTH; //x min
	offset[3] = (x_pos + radius) / TILE_WIDTH; //x max 
}

void GameWindow::draw()
{
	for (int i = offset[0]; i <= offset[1]; i++)
	{
		for (int j = offset[2]; j <= offset[3]; j++)
		{
			bg_tiles.at(i * (row_count) + j).draw_rot(
				WINDOW_WIDTH / 2, 
				WINDOW_HEIGHT / 2, 
				0, 
				alpha, 
				(x_pos - TILE_WIDTH * j) / double(TILE_WIDTH), 
				(y_pos - TILE_HEIGHT * i) / double(TILE_HEIGHT),
				scale,
				scale);
		}
	}
}

void GameWindow::button_down(Gosu::Button button)
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

