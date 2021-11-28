#pragma once
#include <server.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "RenderWindow.hpp"

#define WINDOW_HEIGHT 1280
#define WINDOW_WIDTH 1280
struct UIManager {
public:
	enum class sceneType : uint32_t {
		lobby,
		game
	};

	enum class boardType : uint32_t {
		black,
		white
	};

	float tileDistanceInPixels = WINDOW_WIDTH / 8;	//in pixels, the width of each tile
	float tileDistanceinPixelsHeight = WINDOW_HEIGHT / 8; //in pixels, the height of each tile

	//offsets used to adjust piece into center of tile
	float leftAdjust = tileDistanceInPixels / 8;
	float heightAdjust = tileDistanceinPixelsHeight / 8;

	std::vector<Draggable> chessPieces; 

	std::list<uint32_t> online;

	sceneType scene = sceneType::lobby;
	boardType boardT = boardType::black;
	bool isWhitesTurn = true;

	const std::array<std::string, 5> menuLabels = { "Start Game", "Invite Friend", "See Stats", "Log Out", "Exit" };

	std::string title = "Chess!";

	TextBox textBox;

	//a set of menu boxes
	std::vector<ClickBox> clickBoxes;


	const int boxH = 2300;
	const int boxW = 1200;

	UIManager() {
		
	};

	void sdlInit() noexcept {
		if (SDL_Init(SDL_INIT_VIDEO) > 0) {
			std::cout << "SDL init failed: " << SDL_GetError() << '\n';
		}

		if (TTF_Init() < 0) {
			std::cout << "TTF init failed: " << SDL_GetError() << '\n';
		}

		if (!(IMG_Init(IMG_INIT_PNG)))
			std::cout << "IMG_init failed: " << SDL_GetError() << '\n';

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	}

	void createAndPushArbEvent() {
		SDL_Event ev;
		ev.type = SDL_USEREVENT;
		ev.user.code = 2;
		ev.user.data1 = nullptr;
		ev.user.data2 = nullptr;
		SDL_PushEvent(&ev);	//creates an event so that it triggers the animation
	}

	virtual void requestGame() {}
	virtual void sendUpdatedPiecePos(std::pair<int, int> from, std::pair<int, int> to) {}

	//uses chess coords to decide if move is valid, different from bounds checking
	virtual bool validMove(std::pair<int, int> from, std::pair<int, int> to) { return false; }

	void changeAndRequestGame() {
		requestGame();
		
	}

	void changeLobbyToGame() {
		scene = sceneType::game;
	}
    
	/*
		Sets the board from white's vs black's perspective
	*/
	void changeBoardPerspective(boardType bT) {
		boardT = bT;
	}

	void chessPiecesInit(SDL_Renderer* render, RenderWindow& window, int x_size = 120, int y_size = 120) {
		

		std::string rootPieceDir = "./images/";//"C:/Users/conno/Desktop/chess_pieces_pngs/";

		std::array pieceOrder = {"rook", "knight", "bishop", "queen", "king", "bishop", "knight", "rook"};
		std::array pieceOrderBlack = { "rook", "knight", "bishop", "king", "queen", "bishop", "knight", "rook" };
		SDL_Texture* blackPawn = window.loadTexture((rootPieceDir + "black_pawn.png").c_str());
		SDL_Texture* whitePawn = window.loadTexture((rootPieceDir + "white_pawn.png").c_str());
	
		if (boardT == boardType::white) {
			
			//blackpiece init
			int tileNum = 0;
			for (auto& pieceName : pieceOrder) {
				SDL_Texture* chessPieceTexture;
				chessPieceTexture = window.loadTexture((rootPieceDir + "black_" + pieceName + ".png").c_str());
				int absoluteLeftPos = leftAdjust + (tileDistanceInPixels * tileNum);
				
				Draggable chessPiece(chessPieceTexture, render, absoluteLeftPos , heightAdjust, x_size, y_size);	//whites perspective
				Draggable blackPawnPiece(blackPawn, render, absoluteLeftPos, heightAdjust + (tileDistanceinPixelsHeight), x_size, y_size);

				chessPiece.chessX = heightAdjust / tileDistanceinPixelsHeight;
				chessPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				blackPawnPiece.chessX = (heightAdjust + (tileDistanceinPixelsHeight)) / tileDistanceinPixelsHeight;
				blackPawnPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				chessPieces.push_back(chessPiece);
				chessPieces.push_back(blackPawnPiece);

				
				tileNum++;
			}

			//white init
			tileNum = 0;
			for (auto& pieceName : pieceOrder) {
				SDL_Texture* chessPieceTexture;
				chessPieceTexture = window.loadTexture((rootPieceDir + "white_" + pieceName + ".png").c_str());
				int absoluteLeftPos = leftAdjust + (tileDistanceInPixels * tileNum);
				int absoluteHeightPos = heightAdjust + (tileDistanceinPixelsHeight * 7);

				Draggable chessPiece(chessPieceTexture, render, absoluteLeftPos, absoluteHeightPos, x_size, y_size);	//whites perspective
				Draggable whitePawnPiece(whitePawn, render, absoluteLeftPos, heightAdjust + (tileDistanceinPixelsHeight * 6), x_size, y_size);

				chessPiece.chessX = absoluteHeightPos / tileDistanceinPixelsHeight;
				chessPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				whitePawnPiece.chessX = (heightAdjust + (tileDistanceinPixelsHeight * 6)) / tileDistanceinPixelsHeight;
				whitePawnPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				chessPieces.push_back(chessPiece);
				chessPieces.push_back(whitePawnPiece);
				tileNum++;
			}	
			
		}
		else {
			
			int tileNum = 0;
			for (auto& pieceName : pieceOrderBlack) {
				SDL_Texture* chessPieceTexture;
				chessPieceTexture = window.loadTexture((rootPieceDir + "white_" + pieceName + ".png").c_str());
				int absoluteLeftPos = leftAdjust + (tileDistanceInPixels * tileNum);

				Draggable chessPiece(chessPieceTexture, render, absoluteLeftPos, heightAdjust, x_size, y_size);
				Draggable whitePawnPiece(whitePawn, render, absoluteLeftPos, heightAdjust + (tileDistanceinPixelsHeight), x_size, y_size);

				chessPiece.chessX = heightAdjust / tileDistanceinPixelsHeight;
				chessPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				whitePawnPiece.chessX = (heightAdjust + (tileDistanceinPixelsHeight)) / tileDistanceinPixelsHeight;
				whitePawnPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				chessPieces.push_back(chessPiece);
				chessPieces.push_back(whitePawnPiece);
				tileNum++;
				
			}

			//white init
			tileNum = 0;
			for (auto& pieceName : pieceOrderBlack) {
				SDL_Texture* chessPieceTexture;
				chessPieceTexture = window.loadTexture((rootPieceDir + "black_" + pieceName + ".png").c_str());
				int absoluteLeftPos = leftAdjust + (tileDistanceInPixels * tileNum);
				int absoluteHeightPos = heightAdjust + (tileDistanceinPixelsHeight * 7);

				Draggable chessPiece(chessPieceTexture, render, absoluteLeftPos, absoluteHeightPos, x_size, y_size);
				Draggable blackPawnPiece(blackPawn, render, absoluteLeftPos, heightAdjust + (tileDistanceinPixelsHeight * 6), x_size, y_size);

				chessPiece.chessX = absoluteHeightPos / tileDistanceinPixelsHeight;
				chessPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				blackPawnPiece.chessX = (heightAdjust + (tileDistanceinPixelsHeight * 6)) / tileDistanceinPixelsHeight;
				blackPawnPiece.chessY = absoluteLeftPos / tileDistanceInPixels;

				chessPieces.push_back(chessPiece);
				chessPieces.push_back(blackPawnPiece);
				tileNum++;
				
			}
		}
		
	}

	void menuBoxInit(SDL_Renderer* render, RenderWindow &window) {

		SDL_Texture* menuSlot = window.loadTexture("./images/Steren_Glossy_Rectangle_Black.png");


		for (int i = 0; i < 5; i++) {
			ClickBox box(menuSlot, render);
			box.setCoords(2600, 5000 + 2400 * i);
			box.setSize(boxH, boxW);
			box.setText(menuLabels[i]);
			box.registerClickable([this]() {
				changeAndRequestGame();
			});
			clickBoxes.push_back(box);

		}
	}

	void textBoxInit(SDL_Renderer* render, RenderWindow &window) {

		SDL_Texture* chatBox = window.loadTexture("./images/chat_box.png");

		TextBox newTextBox(chatBox, render);

		textBox = newTextBox;

		textBox.setCoords(0, 600);
		textBox.setSize(600, 600);
	}

	void onlineUpdate() {

		if (online.size() != 0) {
			for (auto& incId : online) {
				std::stringstream ss;
				std::string ret;

				ss << incId;
				ss >> ret;
				if (!textBox.contains(ret))
					textBox.setText(ret);
			}
		}
	}

	void boxRenderAndHighlight(SDL_Renderer * render, RenderWindow &window, int x, int y, bool hasClicked) {
		for (auto& box : clickBoxes) {
			if (bool inBounds = box.withinBounds(x, y)) {
				box.setSize(boxH * 1.04, boxW * 1.04);
				box.Render(window, .3f, .05f, inBounds, hasClicked); //executes the boxes callback if the user clicked within bounds of box
			}
			else {
				box.setSize(boxH, boxW);
				box.Render(window, .3f, .05f);
			}
		}
	}

	void draggableInit(SDL_Renderer* render, RenderWindow &window, const std::string pathToImage) {
		SDL_Texture* menuSlot = window.loadTexture(pathToImage.c_str());

		int width;
		int height;
		SDL_QueryTexture(menuSlot, nullptr, nullptr, &width, &height);

		width = 50;
		height = 150;
		Draggable _dM(menuSlot, render, 0, 0, height, width);

		//draggableMenu = _dM;

		
	}

	std::pair<int, int> pixelToCoord(Draggable& chessPiece) {

		std::pair<int, int> coord;
		int x, y;

		SDL_GetMouseState(&x, &y);

		coord.first = x / tileDistanceInPixels;
		coord.second = y / tileDistanceinPixelsHeight;

		return coord;
	}

	//takes a piece and updates it's (x, y) coords to the nearest center of tile
	void snapToNearestTile(Draggable& pieceToSnap) {
		int _x, _y; //mouse coords
		uint32_t mouseState = SDL_GetMouseState(&_x, &_y);

		int tileIdxX = _x  - (_x % (int)tileDistanceInPixels); // coordinate of left edge of tile
		int tileIdxY = _y  - (_y % (int)tileDistanceinPixelsHeight); //coordinate of top edge of tile

		pieceToSnap.move(tileIdxX + leftAdjust, tileIdxY + heightAdjust);
	}

	void snapToNearestTilePiece(Draggable& piece) {
		int tileIdxX = piece.x - (piece.x % (int)tileDistanceInPixels);	// coordinate of left edge of tile
		int tileIdxY = piece.y - (piece.y % (int)tileDistanceinPixelsHeight); //coordinate of top edge of tile

		piece.move(tileIdxX + leftAdjust, tileIdxY + heightAdjust);
	}

	void snapFromTo(std::pair<int, int> from, std::pair<int, int> to) {

		for (auto& piece : chessPieces) {
			if (piece.withinBounds(from.first, from.second)) { //finds the piece within the bounds of from, moves it, then snaps it to align within the tile
				piece.move(to.first, to.second);
				snapToNearestTilePiece(piece);
				return;
			}
		}

	}
	/*
		Takes in a piece, gets it's coordinates, and then applies a linear transformation
		in order to properly move the correct piece from the enemies point of view
	*/
	std::pair<int, int> flipPixelPosition(Draggable& piece) {

		std::pair<int, int> postFlip{distanceToCenterX(piece), distanceToCenterY(piece)};
		return postFlip;
	}

	int distanceToCenterY(Draggable& piece) { //returns the Y pixel position from the enemies point of view
		return piece.y + (2 * ((tileDistanceinPixelsHeight * 4) - piece.y));
	}
	int distanceToCenterX(Draggable& piece) { //returns the X pixel position from the enemies point of view
		return piece.x + (2 * ((tileDistanceInPixels * 4) - piece.x));
	}

	int distanceToCenterY(int y) { //returns the Y pixel position from the enemies point of view
		return y + (2 * ((tileDistanceinPixelsHeight * 4) - y));
	}
	int distanceToCenterX(int x) { //returns the X pixel position from the enemies point of view
		return x + (2 * ((tileDistanceInPixels * 4) - x));
	}

	void clearDragState() {
		for (auto& piece : chessPieces) {
			piece.isBeingDragged = false;
		}
	}

	void exitGame(RenderWindow& window, TTF_Font * font) {
		window.cleanUp();
		TTF_CloseFont(font);
		TTF_Quit();
		SDL_Quit();
		exit(1);
	}

	bool checkIfOccupiedAndEvict(std::pair<int, int> to, Draggable * except = nullptr) {
		int idx = 0;
		for (auto& piece : chessPieces) {
			if (piece.chessX == to.second && piece.chessY == to.first && &piece != except) {
				chessPieces.erase(chessPieces.begin() + idx);  //removes a piece after a valid attacking move
			}
			idx++;
		}
		return false;
	}

	void Run() {

		TTF_Font* font = TTF_OpenFont("arial.ttf", 55);

		RenderWindow window(title, WINDOW_WIDTH, WINDOW_HEIGHT);

		SDL_Renderer* render = SDL_GetRenderer(window.getWindow());

		SDL_Texture* background = window.loadTexture("./images/background2.png");

		bool run = true;

		bool hasChessPieceInit = false;

		SDL_Event event;


		sdlInit();

		menuBoxInit(render, window);

		textBoxInit(render, window);

		

		bool hasClicked = false;
		bool mouseDown = false;
		bool isDragging = false;

		Draggable* draggedPiece = nullptr; //a temp variable used to render dragged piece last
		std::pair<int, int> transformedFrom; //temp coord used to translate in the receiving client side
		std::pair<int, int> untransformedFrom;
		while (run) {

			

			while (SDL_WaitEvent(&event)) {
				
				switch (event.type) {
					case SDL_QUIT: {
						exitGame(window, font);
						break;
					}
					case SDL_MOUSEBUTTONDOWN: {
						hasClicked = true;
						mouseDown = true;
						break;
					}
					/*
						clear all drag event handlers and update a piece position if one is being dragged
					*/
					case SDL_MOUSEBUTTONUP: {
						hasClicked = false;
						mouseDown = false;
						isDragging = false;
						clearDragState(); //sets all draggable to false;
						if (draggedPiece) {

							std::pair<int, int> chessCoordTo;
							int x, y;
							SDL_GetMouseState(&x, &y);
							chessCoordTo.first = x / tileDistanceInPixels;
							chessCoordTo.second = y / tileDistanceinPixelsHeight;


							if (validMove(untransformedFrom, chessCoordTo)) {
								snapToNearestTile(*draggedPiece);
								draggedPiece->chessY = chessCoordTo.first; //update chess based coordinates
								draggedPiece->chessX = chessCoordTo.second;
								auto transformedTo = flipPixelPosition(*draggedPiece);

								sendUpdatedPiecePos(transformedFrom, transformedTo); //updates other client with new piece position

								checkIfOccupiedAndEvict(chessCoordTo, draggedPiece);

								isWhitesTurn = !isWhitesTurn;
							}
							else {
								draggedPiece->move(distanceToCenterX(transformedFrom.first), distanceToCenterY(transformedFrom.second)); //takes the transformed view, flips it, and moves the piece to where it was picked up from
							}
						}
						draggedPiece = nullptr;
						break;
					}
					default: {
						
					}
					
				}

				window.clear();

				onlineUpdate();

				int x, y; //mouse coords
				uint32_t mouseState = SDL_GetMouseState(&x, &y);



				switch (scene) {
					case sceneType::lobby: {
						background = window.loadTexture("./images/background2.png");
						window.render(background);
						textBox.Render(window, 1.0f, 1.0f, textBox.withinBounds(x, y));

						boxRenderAndHighlight(render, window, x, y, hasClicked); //renders the menu boxes and highlights the one that the mouse is hovering over
																				 //passing hasClicked tells the box to execute it's list of callback functions

						break;
					}

					case sceneType::game: {

						if (boardT == boardType::white) {  //renders the board from either white's or blacks perspective
							background = window.loadTexture("./images/chess_whiteview_board.png");
						}
						else {
							background = window.loadTexture("./images/chess_blackview_board.png");
						}
						window.render(background);
						if (!hasChessPieceInit) { //should fire once
							chessPiecesInit(render, window);
							hasChessPieceInit = true;
						}
						  //allows a user to move a piece if it's their turn
						for (auto& piece : chessPieces) {
							if ((isWhitesTurn && boardT == boardType::white) || (!isWhitesTurn && boardT == boardType::black)) {
								if (hasClicked && !isDragging) {
									if (piece.withinBounds(x, y)) {
										piece.isBeingDragged = true;
										isDragging = true;
										draggedPiece = &piece;

										transformedFrom = flipPixelPosition(piece);	//return the correct pixel position from the enemies point of view of this piece.

										untransformedFrom.first = piece.x / tileDistanceInPixels; //used as chess coordinates, different than pixel transformations
										untransformedFrom.second = piece.y / tileDistanceinPixelsHeight;

										continue;
									}
								}
							}

							piece.Render(window);
						}
						if (draggedPiece && draggedPiece->isBeingDragged && isDragging) { //makes it so that the piece you're dragging is rendered on top of all other pieces
							draggedPiece->move();
							draggedPiece->Render(window);
						}
					
					}
				}

				window.display();
				SDL_DestroyTexture(background);
			}

			

			//std::this_thread::sleep_for(std::chrono::milliseconds(6));
		}

		
	}

	void changeLobby() {
		scene = sceneType::game;
	}

};