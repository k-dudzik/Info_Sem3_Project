// Game.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Gosu/Gosu.hpp"
#include "Gosu/AutoLink.hpp"
#include "time.h"
using namespace std;

class GameWindow : public Gosu::Window
{
public:
	const uint16_t window_width;
	const uint16_t window_height;

	GameWindow(uint16_t w = 480, uint16_t h = 320) : Window(w, h), window_width(w), window_height(h)
	{
		set_caption("Gosu Tutorial Game");
	}

	void update() override;

	void draw() override;

	void button_down(Gosu::Button button) override;
};

void GameWindow::update()
{

}

void GameWindow::draw()
{
	Gosu::Bitmap pic(100, 100, Gosu::Color::WHITE);
	//Gosu::Bitmap::insert(pic, 20, 20);
	Gosu::Graphics::create_image(pic, 0, 0, 100, 100, 0);
	Gosu::Image img(pic);
	img.draw(0, 0, 0);
	Gosu::Image img2("C:/Users/Public/Pictures/Sample Pictures/Koala.jpg");
	img2.draw(30, 30, 1);
}

void GameWindow::button_down(Gosu::Button button)
{
	switch (button.id())
	{
	default:break;
	}
}

void key(Gosu::Button button)
{
	printf("oh");
}

int main()
{
	GameWindow window;
	window.show();
	window.input().on_button_down = key;
	Gosu::Song song("C:/Users/Public/Music/Sample Music/Kalimba.mp3");
	song.play();
	return 0;
}

