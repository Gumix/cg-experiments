//
// https://github.com/Gumix/my-first-raycaster
//

#include <iostream>
#include <SDL.h>

using std::rand;

// Linear interpolation of two values
double Mix(double start, double end, double t)
{
	return start + (end - start) * t;
}

class Color
{
public:
	uint8_t r, g, b;

	Color(uint8_t red, uint8_t green, uint8_t blue)
		: r(red), g(green), b(blue)
	{
	}

	static const Color Black()	{ return Color(0x00, 0x00, 0x00); }
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

	void SetDrawColor(const Color &c)
	{
		if (SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 0xff))
			Error("SDL_SetRenderDrawColor failed");
	};

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
		SetDrawColor(Color::Black());

		if (SDL_RenderClear(renderer))
			Error("SDL_RenderClear failed");
	};

	void Update()
	{
		SDL_RenderPresent(renderer);
	}

	void Pixel(int x, int y, const Color &c = Color::White())
	{
		SetDrawColor(c);

		if (SDL_RenderDrawPoint(renderer, x, y))
			Error("SDL_RenderDrawPoint failed");
	};

	void Line(int x1, int y1, int x2, int y2, const Color &c = Color::White())
	{
		SetDrawColor(c);

		if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2))
			Error("SDL_RenderDrawLine failed");
	}

	void Rect(int x, int y, int w, int h, const Color &c = Color::White())
	{
		SetDrawColor(c);

		SDL_Rect rect = { x, y, w, h };

		if (SDL_RenderDrawRect(renderer, &rect))
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

	void Draw(int x_offset, int y_offset, double scale,
			  const Color &c = Color::White()) const
	{
		int x1s = round(x1 * scale) + x_offset;
		int y1s = round(y1 * scale) + y_offset;
		int x2s = round(x2 * scale) + x_offset;
		int y2s = round(y2 * scale) + y_offset;

		Screen.Line(x1s, y1s, x2s, y2s, c);
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

	bool Intersect(const Wall &wall, double &tw, double &tr)
	{
		double nwx = wall.y2 - wall.y1;
		double nwy = wall.x1 - wall.x2;
		double nrx = dir.y;
		double nry = -dir.x;

		double den = nry * nwx - nrx * nwy;

		if (den == 0.0)	// exact checking is ok here
			return false;

		tw = -(nrx * (wall.x1 - x) + nry * (wall.y1 - y)) / den;
		tr = -(nwy * (wall.y1 - y) + nwx * (wall.x1 - x)) / den;

		return tw > 0.0 && tw < 1.0 && tr > 0.0;
	};
};

class Player
{
	int x, y;
	static const int num_rays = 45;
	Ray rays[num_rays];

public:
	Player() = default;

	Player(int x, int y): x(x), y(y)
	{
		for (int i = 0; i < num_rays; i++)
			rays[i] = Ray(x, y, i);
	};

	bool CanMove(int dx, int dy, int map_width, int map_height)
	{
		if (x + dx < 0 || y + dy < 0)
			return false;

		if (x + dx >= map_width || y + dy >= map_height)
			return false;

		return true;
	}

	void Move(int dx, int dy)
	{
		x += dx;
		y += dy;

		for (int i = 0; i < num_rays; i++)
			rays[i].Move(dx, dy);
	}

	void Draw(const Wall walls[], int num_walls)
	{
		for (int i = 0; i < num_rays; i++)
		{
			bool hit = false;
			int j_hit;
			double tw, tr, tw_hit;
			double tr_hit = std::numeric_limits<double>::max();

			for (int j = 0; j < num_walls; j++)
				if (rays[i].Intersect(walls[j], tw, tr))
					if (tr < tr_hit)
					{
						hit = true;
						j_hit = j;
						tr_hit = tr;
						tw_hit = tw;
					}

			if (hit)
			{
				// TODO: Rework this
				const int y_offset = 270;
				const double scale = 1.5;
				const Wall w = walls[j_hit];
				int x1 = round(x * scale);
				int y1 = round(y * scale) + y_offset;
				int x2 = round(Mix(w.x1, w.x2, tw_hit) * scale);
				int y2 = round(Mix(w.y1, w.y2, tw_hit) * scale) + y_offset;
				Screen.Line(x1, y1, x2, y2);
			}
		}
	};
};

class View
{
protected:
	int x, y;
	int width, height;

public:
	View() = default;

	View(int offset, int width, int height)
		: x(offset), width(width), height(height)
	{
		y = (Screen.GetHeight() - height) / 2;
	};

	void Draw()
	{
		Screen.Rect(x, y, width, height, Color(0, 50, 100));
	};
};

class View2D: public View
{
	double scale;

public:
	View2D() = default;

	View2D(int offset, int width, int height, double scale)
		: View(offset, width, height), scale(scale)
	{
	};

	void Draw(const Wall walls[], int num_walls)
	{
		for (int i = 0; i < num_walls; i++)
			walls[i].Draw(x, y, scale, Color::Gray(50));

		View::Draw();
	};
};

class View3D: public View
{
public:
	View3D() = default;

	View3D(int offset, int width, int height)
		: View(offset, width, height)
	{
	};

	void Draw()
	{
		View::Draw();
	};
};

class Scene
{
	static const int map_width = 320;
	static const int map_height = 240;

	static const int num_walls = 6 + 4;
	Wall walls[num_walls];

	Player neo;
	View2D top;
	View3D scr;

public:
	Scene()
	{
		InitViews();
		InitWalls();
		neo = Player(map_width / 2, map_height / 2);
	}

	void InitViews()
	{
		// Allocate 1/3 of the screen width for 2D view, and 2/3 for 3D view
		double w = Screen.GetWidth() / 3.0;
		double scale = w / map_width;
		double h = map_height * scale;

		top = View2D(0, w + 1, h, scale);
		scr = View3D(w, w * 2, h * 2);
	}

	void InitWalls()
	{
		int w = map_width - 1;
		int h = map_height - 1;

		walls[0] = Wall(0, 0, 0, h);
		walls[1] = Wall(0, 0, w, 0);
		walls[2] = Wall(w, 0, w, h);
		walls[3] = Wall(0, h, w, h);

		for (int i = 4; i < num_walls; i++)
			walls[i] = Wall(rand() % w, rand() % h,
							rand() % w, rand() % h);
	};

	void Draw()
	{
		neo.Draw(walls, num_walls);
		top.Draw(walls, num_walls);
		scr.Draw();
	};

	void Move(int dx, int dy)
	{
		if (neo.CanMove(dx, dy, map_width, map_height))
			neo.Move(dx, dy);
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
