#ifndef CHECKERS_H
#define CHECKERS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <algorithm>

const int WINDOW_SIZE = 800;
const int BOARD_SIZE = 8;
const int CELL_SIZE = WINDOW_SIZE / BOARD_SIZE;

enum class PieceType { None, Man, King };
enum class PieceColor { None, White, Black };

struct Piece {
    PieceType type = PieceType::None;
    PieceColor color = PieceColor::None;
    bool selected = false;
};

class CheckersGame {
private:
    sf::RenderWindow window;
    std::vector<std::vector<Piece>> board;
    PieceColor currentPlayer = PieceColor::White;
    bool isMoving = false;
    sf::Vector2i selectedPiecePos = {-1, -1};
    std::vector<sf::Vector2i> possibleMoves;
    std::vector<sf::Vector2i> captureMoves;
    bool mustCapture = false;

    sf::CircleShape pieceShape;
    sf::CircleShape kingShape;
    sf::Font font;

    bool isValidPosition(int row, int col) const;
    void checkDirection(int row, int col, int dr, int dc, bool checkCaptures);
    void findKingCaptures(int row, int col, int dr, int dc);
    bool canCapture(int fromRow, int fromCol, int midRow, int midCol, int toRow, int toCol);
    bool canPieceCapture(int row, int col);
    void checkForMandatoryCaptures();
    void calculatePossibleMoves(int row, int col);
    bool isPossibleMove(int row, int col);
    void movePiece(int fromRow, int fromCol, int toRow, int toCol);
    bool canContinueCapturing(int row, int col);
    bool checkWinCondition();
    void clearPossibleMoves();
    void handleMouseClick(int x, int y);

public:
    CheckersGame();
    void run();
    void handleEvents();
    void render();
    void initializeBoard();
};

#endif // CHECKERS_H
