#include "game.h"
void Game::LoadAll(){
	auto size=LoadAndGetImageSize("map/map0.jpg");
	assert(size.first);
	std::tie(bkg_->x_size_,bkg_->y_size_)=std::tie(x_size_,y_size_)=size;
	app_=new Vulkan::App(x_size_,y_size_,"PVZ-2024");	
	image_dict["map"]={LoadOneImageNoFailure("map/map0.jpg")};
	image_dict["chooser"]={LoadOneImageNoFailure("others/chooser.png")};
	image_dict["panel"]={LoadOneImageNoFailure("others/panel.png")};
//sun	
	LoadMultiple("sun");
	bkg_->sun_size_=app_->GetImageSize(image_dict["sun"][0]);
	LoadMultiple("pea");
//plants
	LoadPlant("peashooter");
	LoadPlant("sunflower");
	LoadPlant("cherrybomb");
//zombies
	LoadZombie("zombie");
}

Game::Game() :bkg_(new BackGround(this)),gen(time(NULL)){
	y_size_ = bkg_->y_size_;
	LoadAll();
}
std::pair<int,int> Game::LoadAndGetImageSize(const std::string& name)const{
	return app_->LoadAndGetImageSize(ChangeName(name));
}
std::string Game::ChangeName(const std::string &name) const {
	std::string final_name;
	if(name[0]=='/'){
        final_name="assets"+name;
    }else{
        final_name="assets/"+name;
    }
	return final_name;
}

Vulkan::App::image_t Game::LoadOneImage(const std::string& name){
	return app_->LoadOneImage(ChangeName(name));
}
Vulkan::App::image_t Game::LoadOneImageNoFailure(const std::string& name){
	return app_->LoadOneImageNoFailure(ChangeName(name));
}


void Game::LoadMultiple(const std::string& unit_name) {
	assert(!image_dict.count(unit_name) && "Must load each unit exactly one time");
	int i = 1;
	std::vector<int> images;
	while (true) {
		std::string path=unit_name+"/"+Helper::NumToString(i)+".png";
		auto res=LoadOneImage(path);
		if(res==app_->LOAD_IMAGE_FAIL){
			break;
		}
		i++;
		images.push_back(res);
	}
	assert(images.size() && "must has at least one image");
	image_dict[unit_name] = images;
}
void Game::LoadPlant(const std::string& name) {
	LoadMultiple(name);
	plant_available.push_back(name);
	image_dict[name+"_card"]={LoadOneImageNoFailure("cards/"+name+".png")};
}
void Game::LoadZombie(const std::string& name) {
	LoadMultiple(name);
	LoadMultiple(name+"/head");
	LoadMultiple(name+"/body");
	LoadMultiple(name+"/attack");
	LoadMultiple(name+"/explode");
}
std::pair<int,int> Game::GetImageSize(const std::string &name){
	return app_->GetImageSize(image_dict[name][0]);
}
const int Game::GetImageNum(const std::string& name){
	return image_dict[name].size();
}
void Game::Draw(int xp,int yp,Vulkan::App::object_t id,Vulkan::App::image_t im){
	app_->AddToDrawList(xp,yp,id,im);
}
Vulkan::App::image_t Game::GetImage(const std::string& name,int frame){
	return image_dict[name][frame];
}
Vulkan::App::image_t Game::GetCardImage(const std::string& name){
	return GetImage(name+"_card");
}
void Game::Start(){
	Init();
	while(app_->IsRunning()){
		app_->PollEvent();
		Update();
	}
}
void Game::Show()const {
	bkg_->Show();
}
void Game::Init() {
	// initgraph(x_size_ + 10, y_size_ + 10);
	// settextcolor(GREEN);
	// setbkcolor(BLACK);
	// setlinecolor(BLACK);
	//show starter page

	bkg_->ChoosePlants();
}
Vulkan::App::object_t Game::MakeObject(const std::string& name){
	if(name==""){
		return app_->AddOneObject();
	}else{
		return app_->AddOneObject(GetImage(name));
	}
}
void Game::Draw(int xp,int yp,Vulkan::App::object_t id,const std::string& name,int frame){
	Draw(xp,yp,id,GetImage(name,frame));
}
void Game::Update() {
	bkg_->UserClick();
	auto now=app_->GetTime();
	// printf("now is %d, next time is %d\n",now,(render_time_+1)* Parameters::RENDER_TIME);
	if(now>(render_time_+1)* Parameters::RENDER_TIME){
		app_->DrawFrame();
		render_time_=now/Parameters::RENDER_TIME;
		bkg_->Show();
	}
	if(now <= (time_+1) * Parameters::UPDATE_TIME){
		return;
	}
	time_=now/Parameters::UPDATE_TIME;
	printf("time %llu\n", time_);
	if (time_ >= nxt_time_sun) {
		bkg_->CreateSun();
		std::uniform_int_distribution<unsigned long long> distr(0, SUN_TIME);
		nxt_time_sun = time_ + distr(gen);
	}
	if (time_ >= nxt_time_zombie) {
		bkg_->CreateZombie();
		std::uniform_int_distribution<unsigned long long> distr(0, ZOMBIE_TIME);
		nxt_time_zombie = time_ + distr(gen);
	}
	bkg_->UpdateUnits();
}
void Game::Lose() {
	in_game_ = false;
	/**
	 * TODO: add the lose effect here
	 * 
	 */
}