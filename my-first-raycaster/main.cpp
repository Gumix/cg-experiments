//
// https://github.com/Gumix/my-first-raycaster
//

#include <vector>
#include <iostream>
#include <SDL.h>

using std::rand;

// Linear interpolation of two values
double Mix(double start, double end, double t)
{
	return start + (end - start) * t;
}

// Map a value from input range to output range
double Map(double x, double in_min, double in_max,
		   double out_min, double out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class Angle
{
	double rad;

	Angle(double deg): rad(deg * M_PI / 180.0)
	{
	}

public:
	Angle() = default;

	Angle operator - (double deg) const
	{
		Angle res;
		res.rad = rad - Angle(deg);
		return res;
	}

	Angle operator - (const Angle a) const
	{
		Angle res;
		res.rad = rad - a;
		return res;
	}

	Angle & operator += (double deg)
	{
		rad += Angle(deg);
		return *this;
	}

	operator double () const
	{
		return rad;
	}
};

double Sin(Angle a)
{
	return sin(a);
}

double Cos(Angle a)
{
	return cos(a);
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
	Vector2(Angle a)
	{
		x = Cos(a);
		y = Sin(a);
		Normalize();
	}

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

	void SetDrawColor(const Color &c) const
	{
		if (SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 0xff))
			Error("SDL_SetRenderDrawColor failed");
	};

	void Error(const char *msg) const
	{
		std::cerr << "Error: " << msg << ": " << SDL_GetError() << std::endl;
		exit(1);
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

	void Clear() const
	{
		SetDrawColor(Color::Black());

		if (SDL_RenderClear(renderer))
			Error("SDL_RenderClear failed");
	};

	void Update() const
	{
		SDL_RenderPresent(renderer);
	}

	void Pixel(int x, int y, const Color &c = Color::White()) const
	{
		SetDrawColor(c);

		if (SDL_RenderDrawPoint(renderer, x, y))
			Error("SDL_RenderDrawPoint failed");
	};

	void Line(int x1, int y1, int x2, int y2, const Color &c = Color::White()) const
	{
		SetDrawColor(c);

		if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2))
			Error("SDL_RenderDrawLine failed");
	}

	void Rect(int x, int y, int w, int h, const Color &c = Color::White()) const
	{
		SetDrawColor(c);

		SDL_Rect rect = { x, y, w, h };

		if (SDL_RenderDrawRect(renderer, &rect))
			Error("SDL_RenderDrawLine failed");
	}

	void RectFill(int x, int y, int w, int h, const Color &c = Color::White()) const
	{
		SetDrawColor(c);

		SDL_Rect rect = { x, y, w, h };

		if (SDL_RenderFillRect(renderer, &rect))
			Error("SDL_RenderDrawLine failed");
	}
};

static SDL_Screen Screen;

class Wall
{
public:
	int x1, y1, x2, y2;

	Wall(int x1, int y1, int x2, int y2)
		: x1(x1), y1(y1), x2(x2), y2(y2)
	{
	};

	void Draw(double x_offset, double y_offset, double scale,
			  const Color &c = Color::White()) const
	{
		int x1s = round(x1 * scale + x_offset);
		int y1s = round(y1 * scale + y_offset);
		int x2s = round(x2 * scale + x_offset);
		int y2s = round(y2 * scale + y_offset);

		Screen.Line(x1s, y1s, x2s, y2s, c);
	};
};

class Ray
{
	double x, y;
	Angle angle;

public:
	Ray(double x, double y, Angle a): x(x), y(y), angle(a)
	{
	};

	Angle GetAngle() const
	{
		return angle;
	}

	void Rotate(double da)
	{
		angle += da;
	}

	void MoveTo(double new_x, double new_y)
	{
		x = new_x;
		y = new_y;
	}

	bool Intersect(const Wall &wall, double &tw, double &tr) const
	{
		Vector2 dir(angle);

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

struct RayHit
{
	double dist, wall_x, wall_y;
};

class Player
{
	double x, y;
	Angle heading;
	std::vector<Ray> rays;
	static constexpr int num_rays = 320;
	static constexpr double view_angle = 60.0;

public:
	Player() = default;

	Player(double x, double y): x(x), y(y)
	{
		Angle a = heading - view_angle / 2.0;
		for (int i = 0; i < num_rays; i++)
		{
			rays.push_back(Ray(x, y, a));
			a += view_angle / num_rays;
		}
	};

	double GetX() const { return x; }
	double GetY() const { return y; }

	bool CanMove(double dd, int map_width, int map_height) const
	{
		Vector2 dir(heading);
		int new_x = round(x + dir.x * dd);
		int new_y = round(y + dir.y * dd);

		if (new_x < 1 || new_y < 1)
			return false;

		if (new_x >= map_width - 1 || new_y >= map_height - 1)
			return false;

		return true;
	}

	void Rotate(double da)
	{
		heading += da;
		for (size_t i = 0; i < rays.size(); i++)
			rays[i].Rotate(da);
	}

	void Move(double dd)
	{
		Vector2 dir(heading);
		x += dir.x * dd;
		y += dir.y * dd;

		for (size_t i = 0; i < rays.size(); i++)
			rays[i].MoveTo(x, y);
	}

	std::vector<RayHit> CalcRayHits(const std::vector<Wall> &walls) const
	{
		std::vector<RayHit> res;

		for (size_t i = 0; i < rays.size(); i++)
		{
			int j_hit;
			double tw_hit, tr_hit = std::numeric_limits<double>::max();

			for (size_t j = 0; j < walls.size(); j++)
			{
				double tw, tr;
				if (rays[i].Intersect(walls[j], tw, tr))
					if (tr < tr_hit)
					{
						j_hit = j;
						tr_hit = tr;
						tw_hit = tw;
					}
			}

			if (tr_hit != std::numeric_limits<double>::max())
			{
				const Wall w = walls[j_hit];
				res.push_back({
					.dist = tr_hit * Cos(rays[i].GetAngle() - heading),
					.wall_x = Mix(w.x1, w.x2, tw_hit),
					.wall_y = Mix(w.y1, w.y2, tw_hit)
				});
			}
		}

		return res;
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

	void Draw() const
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

	void Draw(double plr_x, double plr_y,
			  const std::vector<Wall> &walls,
			  const std::vector<RayHit> &ray_hits) const
	{
		for (size_t i = 0; i < ray_hits.size(); i++)
		{
			int x1 = round(plr_x * scale + x);
			int y1 = round(plr_y * scale + y);
			int x2 = round(ray_hits[i].wall_x * scale + x);
			int y2 = round(ray_hits[i].wall_y * scale + y);
			Screen.Line(x1, y1, x2, y2, Color::Gray(33));
		}

		for (size_t i = 4; i < walls.size(); i++)
			walls[i].Draw(x, y, scale);

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

	void Draw(const std::vector<RayHit> &ray_hits, int map_width) const
	{
		for (size_t i = 0; i < ray_hits.size(); i++)
		{
			int w = width / ray_hits.size();
			int h = Map(ray_hits[i].dist, 0, map_width, height, 0);
			double d2 = ray_hits[i].dist * ray_hits[i].dist;
			uint8_t b = Map(d2, 0, map_width * map_width, 100, 0);
			Color c = Color::Gray(b);
			Screen.RectFill(x + i * w, y + (height - h) / 2, w, h, c);
		}

		View::Draw();
	};
};

class Scene
{
	static const int map_width = 320;
	static const int map_height = 240;

	static const int num_walls = 6 + 4;
	std::vector<Wall> walls;

	Player neo;
	View2D top;
	View3D scr;

	std::vector<RayHit> ray_hits;

public:
	Scene()
	{
		InitViews();
		InitWalls();
		neo = Player(map_width / 2, map_height / 2);
		ray_hits = neo.CalcRayHits(walls);
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

		walls.push_back(Wall(0, 0, 0, h));
		walls.push_back(Wall(0, 0, w, 0));
		walls.push_back(Wall(w, 0, w, h));
		walls.push_back(Wall(0, h, w, h));

		for (int i = 4; i < num_walls; i++)
			walls.push_back(Wall(rand() % w, rand() % h,
							rand() % w, rand() % h));
	};

	void Draw() const
	{
		top.Draw(neo.GetX(), neo.GetY(), walls, ray_hits);
		scr.Draw(ray_hits, map_width);
	};

	void Move(double da, double dd)
	{
		if (da)
			neo.Rotate(da);

		if (dd && neo.CanMove(dd, map_width, map_height))
			neo.Move(dd);

		if (da || dd)
			ray_hits = neo.CalcRayHits(walls);
	}
};

void KeyDown(SDL_Keycode key, double &da, double &dd)
{
	switch (key)
	{
		case SDLK_LEFT:
			da = -0.5;
			break;
		case SDLK_RIGHT:
			da = 0.5;
			break;
		case SDLK_UP:
			dd = 0.5;
			break;
		case SDLK_DOWN:
			dd = -0.5;
			break;
	}
}

void KeyUp(SDL_Keycode key, double &da, double &dd)
{
	switch (key)
	{
		case SDLK_LEFT:
			if (da < 0.0)
				da = 0.0;
			break;
		case SDLK_RIGHT:
			if (da > 0.0)
				da = 0.0;
			break;
		case SDLK_UP:
			if (dd > 0)
				dd = 0.0;
			break;
		case SDLK_DOWN:
			if (dd < 0)
				dd = 0.0;
			break;
	}
}

int main()
{
	Scene Scene;
	double da = 0.0, dd = 0.0;
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
					KeyDown(event.key.keysym.sym, da, dd);
					break;
				case SDL_KEYUP:
					KeyUp(event.key.keysym.sym, da, dd);
					break;
				case SDL_QUIT:
					stop = true;
					break;
			}
		}

		Scene.Move(da, dd);
		Screen.Clear();
		Scene.Draw();
		Screen.Update();
		SDL_Delay(10);
	}

	return 0;
}
