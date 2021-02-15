//
// https://github.com/Gumix/my-first-raycaster
//

#include <iostream>
#include <SDL.h>

class SDL_Screen
{
	int width, height;
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

public:
	SDL_Screen()
	{
		if (SDL_Init(SDL_INIT_VIDEO))
			Error("SDL_Init failed");

		SDL_DisplayMode mode;
		if (SDL_GetCurrentDisplayMode(0, &mode))
			Error("SDL_GetCurrentDisplayMode failed");
		width = mode.w;
		height = mode.h;

		if (SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
										&window, &renderer))
			Error("SDL_CreateWindowAndRenderer failed");

		if (SDL_RenderSetLogicalSize(renderer, width, height))
			Error("SDL_RenderSetLogicalSize failed");

		Clear();
	};

	~SDL_Screen()
	{
		if (renderer)
		{
			SDL_DestroyRenderer(renderer);
			renderer = nullptr;
		}

		if (window)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}

		SDL_Quit();
	};

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

	void Clear()
	{
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff))
			Error("SDL_SetRenderDrawColor failed");

		if (SDL_RenderClear(renderer))
			Error("SDL_RenderClear failed");
	};

	void Update()
	{
		SDL_RenderPresent(renderer);
	}

	void Pixel(int x, int y)
	{
		if (SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff))
			Error("SDL_SetRenderDrawColor failed");

		if (SDL_RenderDrawPoint(renderer, x, y))
			Error("SDL_RenderDrawPoint failed");
	};

	void Line(int x1, int y1, int x2, int y2)
	{
		if (SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff))
			Error("SDL_SetRenderDrawColor failed");

		if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2))
			Error("SDL_RenderDrawLine failed");
	}

	void Error(const char *msg)
	{
		std::cerr << "Error: " << msg << ": " << SDL_GetError() << std::endl;
		exit(1);
	};
};

static SDL_Screen Screen;

class Scene
{
	int x = 100;
	int y = 100;

public:
	void Draw()
	{
		Screen.Pixel(x, y);
	};

	void Move(int dx, int dy)
	{
		x += dx;
		y += dy;
	}
};

void KeyDown(SDL_Keycode key, int &dx, int &dy)
{
	switch (key)
	{
		case SDLK_LEFT:
			dx = -1;
			break;
		case SDLK_RIGHT:
			dx = 1;
			break;
		case SDLK_UP:
			dy = -1;
			break;
		case SDLK_DOWN:
			dy = 1;
			break;
	}
}

void KeyUp(SDL_Keycode key, int &dx, int &dy)
{
	switch (key)
	{
		case SDLK_LEFT:
			if (dx < 0)
				dx = 0;
			break;
		case SDLK_RIGHT:
			if (dx > 0)
				dx = 0;
			break;
		case SDLK_UP:
			if (dy < 0)
				dy = 0;
			break;
		case SDLK_DOWN:
			if (dy > 0)
				dy = 0;
			break;
	}
}

int main()
{
	Scene Scene;
	int dx = 0, dy = 0;
	bool stop = false;

	while (!stop)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					KeyDown(event.key.keysym.sym, dx, dy);
					break;
				case SDL_KEYUP:
					KeyUp(event.key.keysym.sym, dx, dy);
					break;
				case SDL_QUIT:
					stop = true;
					break;
			}
		}

		Scene.Move(dx, dy);
		Screen.Clear();
		Scene.Draw();
		Screen.Update();
		SDL_Delay(10);
	}

	return 0;
}
