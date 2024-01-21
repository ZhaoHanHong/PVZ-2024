#pragma once
#include <unordered_set>
#include <random>
// #include <graphics.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <Windows.h>

#include "../../gui/lib/include/app.h"
/**
 * FIXME:
 * 
 */
#define IMAGE std::pair<int,int>
#define ExMessage std::pair<int,int>
#define outtextxy(a,b,c) 1
#define Resize(a,b,c)1
#include "unit.h"
#include "game.h"
#include "game_parameters.h"
class Game;
class Unit;
class Plant;
class Parameters;
class Zombie;
class BackGround {
public:
	Game* gm_;
	int map_id_ = 0;
public:
	std::mt19937 gen_;
	std::unordered_set<Unit*> units_;
	std::vector<std::vector<Unit*>> grid_manager_;
public:
	int x_size_, y_size_;
	const int SUN_STEPS =  Parameters::SUN_STEPS;
	const int ZOMBIE_STEPS =  Parameters::ZOMBIE_STEPS;
	const int STEPS = 2;//inital choose plant steps
public:
	//graphics constants(unit: pixels)
	/**
	 * @brief note that the y axis point from top to bottom
	 * 
	 */
	const int CHOOSER_X = 100, CHOOSER_Y = 0;
	const int CARD_START_X = 130, CARD_START_Y = 7;
	const int CARD_END_X = 610;
	const int CARD_NUM = 8;
	const int CARD_X = 54, CARD_Y = 70;
	const int SUN_CNT_X = 130, SUN_CNT_Y = 65;//the place for showing current sun counts
	const int GRID_START_X = 240, GRID_START_Y = 90;//left-top corner
	const int GRID_END_X = 980, GRID_END_Y = 570;//right-bottom corner
	const int GRID_X = 81, GRID_Y = 96;//single grid (average) size
	const int GRID_X_NUM = 10, GRID_Y_NUM = 5;
	const int PANEL_START_X = 24, PANEL_START_Y = 111;
	const int PANEL_X = 53, PANEL_Y = 74;
	const int PANEL_END_X = 439, PANEL_END_Y = 469;
public:
	ExMessage msg_;//interact with user
	Vulkan::App::object_t map_obj_id_;
	Vulkan::App::object_t chooser_obj_id_;  
	Vulkan::App::object_t panel_obj_id_;
	std::unordered_map<std::string, int> units_price_;//price(unit:suns)
public:
	std::pair<int, int> sun_size_;//this is used for collecting sun
public:
	int sun_cnt_ = 50;//initial sun count is 50
	/**
	 * @brief the number of units constructed.
	 * 
	 */
	int unit_cnt_=0;
	std::vector<std::pair<std::string,Vulkan::App::object_t> > bar_;//bar_=chooser
	std::pair<bool, Plant*> placing_plant_;//the plant which is currently bought and following the mouse. If .first is false, then no such plant exist
public:
//Initialize
	BackGround( Game* gm,int id=0);
	void InitPrice();
	void ChoosePlants();
	int UnitCnt();
//Get image based on name and status. Warning: if status out of bound, then this will abort.
//Create
	// int CreateUnit(const std::string& name);
	void CreateSun();
	void CreateZombie();
//Others
	void UpdateUnits();
	void UserClick();
	void Show();

	Plant* MakePlant(std::string name, int x, int y);//used in interacting with bar
	std::pair<int,Unit*> FindMinZombie(int x, int row);//find zombie in given row having least x-coordinate, used in pea attacking
	std::pair<int, Unit*> FindMaxPlant(int x, int row);//find plant in given row having max x-coordinate, used in zombie eating
	std::vector<Unit*> GetZombieInRange(std::pair<int,int> left_top,std::pair<int,int> right_bot);
};