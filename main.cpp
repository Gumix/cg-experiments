//
// https://github.com/Gumix/my-first-raycaster
//

#include <iostream>
#include <SDL.h>

using std::rand;

class Color
{
public:
	uint8_t r, g, b;

	Color(uint8_t red, uint8_t green, uint8_t blue)
		: r(red), g(green), b(blue)
	{
	}

	static const Color White()	{ return Color(0xff, 0xff, 0xff); }
	static const Color Red()	{ return Color(0xff, 0x00, 0x00); }
	static const Color Green()	{ return Color(0x00, 0xff, 0x00); }
	static const Color Blue()	{ return Color(0x00, 0x00, 0xff); }
	static const Color Magenta(){ return Color(0xff, 0x00, 0xff); }
	static const Color Acid()	{ return Color(0xc6, 0xff, 0x00); }

	static const Color Gray(uint8_t w)
	{
		double wd = w / 100.0 * 255.0;
		w = std::min(int(wd + 0.5), 255);
		return Color(w, w, w);
	}
};

class Vector2
{
public:
	double x, y;

	Vector2() = default;
	Vector2(double x, double y): x(x), y(y) { }
	double Length() const { return sqrt(x*x + y*y); }
	void Normalize() { *this /= Length(); }
	Vector2 operator - () const { return Vector2(-x, -y); }
	Vector2 operator + (const Vector2 v) const { return Vector2(x + v.x, y + v.y); }
	Vector2 operator - (const Vector2 v) const { return Vector2(x - v.x, y - v.y); }
	double operator * (const Vector2 v) const { return x * v.x + y * v.y; }
	Vector2 operator / (double k) const { return Vector2(x / k, y / k); }
	Vector2 & operator /= (double k) { x /= k; y /= k; return *this; }
};

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

	void Pixel(int x, int y, const Color &c = Color::White())
	{
		if (SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 0xff))
			Error("SDL_SetRenderDrawColor failed");

		if (SDL_RenderDrawPoint(renderer, x, y))
			Error("SDL_RenderDrawPoint failed");
	};

	void Line(int x1, int y1, int x2, int y2, const Color &c = Color::White())
	{
		if (SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 0xff))
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

class Wall
{
public:
	int x1, y1, x2, y2;

	Wall() = default;

	Wall(int x1, int y1, int x2, int y2)
		: x1(x1), y1(y1), x2(x2), y2(y2)
	{
	};

	void Draw(const Color &c = Color::White())
	{
		Screen.Line(x1, y1, x2, y2, c);
	};
};

class Ray
{
	int x, y;
	Vector2 dir;

public:
	Ray() = default;

	Ray(int x, int y, int a): x(x), y(y)
	{
		double angle = a * M_PI / 180.0;
		dir.x = cos(angle);
		dir.y = sin(angle);
		dir.Normalize();
	};

	void Move(int dx, int dy)
	{
		x += dx;
		y += dy;
	}

	bool Intersect(const Wall &wall)
	{
		// TODO
		(void) wall;
		return true;
	};
};

class Bulb
{
	int x, y;
	static const int num_rays = 360;
	Ray rays[num_rays];

public:
	Bulb() = default;

	Bulb(int x, int y): x(x), y(y)
	{
		for (int i = 0; i < num_rays; i++)
			rays[i] = Ray(x, y, i);
	};

	void Move(int dx, int dy)
	{
		x += dx;
		y += dy;

		for (int i = 0; i < num_rays; i++)
			rays[i].Move(dx, dy);
	}

	void Draw(const Wall &wall)
	{
		for (int i = 0; i < num_rays; i++)
		{
			if (rays[i].Intersect(wall))
			{
				double a = i * M_PI / 180.0;
				Screen.Line(x, y, x + 10 * cos(a), y + 10 * sin(a));
			}
		}
	};
};

class Scene
{
	Bulb bulb;
	Wall wall;

public:
	Scene()
	{
		int width = Screen.GetWidth();
		int height = Screen.GetHeight();

		bulb = Bulb(width / 2, height / 2);

		wall = Wall(rand() % width, rand() % height,
					rand() % width, rand() % height);
	}

	void Draw()
	{
		bulb.Draw(wall);
		wall.Draw(Color::Gray(50));
	};

	void Move(int dx, int dy)
	{
		bulb.Move(dx, dy);
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

	std::srand(std::time(nullptr));

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
