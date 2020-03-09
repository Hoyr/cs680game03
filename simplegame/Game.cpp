#include "Game.hpp"
#include "DungeonActorComponent.hpp"
#include "NewtonPhysicsComponent.hpp"
#include <gamelib_story_screen.hpp>

constexpr int SOUND_BLIP = 6;

void Game::init() {
	GameLib::Locator::provide(&context);
	if (context.audioInitialized())
		GameLib::Locator::provide(&audio);
	GameLib::Locator::provide(&input);
	GameLib::Locator::provide(&graphics);
	GameLib::Locator::provide(&world);
	GameLib::Locator::provide(&box2d);

	box2d.init();

	audio.setVolume(0.2f);

	PlaySoundCommand play0(0, false);
	PlaySoundCommand play1(1, false);
	PlaySoundCommand play2(2, false);
	PlaySoundCommand play3(3, false);
	PlayMusicCommand playMusic1(0);
	PlayMusicCommand playMusic2(1);
	PlayMusicCommand playMusic3(2);

	input.back = &quitCommand;
	input.start = &shakeCommand;
	input.axis1X = &xaxisCommand;
	input.axis1Y = &yaxisCommand;
}


void Game::kill() {
	double totalTime = stopwatch.stop_s();
	HFLOGDEBUG("Sprites/sec = %5.1f", spritesDrawn / totalTime);
	HFLOGDEBUG("Frames/sec = %5.1f", frames / totalTime);

	actorPool.clear();
}


void Game::main(int argc, char** argv) {
	init();
	loadData();
	showIntro();
	initLevel(1);
	if (playGame()) {
		showWonEnding();
	} else {
		showLostEnding();
	}
	kill();
}


void Game::loadData() {
	for (auto sp : searchPaths) {
		context.addSearchPath(sp);
	}
	SDL_Texture* testPNG = context.loadImage("godzilla.png");
	SDL_Texture* testJPG = context.loadImage("parrot.jpg");
	graphics.setTileSize({ 32, 32 });
	int spriteCount = context.loadTileset(0, 32, 32, "SPAAACE.png");
	if (!spriteCount) {
		HFLOGWARN("Tileset not found");
	}
	context.loadTileset(GameLib::LIBXOR_TILESET32, 32, 32, "LibXORColors32x32.png");

	context.loadAudioClip(0, "starbattle-bad.wav");
	context.loadAudioClip(1, "starbattle-dead.wav");
	context.loadAudioClip(2, "starbattle-endo.wav");
	context.loadAudioClip(3, "starbattle-exo.wav");
	context.loadAudioClip(4, "starbattle-ok.wav");
	context.loadAudioClip(5, "starbattle-pdead.wav");
	context.loadAudioClip(SOUND_BLIP, "blip.wav");
	context.loadMusicClip(0, "starbattlemusic1.mp3");
	context.loadMusicClip(1, "starbattlemusic2.mp3");
	context.loadMusicClip(2, "distoro2.mid");

	gothicfont.load("fonts-japanese-gothic.ttf", 36);
	minchofont.load("fonts-japanese-mincho.ttf", 36);

	worldPath = context.findSearchPath(worldPath);
	if (!world.load(worldPath)) {
		HFLOGWARN("world.txt not found");
	}
}


void Game::initLevel(int levelNum) {
	
	auto NewDungeonActor = []() { return std::make_shared<GameLib::DungeonActorComponent>(); };
	auto NewInput = []() { return std::make_shared<GameLib::SimpleInputComponent>(); };
	auto NewRandomInput = []() { return std::make_shared<GameLib::RandomInputComponent>(); };
	auto NewActor = []() { return std::make_shared<GameLib::ActorComponent>(); };
	auto NewPhysics = []() { return std::make_shared<GameLib::SimplePhysicsComponent>(); };
	auto NewGraphics = []() { return std::make_shared<GameLib::SimpleGraphicsComponent>(); };

	float cx = world.worldSizeX * 0.5f;
	float cy = world.worldSizeY * 0.5f;
	float speed = (float)graphics.getTileSizeX();
	box2d.world_.SetAllowSleeping(false);
	box2d.setGravity({0,0});

	GameLib::ActorPtr actor;
	actor = _makeActor(cx + 6, cy, speed, 0, NewInput(), NewActor(), NewPhysics(), NewGraphics());
	world.addDynamicActor(actor);

	actor = _makeActor(cx + 6, cy + 4.5f, 4, 11, nullptr, NewActor(), NewPhysics(), NewGraphics());
	world.addStaticActor(actor);

	actor = _makeActor(cx + 10, cy - 4, 4, 12, nullptr, NewActor(), NewPhysics(), NewGraphics());
	world.addStaticActor(actor);

	actor = _makeActor(16, 6, 4, 13, NewRandomInput(), NewActor(), NewPhysics(), NewGraphics());
	world.addDynamicActor(actor);

	actor = _makeActor(6, 6, 4, 13, NewRandomInput(), NewActor(), NewPhysics(), NewGraphics());
	world.addDynamicActor(actor);

	actor = _makeActor(10, 10, 4, 11, nullptr, NewActor(), NewPhysics(), NewGraphics());
	world.addTriggerActor(actor);

	actor = _makeActor(19, 10, 4, 12, nullptr, NewActor(), NewPhysics(), NewGraphics());
	world.addTriggerActor(actor);
}


void Game::showIntro() {
	// context.playMusicClip(0);
	GameLib::StoryScreen ss;
	ss.setBlipSound(SOUND_BLIP);
	if (!ss.load("dialog.txt")) {
		// do something default
		ss.setFont(0, "URWClassico-Bold.ttf", 2.0f);
		ss.setFont(1, "fonts-japanese-mincho.ttf", 2.0f);
		ss.setFont(2, "LiberationSans-BoldItalic.ttf", 2.0f);
		ss.setFont(3, "fonts-japanese-mincho.ttf", 1.0f);
		ss.setFont(4, "LiberationSans-Regular.ttf", 1.0f);
		ss.setFont(5, "fonts-japanese-gothic.ttf", 1.0f);
		ss.setFont(6, "fonts-japanese-gothic.ttf", 0.5f);
		ss.setFontStyle(0, 1, ss.HALIGN_RIGHT, ss.VALIGN_BOTTOM);
		ss.setFontStyle(1, 0, ss.HALIGN_CENTER, ss.VALIGN_CENTER);
		ss.setFontStyle(2, 1, ss.HALIGN_LEFT, ss.VALIGN_TOP);
		ss.setFontStyle(3, 0, ss.HALIGN_CENTER, ss.VALIGN_CENTER);
		ss.setFontStyle(4, 1, ss.HALIGN_RIGHT, ss.VALIGN_CENTER);
		ss.setFontStyle(5, 0, ss.HALIGN_LEFT, ss.VALIGN_BOTTOM);
		ss.setFontStyle(6, 0, ss.HALIGN_LEFT, ss.VALIGN_TOP);
		ss.setImage(0, "godzilla.png", 4.0f, 4.0f);
		ss.setImage(1, "parrot.jpg", 6.0f, 4.0f);
		ss.setImage(2, "bunny.jpg", 20.0f, 20.0f);
		ss.setImage(3, "dogbones.png", 20.0f, 20.0f);

		ss.newFrame(10000, 4, 2, 4, 2, GameLib::ComposeColor(GameLib::FORESTGREEN, GameLib::AZURE, 3, 1, 2, 0), 0);
		ss.frameHeader(1, "SPAAACE Intruders");
		ss.frameImage(2, { -1.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.2f }, { 0.0f, 0.0f });
		ss.frameLine(
			3,
			"by Dain Harmon, cs680 studios a division of UAF Publishing");
		ss.newFrame(5000, GameLib::BLACK, 3, 4, 2, GameLib::WHITE);
		ss.frameImage(3, { -1.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 1.2f }, { 0.0f, 0.0f });
		ss.frameLine(3, "Powered by the Amazing GameLib Engine");
		ss.newFrame(20000, GameLib::BLACK, 3, GameLib::RED, 2, GameLib::YELLOW);
		ss.frameHeader(0, "SPAAACE Intruders");
		ss.frameImage(0, { -6.0f, 6.0f }, { 6.0f, -1.0f }, { 10.0f, 0.2f }, { -117.0f, 3600.0f });
		ss.frameImageOps({ 0.2f, 0.8f }, { -0.2f, 0.5f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.0f });
		ss.frameLine(
			3,
			"Episode VI.3:\nThe Alien Hordes attack our hero in deep space!\nDefeat them and clear the path to Eurf!");
	}
	ss.play();
}


void Game::showWonEnding() {}


void Game::showLostEnding() {
	GameLib::StoryScreen ss;
	ss.setBlipSound(SOUND_BLIP);
	ss.setFont(0, "URWClassico-Bold.ttf", 2.0f);
	ss.setFont(1, "URWClassico-Bold.ttf", 1.0f);
	ss.setFontStyle(0, 1, ss.HALIGN_CENTER, ss.VALIGN_BOTTOM);
	ss.setFontStyle(1, 0, ss.HALIGN_CENTER, ss.VALIGN_CENTER);
	ss.newFrame(1000, 0, 0, 0, 0, GameLib::BLACK);
	ss.newFrame(5000, GameLib::BLACK, 3, GameLib::RED, 2, GameLib::YELLOW);
	ss.frameHeader(0, "The End");
	ss.frameLine(1, "Oh! This game must not be finished!");
	ss.newFrame(0, 0, 0, 0, 0, GameLib::BLACK);
	ss.play();
}


void Game::startTiming() {
	t0 = stopwatch.stop_sf();
	lag = 0.0f;
}


void Game::updateTiming() {
	t1 = stopwatch.stop_sf();
	dt = t1 - t0;
	t0 = t1;
	GameLib::Context::deltaTime = dt;
	GameLib::Context::currentTime_s = t1;
	GameLib::Context::currentTime_ms = t1 * 1000;
	lag += dt;
}


void Game::shake() {
	if (nextShakeTime > endShakeTime)
		return;
	nextShakeTime += shakeDt;
	using GameLib::random;
	glm::ivec2 screenShake{ random.between(-shakeAmount, shakeAmount), random.between(-shakeAmount, shakeAmount) };
	graphics.setOffset(screenShake);
}


void Game::shake(int amount, float timeLength, float dt) {
	if (amount == 0) {
		graphics.setOffset({ 0, 0 });
		return;
	}
	shakeAmount = amount;
	endShakeTime = t1 + timeLength;
	nextShakeTime = t1 + dt;
	shakeDt = dt;
	shake();
}


bool Game::playGame() {
	stopwatch.start();
	startTiming();
	world.start(t0);
	graphics.setCenter(graphics.origin());
	bool gameWon = false;
	bool gameOver = false;
	while (!context.quitRequested && !gameOver) {
		updateTiming();

		context.getEvents();
		input.handle();
		_debugKeys();

		context.clearScreen(backColor);
		world.drawTiles(graphics);
		while (lag >= Game::MS_PER_UPDATE) {
			updateWorld();
			lag -= Game::MS_PER_UPDATE;
		}

		shake();
		updateCamera();
		drawWorld();
		drawHUD();

		context.swapBuffers();
		frames++;
		std::this_thread::yield();
	}

	return gameWon;
}


void Game::updateCamera() {
	glm::ivec2 xy = world.dynamicActors[0]->pixelCenter(graphics);
	glm::ivec2 center = graphics.center();
	center.x = GameLib::clamp(center.x, xy.x - 100, xy.x + 100);
	center.y = GameLib::clamp(center.y, xy.y - 100, xy.y + 100);
	center.y = std::min(graphics.getCenterY(), center.y);
	graphics.setCenter(center);
}


void Game::updateWorld() {
	world.update(Game::MS_PER_UPDATE);
	world.physics(Game::MS_PER_UPDATE);
}


void Game::drawWorld() {
	world.draw(graphics);
}


void Game::drawHUD() {
	minchofont.draw(0, 0, "Hello, world!", GameLib::Red, GameLib::Font::SHADOWED);
	gothicfont.draw(
		(int)graphics.getWidth(),
		0,
		"Hello, world!",
		GameLib::Blue,
		GameLib::Font::HALIGN_RIGHT | GameLib::Font::SHADOWED);

	int x = (int)graphics.getCenterX();
	int y = (int)graphics.getCenterY() >> 1;
	float s = GameLib::wave(t1, 1.0f);
	SDL_Color c = GameLib::MakeColorHI(7, 4, s, false);
	minchofont.draw(
		x,
		y,
		"Collisions",
		c,
		GameLib::Font::SHADOWED | GameLib::Font::HALIGN_CENTER | GameLib::Font::VALIGN_CENTER | GameLib::Font::BOLD |
			GameLib::Font::ITALIC);

	minchofont.draw(
		0,
		(int)graphics.getHeight() - 2,
		"HP: 56",
		GameLib::Gold,
		GameLib::Font::VALIGN_BOTTOM | GameLib::Font::SHADOWED);

	char fpsstr[64] = { 0 };
	snprintf(fpsstr, 64, "%3.2f", 1.0f / dt);
	minchofont.draw(
		(int)graphics.getWidth(),
		(int)graphics.getHeight() - 2,
		fpsstr,
		GameLib::Gold,
		GameLib::Font::HALIGN_RIGHT | GameLib::Font::VALIGN_BOTTOM | GameLib::Font::SHADOWED);
}


void Game::_debugKeys() {
	if (context.keyboard.checkClear(SDL_SCANCODE_F5)) {
		if (!world.load(worldPath)) {
			HFLOGWARN("world.txt not found");
		}
	}

	if (shakeCommand.checkClear()) {
		shake(4, 5, 50 * MS_PER_UPDATE);
	}
}
