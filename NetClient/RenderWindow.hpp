#pragma once
#include <SDL.h>
#include <queue>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "server.h"
struct RenderWindow {

	RenderWindow(std::string window_title, size_t width, size_t height) 
		: window(nullptr), renderer(nullptr)
	{
		window = SDL_CreateWindow(window_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

		if (!window) {
			std::cerr << "Window init failed: " << SDL_GetError() << '\n';
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	}

	SDL_Texture* loadTexture(const char* filePath) {
		SDL_Texture * texture = nullptr;
		
		texture = IMG_LoadTexture(renderer, filePath);

		if (!texture)
			std::cerr << "Texture loading failed: " << SDL_GetError() << '\n';
		return texture;
	}

	void clear() {
		SDL_RenderClear(renderer);
	}

	

	void render(SDL_Texture* tex, SDL_Rect * Src = nullptr, SDL_Rect * Dst = nullptr) {
		SDL_RenderCopy(renderer, tex, Src, Dst);
	}
	void display() {
		SDL_RenderPresent(renderer);
	}

	void cleanUp() {
		SDL_DestroyWindow(window);
	}
public:
	auto getRenderer() {
		return renderer;
	}

	auto getWindow() {
		return window;
	}

private:
	SDL_Window * window;
	SDL_Renderer * renderer;
};

struct ClickBox {

public:
	SDL_Texture* clickable;

	
	
	typedef std::function<void(void)> func;
	std::vector<func> functions;

	SDL_Renderer* render;
	int x{ 0 };
	int y{ 0 };

	int h{ 0 };
	int w{ 0 };

	float curXScale{ 0 };
	float curYScale{ 0 };

	ClickBox() {};

	ClickBox(SDL_Texture * click, SDL_Renderer * _render) : clickable(click), render(_render) {
	
	}

	ClickBox(SDL_Texture* click, SDL_Renderer* _render, int _x, int _y, int _w, int _h) : clickable(click), render(_render), x(_x), y(_y), w(_w), h(_h) {

	}

	bool withinBounds(int _x, int _y) {
		float normalX = x * curXScale;
		float normalY = y * curYScale;
		float normalWSize = w * curXScale;
		float normalHSize = h * curYScale;
		//std::cout << "Normal x : " << normalX << '\n';
		//std::cout << "Coords x : " << _x << '\n';
		return (_x >= normalX && _x <= normalX + normalWSize) && (_y >= normalY && _y <= normalY + normalHSize);
	}

	void Render(RenderWindow & window, float xScale = 1.0f, float yScale = 1.0f, bool focus = false, bool clicked = false) {
		isInFocus = focus;
		
		curXScale = xScale;
		curYScale = yScale;
		
		posContainer.x = x;
		posContainer.y = y;
		posContainer.h = h;
		posContainer.w = w;
		SDL_RenderCopy(render, clickable, nullptr, &posContainer);
		float xInv = 1 / xScale;
		float yInv = 1 / yScale;

		SDL_RenderSetScale(render, xScale, yScale);
		window.render(clickable, nullptr, &posContainer);
		SDL_RenderSetScale(render, xInv, yInv);

		if (isClickable && isInFocus && clicked) {
			for (auto& func : functions) {
				func();
			}
		}

		if (!text.empty()) {
			renderText();
		}

		
	}



	
	void registerClickable(func f) {

		functions.push_back(f);

		isClickable = true;
	}

	void deRegisterClickable() {
		isClickable = false;
	}

	void setText(std::string txt) {
		text = txt;
	}

	void resetText() { text.clear(); }

	

	void setCoords(int _x, int _y) {
		x = _x;
		y = _y;
	}

	void setSize(int _h, int _w) {
		h = _h;
		w = _w;
	}

	void setFont(TTF_Font* f) { font = f; }
	void setColor(SDL_Color& c) { color = c; }

	SDL_Rect posContainer;

	TTF_Font* font;// = TTF_OpenFont("arial.ttf", 55);
	SDL_Color color = { 255, 255, 255 };

	std::string text;

	bool isClickable = false;
	bool isInFocus = false;


	void renderText() {
		
		font = TTF_OpenFont("arial.ttf", 55);

		SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), color);

		

		auto texture = SDL_CreateTextureFromSurface(render, surf);

		//SDL_Rect textPlace = {this->x * curXScale, this->y * curYScale, this->w* curXScale, this->h * curYScale};
		int textW = surf->w;
		int textH = surf->h;

		SDL_Rect textPlace = { this->x * curXScale + 40 , this->y * curYScale + 25 , textW, textH };
		if (!isInFocus) {
			SDL_RenderSetScale(render, 1.0f, 1.0f);
		}
		else {
			SDL_RenderSetScale(render, 1.01f, 1.01f);
		}
		SDL_RenderCopy(render, texture, nullptr, &textPlace);
		if (isInFocus) {
			SDL_RenderSetScale(render, 1.0f, 1.0f);
		}
		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surf);
		TTF_CloseFont(font);
	}
};

struct TextBox : public ClickBox {

	SDL_Texture* clickable;
	SDL_Renderer* render;

	TextBox(SDL_Texture* click, SDL_Renderer* _render) : clickable(click), render(_render) {

	}

	TextBox() {};
	
	std::deque<std::string> qText;

	void setText(std::string text) {
		qText.push_back(text);
	}

	void removeText() {
		qText.pop_front();
	}

	bool contains(std::string text) {
		for (auto& word : qText) {
			if (text == word)
				return true;
		}
		return false;
	}

	void Render(RenderWindow& window, float xScale = 1.0f, float yScale = 1.0f, bool focus = false, bool clicked = false) {
		isInFocus = focus;

		curXScale = xScale;
		curYScale = yScale;

		posContainer.x = x;
		posContainer.y = y;
		posContainer.h = h;
		posContainer.w = w;
		SDL_RenderCopy(render, clickable, nullptr, &posContainer);
		float xInv = 1 / xScale;
		float yInv = 1 / yScale;

		SDL_RenderSetScale(render, xScale, yScale);
		window.render(clickable, nullptr, &posContainer);
		SDL_RenderSetScale(render, xInv, yInv);

		if (isClickable && isInFocus && clicked) {
			for (auto& func : functions) {
				func();
			}
		}

		if (!qText.empty()) {
			renderText();
		}


	}

	void renderText() {

		int yDist = 45;
		int i = 0;
		for (auto& line : qText) {
			font = TTF_OpenFont("arial.ttf", 55);
			SDL_Surface* surf = TTF_RenderText_Solid(font, line.c_str(), color);

			auto texture = SDL_CreateTextureFromSurface(render, surf);

			
			int textW = surf->w;
			int textH = surf->h;

			SDL_Rect textPlace = { this->x * curXScale + 80 , this->y * curYScale + 65 + (yDist * i) , textW, textH };
			
			SDL_RenderSetScale(render, 1.0f, 1.0f);
			
			
			SDL_RenderCopy(render, texture, nullptr, &textPlace);
			
			SDL_DestroyTexture(texture);
			SDL_FreeSurface(surf);
			TTF_CloseFont(font);
			i++;
		}
	}

};

struct Draggable : public ClickBox {

	SDL_Texture* draggable;

	Draggable() {};

	bool isBeingDragged = false;
	
	int chessX, chessY;

	Draggable(SDL_Texture* _d, SDL_Renderer* _render, int _x, int _y, int _w, int _h) : draggable(_d), ClickBox(_d, _render, _x, _y, _h, _w) {
		//SDL_QueryTexture(draggable, nullptr, nullptr, &width, &height);
	};

	~Draggable() {
		//SDL_DestroyTexture(draggable);
	}

	void move() {
		
		int _x, _y; //mouse coords
		uint32_t mouseState = SDL_GetMouseState(&_x, &_y);
		x = _x - (this->w * .5);
		y = _y - (this->h * .5);
	}

	void move(int _x, int _y ) {
		x = _x;
		y = _y;
	}

	bool operator==(Draggable& rhs) {
		return &rhs == this;
	}

};