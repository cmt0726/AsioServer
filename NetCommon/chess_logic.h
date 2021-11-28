#pragma once

#include <bitset>
#include "net_common.h"
#include <unordered_map>
#include <cmath>
static const struct pieceMap {
	const uint8_t EMPTY = 0;
	const uint8_t PAWN = 1;
	const uint8_t ROOK = 2;
	const uint8_t KNIGHT = 3;
	const uint8_t BISHOP = 4;
	const uint8_t QUEEN = 5;
	const uint8_t KING = 6;
	const uint8_t PIECE_COLOR = 7;
} pm;

//used as the default board setup


static const std::array<std::array<uint8_t, 8>, 8> boardInit = {{
	{pm.ROOK, pm.KNIGHT, pm.BISHOP, pm.QUEEN, pm.KING, pm.BISHOP, pm.KNIGHT, pm.ROOK},
	{pm.PAWN, pm.PAWN, pm.PAWN, pm.PAWN	, pm.PAWN, pm.PAWN, pm.PAWN, pm.PAWN},
	{pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY},
	{pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY},
	{pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY},
	{pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY, pm.EMPTY},
	{pm.PAWN, pm.PAWN, pm.PAWN, pm.PAWN, pm.PAWN, pm.PAWN, pm.PAWN, pm.PAWN},
	{pm.ROOK, pm.KNIGHT, pm.BISHOP, pm.QUEEN, pm.KING, pm.BISHOP, pm.KNIGHT, pm.ROOK}
}};


std::array<bool, 8> pawnMovedWhite{ { false, false, false, false, false, false, false, false } };
std::array<bool, 8> pawnMovedBlack{ { false, false, false, false, false, false, false, false } };

std::array<bool, 8> pawnMovedWhiteLastTurn{ { false, false, false, false, false, false, false, false } };
std::array<bool, 8> pawnMovedBlackLastTurn{ { false, false, false, false, false, false, false, false } };

std::array<bool, 2> rookMovedWhite{ {false, false} };
std::array<bool, 2> rookMovedBlack{ {false, false} };

bool kingMovedWhite = false;
bool kingMovedBlack = false;

	
struct GameTile {
	std::bitset<1> color; //Black or White

	/*
	*	0b0 = nothing
		0b1 = pawn
		0b10 = rook
		0b11 = knight
		0b100 = bishop
		0b101 = queen
		0b110 = king
		0b111 = piece color
	*/
	std::bitset<3> piece;

	bool pieceColor; //true -> black. false -> white

};

struct King {
	std::string name = "King";
	bool color = false;

	King(bool inColor) : color(inColor) {};
	King() {};

	bool move(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board, bool sim = false) {
		return validateMove(row1, col1, row2, col2, Board, sim);
	}
	bool validateMove(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board, bool sim) {

		int absDistRow = std::abs( (int) (row2 - row1) );
		int absDistCol = std::abs( (int) (col2 - col1) );

		if (absDistRow > 1) {
			return false;
		}
		else {
			//handling castling. Castling is a king move but is characterized
			//by a lateral movement of which the king and that sides' rook 
			//meet in a central place having moved a few squares
			//All squares inbetween must be empty and neither the king or that sides
			//rook should move
			bool spaceBetweenOccupied = true;
			//bool spaceBetweenAttacked = false;
			if (color) {
				
				if (kingMovedBlack)
					return false;
				if ((col2 - col1) == 2) { //king side castle
					spaceBetweenOccupied = Board[row1][col1 + 1].piece != 0 && Board[row1][col1 + 2].piece != 0;
				}
				else if ((col2 - col1) == -2) {	//queen side castle
					spaceBetweenOccupied = Board[row1][col1 - 1].piece != 0 && Board[row1][col1 - 2].piece != 0 && Board[row1][col1 - 3].piece != 0;
				}
				if (!spaceBetweenOccupied && !sim)
					kingMovedBlack = true;
			}
			else {
				if (kingMovedWhite)
					return false;
				if ((col2 - col1) == 2) {  //king side castle
					spaceBetweenOccupied = Board[row1][col1 + 1].piece != 0 && Board[row1][col1 + 2].piece != 0;
				}
				else if ((col2 - col1) == -2) {	//queen side castle
					spaceBetweenOccupied = Board[row1][col1 - 1].piece != 0 && Board[row1][col1 - 2].piece != 0 && Board[row1][col1 - 3].piece != 0;
				}
				if (!spaceBetweenOccupied && !sim)
					kingMovedWhite = true;
			}
			return !spaceBetweenOccupied; //returns true if the spaces between where the rook and king is are empty
		}

		//at this point it's assumed that the king is trying to capture a piece
		bool pieceOccupied = Board[row2][col2].piece != 0;
		bool attackedPieceOccupied = Board[row2][col2].pieceColor == !color;

		if (!attackedPieceOccupied && pieceOccupied) return false;
		return true;
	}
};

struct Rook {
	std::string name = "Rook";
	bool color = false;

	Rook(bool inColor) : color(inColor) {};
	Rook() {};

	bool move(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		return validateMove(row1, col1, row2, col2, Board);
	}

	bool validateMove(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		int rowDist = row2 - row1;
		int colDist = col2 - col1;
		bool pieceOccupied = Board[row2][col2].piece != 0;
		bool attackedPieceOccupied = Board[row2][col2].pieceColor == !color;

		if (!attackedPieceOccupied && pieceOccupied) return false;

		if (colDist != 0 && rowDist != 0)
			return false;
		if (rowDist == 0) { //sliding on same row
			if (colDist < 0) { //going left
				for (int i = 1; i < std::abs(colDist); i++) {
					if (Board[row1][col1 - i].piece != 0)
						return false;
				}
			}
			else { //going right
				for (int i = 1; i < std::abs(colDist); i++) {
					if (Board[row1][col1 + i].piece != 0)
						return false;
				}
			}
		}
		else { //sliding on same column
			if (rowDist < 0) { //going up
				for (int i = 1; i < std::abs(colDist); i++) {
					if (Board[row1 - i][col1].piece != 0)
						return false;
				}
			}
			else {	//going down
				for (int i = 1; i < std::abs(colDist); i++) {
					if (Board[row1 + i][col1].piece != 0)
						return false;
				}
			}
		}
		return true;
	}
};

struct Queen {
	std::string name = "Queen";
	bool color = false;

	Queen(bool inColor) : color(inColor) {};
	Queen() {};

	bool move(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		return validateMove(row1, col1, row2, col2, Board);
	}

	bool validateMove(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		bool diagDist = (std::abs((int)(row2 - row1))) == (std::abs((int)(col2 - col1)));
		int rowDist = row2 - row1;
		int colDist = col2 - col1;
		bool pieceOccupied = Board[row2][col2].piece != 0;
		bool attackedPieceOccupied = Board[row2][col2].pieceColor == !color;

		if (!attackedPieceOccupied && pieceOccupied) return false;

		if (diagDist) {	//this is a diagonal move

			bool isPieceInWay = false;
			if (rowDist < 0) { //going up
				if (colDist < 0) { // going left
					for (int i = 1; i < std::abs(rowDist); i++) {
						if (Board[row1 - i][col1 - i].piece != 0)
							return false;
					}
				}
				else { //going right 
					for (int i = 1; i < std::abs(rowDist); i++) {
						if (Board[row1 - i][col1 + i].piece != 0)
							return false;
					}
				}
			}
			else { //going down
				if (colDist < 0) { //going left
					for (int i = 1; i < std::abs(rowDist); i++) {
						if (Board[row1 + i][col1 - i].piece != 0)
							return false;
					}
				}
				else { //going right
					for (int i = 1; i < std::abs(rowDist); i++) {
						if (Board[row1 + i][col1 + i].piece != 0)
							return false;
					}
				}
			}
		}
		else {
			if (colDist != 0 && rowDist != 0)
				return false;
			if (rowDist == 0) { //sliding on same row
				if (colDist < 0) { //going left
					for (int i = 1; i < std::abs(colDist); i++) {
						if (Board[row1][col1 - i].piece != 0)
							return false;
					}
				}
				else { //going right
					for (int i = 1; i < std::abs(colDist); i++) {
						if (Board[row1][col1 + i].piece != 0)
							return false;
					}
				}
			}
			else { //sliding on same column
				if (rowDist < 0) { //going up
					for (int i = 1; i < std::abs(colDist); i++) {
						if (Board[row1 - i][col1].piece != 0)
							return false;
					}
				}
				else {	//going down
					for (int i = 1; i < std::abs(colDist); i++) {
						if (Board[row1 + i][col1].piece != 0)
							return false;
					}
				}
			}
		}
		return true;
	}
};

struct Bishop {
	std::string name = "Bishop";
	bool color = false;

	Bishop(bool inColor) : color(inColor) {};
	Bishop() {};

	bool move(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		return validateMove(row1, col1, row2, col2, Board);

	}

	bool validateMove(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		bool diagDist = (std::abs((int)(row2 - row1))) == (std::abs((int)(col2 - col1)));
		if (!diagDist)
			return false;
		bool pieceOccupied = Board[row2][col2].piece != 0;
		bool attackedPieceOccupied = Board[row2][col2].pieceColor == !color;

		if (!attackedPieceOccupied && pieceOccupied) return false;

		int rowDist = row2 - row1;
		int colDist = col2 - col1;
		bool isPieceInWay = false;
		if (rowDist < 0) { //going up
			if (colDist < 0) { // going left
				for (int i = 1; i < std::abs(rowDist); i++) {
					if (Board[row1 - i][col1 - i].piece != 0)
						return false;
				}
			}
			else { //going right 
				for (int i = 1; i < std::abs(rowDist); i++) {
					if (Board[row1 - i][col1 + i].piece != 0)
						return false;
				}
			}
		}
		else { //going down
			if (colDist < 0) { //going left
				for (int i = 1; i < std::abs(rowDist); i++) {
					if (Board[row1 + i][col1 - i].piece != 0)
						return false;
				}
			}
			else { //going right
				for (int i = 1; i < std::abs(rowDist); i++) {
					if (Board[row1 + i][col1 + i].piece != 0)
						return false;
				}
			}
		}
		return true;
	}
};

struct Knight {
	std::string name = "Knight";
	bool color = false;

	Knight(bool inColor) : color(inColor) {};
	Knight() {};

	bool move(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		return validateMove(row1, col1, row2, col2, Board);
			
	}

	bool validateMove(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board) {
		int heightDist = std::abs((int) row2 - (int) row1);
		
		int colDist = std::abs((int)col2 - (int)col1);
		
		if (heightDist > 2) {
			return false;
		}

		if (heightDist == 1 && colDist > 2) {
			return false;
		}
		else if (heightDist == 2 && colDist > 1) {
			return false;
		}
		bool pieceOccupied = Board[row2][col2].piece != 0;
		bool attackedPieceOccupied = Board[row2][col2].pieceColor == !color;

		if (!attackedPieceOccupied && pieceOccupied) return false;

		return true;

	}
};


struct Pawn{

	std::string name = "Pawn";
	bool color = false;
	bool hasMoved = false;

	Pawn(bool inColor) : color(inColor) {}
	Pawn() {}

	bool move(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board, bool sim = false) {

		return validateMove(row1, col1, row2, col2, Board, sim);

	}

	bool validateMove(size_t row1, size_t col1, size_t row2, size_t col2, std::array<std::array<GameTile, 8>, 8>& Board, bool sim = false){


		if (color) {
			size_t colDist = col2 - col1;
			bool validDist;
			if (!pawnMovedBlack[col1]) {
				validDist = (row2 - row1) <= 2 && (row2 - row1) >= 1;
				if (!sim) {
					pawnMovedBlack[col1] = true;
					pawnMovedBlackLastTurn[col1] = true;
				}
			}
			else {

				validDist = (row2 - row1) == 1;
				if (!sim) {
					pawnMovedBlackLastTurn[col1] = false;
				}
			}
			if (!validDist) {
				return false;
			}
			bool pieceOccupy = Board[row2][col2].piece != 0; //a basic diagonal attack

			

			//Pawn is trying to attack 1 space diagonally
			if (std::abs((int)colDist) == 1) {

				if (!pieceOccupy) {	//trying to attack an empty space
					//en passant. If the enemy piece has moved two spaces out of their starting position
					//then an attacking pawn that can attack the space that the double moving 
					//attacking pawn traveled over can attack that space and capture the pawn that moved two squares
					//only valid in the case that the pawn has moved twice out of start (i.e., as if the pawn had moved once and was being attacked directly)
					bool pieceOccupyEnPassant = Board[row2 - 1][col2].piece != 0 && pawnMovedWhiteLastTurn[col2];
					return pieceOccupyEnPassant;
				}
				bool attackedPieceOccupied = Board[row2][col2].pieceColor == !color; //if the attacked piece is the opposite color of mine
				if (attackedPieceOccupied) {
					return validDist && attackedPieceOccupied;
				}
			}
			return validDist && !pieceOccupy;
		}
		else {
			size_t colDist = col2 - col1;
			bool validDist;
			if (!pawnMovedWhite[col1]) {
				validDist = (row2 - row1) >= -2 && (row2 - row1) <= -1;
				pawnMovedWhite[col1] = true;
				pawnMovedWhiteLastTurn[col1] = true;
			}
			else {
				pawnMovedWhiteLastTurn[col1] = false;
				validDist = (row2 - row1) == -1;
			}
			if (!validDist) {
				return false;
			}
			bool pieceOccupy = Board[row2][col2].piece != 0;
			

			//Pawn is trying to attack 1 space diagonally
			if (std::abs((int) colDist) == 1) {
				
				if (!pieceOccupy) {	//trying to attack an empty space
					bool pieceOccupyEnPassant = Board[row2 + 1][col2].piece != 0 && pawnMovedBlackLastTurn[col2];
					return pieceOccupyEnPassant;
				}
				bool attackedPieceOccupied = Board[row2][col2].pieceColor == !color; //if the attacked piece is the opposite color of mine
				if (attackedPieceOccupied) {
					return validDist && attackedPieceOccupied;
				}
				
			}

			return validDist && !pieceOccupy;
		}
	}

	
};




/*
	A chess board is an aggregate of 8x8 tiles.
	Each tile keeps track of it's own color, the kind of piece that's on it
		and the color of that piece
*/
struct ChessBoard {
private:
	std::array<std::array<GameTile, 8>, 8> Board;
	std::unordered_map<unsigned long, std::string> bitToPiece{ {0, "-"}, {1, "P"}, {2, "R"}, {3, "Kn"}, {4, "B"}, {5, "Q"}, {6, "Ki"} };
	bool whosTurn = false; //starts with white
	
public:
	ChessBoard(bool colorWhite) {
		
		int i = 0;
		int bPieces = 16;
		for (auto& row : Board) {
			int j = 0;
			for (auto& tile : row) {
				
				if (colorWhite) {
					tile.color = 0b1;
				}
				else {
					tile.color = 0;
				}
				colorWhite != colorWhite;

				tile.piece = boardInit[i][j];
				if (bPieces > 0) {
					tile.pieceColor = true;
				}
				else {
					tile.pieceColor = false;
				}
				bPieces--;
				j++;
			}
			i++;
		}
	}

	void printBoard() {
		
		Move(0, 4, 0, 6, false);

		for (auto& row : Board) {
			for (auto& tile : row) {
				std::string pieceColor = "";
				if(tile.piece != pm.EMPTY)
					pieceColor = (tile.pieceColor) ? "b" : "w";
				//appends piece Color of a given tile as well as the name of the piece that is stored on that tile
				std::cout << std::setw(4) << pieceColor + bitToPiece.find(tile.piece.to_ulong())->second << " ";
			}
			std::cout << '\n';
		}
		
	}

	auto& getBoard() {
		return Board;
	}

	/*
		iterates over all possible moves and if none remove the king from check
		then the king is in checkmate
	*/
	bool isCheckMate(bool whichKing) {	

		bool tempIsCheckMate = true;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (Board[i][j].pieceColor && whichKing) {
					for (int k = 0; k < 8; k++) {
						for (int l = 0; l < 8; l++) {
							auto& to = Board[k][l];
							bool valid = Move(i, j, k, l, true, false, true);
							if (valid) {
								tempIsCheckMate = false;
							}
						}
					}
				}
				else if (!Board[i][j].pieceColor && !whichKing) {
					for (int k = 0; k < 8; k++) {
						for (int l = 0; l < 8; l++) {
							bool valid = Move(i, j, k, l, true, false, false);
							if (valid) {
								tempIsCheckMate = false;
							}
						}
					}
				}

			}
		}
		return tempIsCheckMate;
	}

	bool isStaleMate(bool whichColor) {

		bool bIsInCheck = isInCheck(whichColor);
		if (bIsInCheck)
			return false;

		bool hasValidMove = false;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (Board[i][j].pieceColor && whichColor) {
					for (int k = 0; k < 8; k++) {
						for (int l = 0; l < 8; l++) {
							bool valid = Move(i, j, k, l, true, false, true);
							if (valid) {
								hasValidMove = true;
							}
						}
					}
				}
				else if (!Board[i][j].pieceColor && !whichColor) {
					for (int k = 0; k < 8; k++) {
						for (int l = 0; l < 8; l++) {
							bool valid = Move(i, j, k, l, true, false, false);
							if (valid) {
								hasValidMove = true;
							}
						}
					}
				}

			}
		}
		return !hasValidMove;
	}

	bool Move(size_t row1, size_t col1, size_t row2, size_t col2, bool whichKing, bool sim = false, bool kingCheck = false) {


		std::string pieceName = bitToPiece.find(Board[row1][col1].piece.to_ulong())->second;
		if (pieceName == "-") return false; //moving from an empty square

		bool moveIsValid = false;
		bool pieceColor = Board[row1][col1].pieceColor;
		bool kingCastle = false;
		bool tryKingCastle = false;
		//iterates over all possible pieces
		//and initializes each one as black "Piece piece(true)" or white "Piece piece"
		//and subsequently calculates whether or not that move is valid with that piece
		if (pieceName == "P") {
			pieceColor = Board[row1][col1].pieceColor;
			if (Board[row1][col1].pieceColor) {
				Pawn p(true);
				moveIsValid = p.move(row1, col1, row2, col2, Board, sim);
				
			}
			else {
				Pawn p;
				moveIsValid = (p.move(row1, col1, row2, col2, Board, sim));
			}
			
		}
		else if (pieceName == "Kn") {
			if (Board[row1][col1].pieceColor) {
				Knight kn(true);
				moveIsValid = (kn.move(row1, col1, row2, col2, Board));
			}
			else {
				Knight kn;
				moveIsValid = (kn.move(row1, col1, row2, col2, Board));
			}
		}
		else if (pieceName == "B") {
			if (Board[row1][col1].pieceColor) {
				Bishop b(true);
				moveIsValid = (b.move(row1, col1, row2, col2, Board));
			}
			else {
				Bishop b;
				moveIsValid = (b.move(row1, col1, row2, col2, Board));
			}
		}
		else if (pieceName == "Q") {
			if (Board[row1][col1].pieceColor) {
				Queen q(true);
				moveIsValid = (q.move(row1, col1, row2, col2, Board));
			}
			else {
				Queen q;
				moveIsValid = (q.move(row1, col1, row2, col2, Board));
			}
		}
		else if (pieceName == "R") {
			if (Board[row1][col1].pieceColor) {
				Rook r(true);
				moveIsValid = (r.move(row1, col1, row2, col2, Board));
			}
			else {
				Rook r;
				moveIsValid = (r.move(row1, col1, row2, col2, Board));
			}
		}
		else if (pieceName == "Ki") {
			if (Board[row1][col1].pieceColor) {
				King ki(true);
				moveIsValid = (ki.move(row1, col1, row2, col2, Board, sim));
				if (moveIsValid && std::abs((int)(col2 - col1)) == 2) {
					tryKingCastle = true;
					bool spaceBetweenAttacked = false;
					if (col2 - col1 == 2) {	//king side castle, have to check if any tile is being attacked
						spaceBetweenAttacked = isTileAttacked(pieceColor, row1, col1 + 1) || isTileAttacked(pieceColor, row1, col1 + 2);
					}
					else {
						spaceBetweenAttacked = isTileAttacked(pieceColor, row1, col1 - 1) || isTileAttacked(pieceColor, row1, col1 - 2);
					}
					kingCastle = true && !spaceBetweenAttacked;
				}
					
			}
			else {
				King ki;
				moveIsValid = (ki.move(row1, col1, row2, col2, Board, sim));
				if (moveIsValid && std::abs((int)(col2 - col1)) == 2) {
					tryKingCastle = true;
					bool spaceBetweenAttacked = false;
					if (col2 - col1 == 2) {	//king side castle, have to check if any tile is being attacked
						spaceBetweenAttacked = isTileAttacked(pieceColor, row1, col1 + 1) || isTileAttacked(pieceColor, row1, col1 + 2);
					}
					else {
						spaceBetweenAttacked = isTileAttacked(pieceColor, row1, col1 - 1) || isTileAttacked(pieceColor, row1, col1 - 2);
					}
					kingCastle = true && !spaceBetweenAttacked;
				}
					
			}
		}
		/*
			If we're simulating a move (this is true if this function is called from isInCheck()) 
			then we don't actually want to swap, we just want to check if the move is valid
		*/
		if (sim) {
			if (whichKing) {
				if (moveIsValid && pieceColor == kingCheck)
					return false;
				return moveIsValid;
			}
			return moveIsValid;
		} else {
			if (moveIsValid) {

				auto temp = Board[row2][col2]; //save piece values so after the swap we can restore board state
				auto temp2 = Board[row1][col1];

				bool blackEnPassant = std::abs((int)(col2 - col1)) && pieceColor && pawnMovedWhiteLastTurn[col2] && Board[row1][col1].piece == pm.PAWN;
				bool whiteEnPassant = std::abs((int)(col2 - col1)) && !pieceColor && pawnMovedBlackLastTurn[col2] && Board[row1][col1].piece == pm.PAWN;
				if (blackEnPassant) {  //need to handle a swap during an en passant attack
					temp = Board[row2 - 1][col2];
					swap(row1, col1, row2 - 1, col2); //a potentially temporary swap on the board, is reverted back if the move is valid and wouldn't put the king in check	
					swap(row2 - 1, col2, row2, col2);
				}
				else if (whiteEnPassant) {
					temp = Board[row2 + 1][col2];
					swap(row1, col1, row2 + 1, col2);
					swap(row2 + 1, col2, row2, col2);
				}
				else {
					if (kingCastle) {
						size_t row = pieceColor ? 0 : 7; //handles swapping ? black rooks : white rooks.
						swap(row1, col1, row2, col2);
						if (col2 - col1 == -2) {
							swap(row, 0, row, 3);
						}
						else {
							swap(row, 7, row, 5);
						}
					}
					else {
						if(!tryKingCastle)
							swap(row1, col1, row2, col2);
					}
					 
				}
				 


				bool check;
				if (!whichKing) {
					if (Board[row1][col1].pieceColor) {	//if the black king is the one to check
						check = isInCheck(true);
					}
					else { //if the white king is the one to check
						check = isInCheck(false);
					}
					if (check) {  //restore board state
						if (blackEnPassant) {
							Board[row2 + 1][col2] = temp;
						}
						else if (whiteEnPassant) {
							Board[row2 - 1][col2] = temp;
						}
						else {
							Board[row2][col2] = temp;

							Board[row1][col1] = temp2;
						}
						
						return false;
					}
					if (pieceName == "P") {	//handles double
						if (Board[row1][col1].pieceColor) {
							if (row1 == 1) {
								pawnMovedBlack[col1] = true;
							}
						}
						else {
							if (row1 == 6) {
								pawnMovedWhite[col1] = true;
							}
						}
					}
					return true; //not in check, move is valid
				}
				else {
					if (kingCheck) {
						check = isInCheck(true);
						if (check) {  //restore board state
							Board[row2][col2] = temp;

							Board[row1][col1] = temp2;
							return false;
						}
						return true; //not in check, move is valid
					}
					else {
						check = isInCheck(false);
						if (check) {  //restore board state
							Board[row2][col2] = temp;

							Board[row1][col1] = temp2;
							return false;
						}
						return true; //not in check, move is valid
					}
				}
			}
			else {
				return false; //move wasn't valid to begin with
			}
		}
	}

	void swap(size_t row1, size_t col1, size_t row2, size_t col2) {
		auto temp = Board[row1][col1];
		GameTile emptyGameTile;
		emptyGameTile.color = temp.color;
		emptyGameTile.piece = pm.EMPTY;
		Board[row2][col2] = temp;
		Board[row1][col1] = emptyGameTile;
	}
	//todo
	bool wouldPin(size_t row1, size_t col1, size_t row2, size_t col2) {
		return false;
	}

	/*
		This function repeatedly calls Move from every possible attacking square
		, simulates that move with the 'true' flag to Move. And if any enemy
		piece has a valid move covering the moving players king then this returns true.
	*/
	bool isInCheck(bool whichKing) {
		auto kingCoord = getKingCoord(whichKing);
		bool check = false;
		for (size_t i = 0; i < 8; i++) {
			for (size_t j = 0; j < 8; j++) {
				if (Move(i, j, kingCoord.first, kingCoord.second, false, true)) { //simulate a move that might attack the king
					return true;
				}
			}
		}
		return false;
	}

	bool isTileAttacked(bool whichColor, size_t row, size_t col) {
		for (size_t i = 0; i < 8; i++) {
			for (size_t j = 0; j < 8; j++) {
				if (Move(i, j, row, col, true, true, whichColor)) { //simulate a move that might attack the king
					return true;
				}
			}
		}
		return false;
	}

	

	/*
		returns : whichKing ? black king 2-d coord : white king 2-d coord
	*/
	std::pair<size_t, size_t> getKingCoord(bool whichKing) {
		std::pair<size_t, size_t> kingCoord;

		for (size_t i = 0; i < 8; i++) {
			for (size_t j = 0; j < 8; j++) {
				if (bitToPiece.find(Board[i][j].piece.to_ulong())->second == "Ki") {
					if (whichKing && Board[i][j].pieceColor) {
						kingCoord.first = i;
						kingCoord.second = j;
					}
					else {
						if (!whichKing && !Board[i][j].pieceColor) {
							kingCoord.first = i;
							kingCoord.second = j;
						}
					}
				}
			}
		}
		return kingCoord;
	}



};