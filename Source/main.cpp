#include "gpch.h"
#include "Engine/Application.h"
#include "Game/Game.h"

int main() {
	ApplicationSpecifications appSpecs{};
	appSpecs.name = "Five Nights At Freddy";
	appSpecs.width = 800;
	appSpecs.height = 800;
	appSpecs.appVersion = 0.5;
	Application app(appSpecs);
	MEM::Ref<FiveNightsAtFreddys> fnaf = MEM::CreateRef<FiveNightsAtFreddys>();
	app.SetGame(fnaf);
	app.Run();
	return 0;
}