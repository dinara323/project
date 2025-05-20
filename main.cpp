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

    bool isValidPosition(int row, int col) const {
        return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
    }

    void checkDirection(int row, int col, int dr, int dc, bool checkCaptures) {
        if (board[row][col].type == PieceType::Man) {
            int forward_dir = (board[row][col].color == PieceColor::White) ? 1 : -1;

            if (checkCaptures) {
                int newRow = row + dr;
                int newCol = col + dc;
                int jumpRow = row + 2 * dr;
                int jumpCol = col + 2 * dc;

                if (canCapture(row, col, newRow, newCol, jumpRow, jumpCol)) {
                    captureMoves.emplace_back(jumpRow, jumpCol);
                }
            } else {
                if (dr != forward_dir) return;

                int newRow = row + dr;
                int newCol = col + dc;

                if (isValidPosition(newRow, newCol) && board[newRow][newCol].color == PieceColor::None) {
                    possibleMoves.emplace_back(newRow, newCol);
                }
            }
        } else {
            int r = row + dr;
            int c = col + dc;
            bool foundEnemy = false;

            while (isValidPosition(r, c)) {
                if (board[r][c].color != PieceColor::None) {
                    if (board[r][c].color == currentPlayer) break;
                    if (!foundEnemy) {
                        foundEnemy = true;
                        int jumpR = r + dr;
                        int jumpC = c + dc;

                        while (isValidPosition(jumpR, jumpC) && board[jumpR][jumpC].color == PieceColor::None) {
                            captureMoves.emplace_back(jumpR, jumpC);
                            jumpR += dr;
                            jumpC += dc;
                        }
                    }
                    break;
                }
                if (!checkCaptures) possibleMoves.emplace_back(r, c);
                r += dr;
                c += dc;
            }
        }
    }

    void findKingCaptures(int row, int col, int dr, int dc) {
        int r = row + dr;
        int c = col + dc;
        bool foundEnemy = false;

        while (isValidPosition(r, c)) {
            if (board[r][c].color != PieceColor::None) {
                if (board[r][c].color == currentPlayer) break;
                if (!foundEnemy) {
                    foundEnemy = true;
                    int jumpR = r + dr;
                    int jumpC = c + dc;

                    while (isValidPosition(jumpR, jumpC) && board[jumpR][jumpC].color == PieceColor::None) {
                        captureMoves.emplace_back(jumpR, jumpC);
                        jumpR += dr;
                        jumpC += dc;
                    }
                }
                break;
            }
            r += dr;
            c += dc;
        }
    }

public:
    CheckersGame() : window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Шашки") {
        window.setVerticalSyncEnabled(true);
        board.resize(BOARD_SIZE, std::vector<Piece>(BOARD_SIZE));

        pieceShape.setRadius(CELL_SIZE / 2 - 10);
        pieceShape.setOutlineThickness(2);
        pieceShape.setOutlineColor(sf::Color::Black);

        kingShape.setRadius(CELL_SIZE / 2 - 10);
        kingShape.setOutlineThickness(2);
        kingShape.setOutlineColor(sf::Color::Black);
        kingShape.setPointCount(6);

        if (!font.loadFromFile("arial.ttf")) {
            font.loadFromMemory(NULL, 0);
        }

        initializeBoard();
    }

    void initializeBoard() {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                if ((row + col) % 2 == 1) {
                    board[row][col] = {PieceType::Man, PieceColor::White};
                }
            }
        }

        for (int row = 5; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                if ((row + col) % 2 == 1) {
                    board[row][col] = {PieceType::Man, PieceColor::Black};
                }
            }
        }
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            render();
        }
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                handleMouseClick(event.mouseButton.x, event.mouseButton.y);
            }
        }
    }

    void handleMouseClick(int x, int y) {
        int col = x / CELL_SIZE;
        int row = y / CELL_SIZE;

        if (!isValidPosition(row, col)) return;

        checkForMandatoryCaptures();

        if (board[row][col].color == currentPlayer) {
            if (mustCapture && !canPieceCapture(row, col)) return;

            selectedPiecePos = {row, col};
            board[row][col].selected = true;
            calculatePossibleMoves(row, col);
        } else if (isMoving && isPossibleMove(row, col)) {
            movePiece(selectedPiecePos.x, selectedPiecePos.y, row, col);

            if (abs(selectedPiecePos.x - row) == 2 && canContinueCapturing(row, col)) {
                selectedPiecePos = {row, col};
                board[row][col].selected = true;
                calculatePossibleMoves(row, col);
                return;
            }

            currentPlayer = (currentPlayer == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            isMoving = mustCapture = false;
            clearPossibleMoves();

            if (checkWinCondition()) {
                std::cout << (currentPlayer == PieceColor::White ? "Black" : "White") << " wins!" << std::endl;
                window.close();
            }
        }
    }

    void checkForMandatoryCaptures() {
        mustCapture = false;
        for (int row = 0; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                if (board[row][col].color == currentPlayer && canPieceCapture(row, col)) {
                    mustCapture = true;
                    return;
                }
            }
        }
    }

    bool canPieceCapture(int row, int col) {
        if (board[row][col].type == PieceType::Man) {
            for (int dr : {-1, 1}) {
                for (int dc : {-1, 1}) {
                    if (canCapture(row, col, row + dr, col + dc, row + 2*dr, col + 2*dc)) {
                        return true;
                    }
                }
            }
        } else {
            for (int dr : {-1, 1}) {
                for (int dc : {-1, 1}) {
                    int r = row + dr;
                    int c = col + dc;
                    bool foundEnemy = false;

                    while (isValidPosition(r, c)) {
                        if (board[r][c].color != PieceColor::None) {
                            if (board[r][c].color == currentPlayer) break;
                            if (!foundEnemy) {
                                foundEnemy = true;
                                int jumpR = r + dr;
                                int jumpC = c + dc;
                                if (isValidPosition(jumpR, jumpC) && board[jumpR][jumpC].color == PieceColor::None) {
                                    return true;
                                }
                            }
                            break;
                        }
                        r += dr;
                        c += dc;
                    }
                }
            }
        }
        return false;
    }

    bool canCapture(int fromRow, int fromCol, int midRow, int midCol, int toRow, int toCol) {
        return isValidPosition(toRow, toCol) &&
               board[toRow][toCol].color == PieceColor::None &&
               isValidPosition(midRow, midCol) &&
               board[midRow][midCol].color != PieceColor::None &&
               board[midRow][midCol].color != board[fromRow][fromCol].color;
    }

    void calculatePossibleMoves(int row, int col) {
        possibleMoves.clear();
        captureMoves.clear();
        isMoving = true;

        if (board[row][col].type == PieceType::Man) {
            // Проверка взятий во всех направлениях
            for (int dr : {-1, 1}) {
                for (int dc : {-1, 1}) {
                    checkDirection(row, col, dr, dc, true);
                }
            }

            int forward_dir = (board[row][col].color == PieceColor::White) ? 1 : -1;
            for (int dc : {-1, 1}) {
                checkDirection(row, col, forward_dir, dc, false);
            }
        } else {
            for (int dr : {-1, 1}) {
                for (int dc : {-1, 1}) {
                    findKingCaptures(row, col, dr, dc);
                    if (!mustCapture) checkDirection(row, col, dr, dc, false);
                }
            }
        }

        if (!captureMoves.empty()) possibleMoves = captureMoves;
    }

    bool canContinueCapturing(int row, int col) {
        if (board[row][col].type == PieceType::Man) {
            for (int dr : {-1, 1}) {
                for (int dc : {-1, 1}) {
                    if (canCapture(row, col, row + dr, col + dc, row + 2*dr, col + 2*dc)) {
                        return true;
                    }
                }
            }
        } else {
            for (int dr : {-1, 1}) {
                for (int dc : {-1, 1}) {
                    int r = row + dr;
                    int c = col + dc;
                    bool foundEnemy = false;

                    while (isValidPosition(r, c)) {
                        if (board[r][c].color != PieceColor::None) {
                            if (board[r][c].color == currentPlayer) break;
                            if (!foundEnemy) {
                                foundEnemy = true;
                                int jumpR = r + dr;
                                int jumpC = c + dc;
                                if (isValidPosition(jumpR, jumpC) && board[jumpR][jumpC].color == PieceColor::None) {
                                    return true;
                                }
                            }
                            break;
                        }
                        r += dr;
                        c += dc;
                    }
                }
            }
        }
        return false;
    }

    bool isPossibleMove(int row, int col) {
        for (const auto& pos : possibleMoves) {
            if (pos.x == row && pos.y == col) {
                return true;
            }
        }
        return false;
    }

    void movePiece(int fromRow, int fromCol, int toRow, int toCol) {
        board[toRow][toCol] = board[fromRow][fromCol];
        board[fromRow][fromCol] = {PieceType::None, PieceColor::None};
        board[toRow][toCol].selected = false;

        if (board[toRow][toCol].type == PieceType::Man) {
            if ((board[toRow][toCol].color == PieceColor::White && toRow == BOARD_SIZE - 1) ||
                (board[toRow][toCol].color == PieceColor::Black && toRow == 0)) {
                board[toRow][toCol].type = PieceType::King;
            }
        }

        if (abs(fromRow - toRow) == 2) {
            int midRow = (fromRow + toRow) / 2;
            int midCol = (fromCol + toCol) / 2;
            board[midRow][midCol] = {PieceType::None, PieceColor::None};
        }
    }

    bool checkWinCondition() {
        bool whiteExists = false;
        bool blackExists = false;

        for (const auto& row : board) {
            for (const auto& piece : row) {
                if (piece.color == PieceColor::White) whiteExists = true;
                if (piece.color == PieceColor::Black) blackExists = true;
                if (whiteExists && blackExists) return false;
            }
        }
        return true;
    }

    void clearPossibleMoves() {
        possibleMoves.clear();
        captureMoves.clear();
        for (auto& row : board) {
            for (auto& piece : row) {
                piece.selected = false;
            }
        }
        selectedPiecePos = {-1, -1};
    }

    void render() {
        window.clear();

        for (int row = 0; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
                cell.setFillColor((row + col) % 2 == 0 ? sf::Color(210, 180, 140) : sf::Color(139, 69, 19));
                window.draw(cell);
            }
        }

        for (const auto& pos : possibleMoves) {
            sf::RectangleShape highlight(sf::Vector2f(CELL_SIZE - 10, CELL_SIZE - 10));
            highlight.setPosition(pos.y * CELL_SIZE + 5, pos.x * CELL_SIZE + 5);
            highlight.setFillColor(sf::Color(0, 255, 0, 100));
            window.draw(highlight);
        }

        for (int row = 0; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                if (board[row][col].color != PieceColor::None) {
                    sf::Vector2f position(col * CELL_SIZE + CELL_SIZE / 2,
                                        row * CELL_SIZE + CELL_SIZE / 2);

                    if (board[row][col].type == PieceType::Man) {
                        pieceShape.setPosition(position.x - pieceShape.getRadius(),
                                            position.y - pieceShape.getRadius());
                        pieceShape.setFillColor(board[row][col].color == PieceColor::White ?
                                            sf::Color::White : sf::Color(150, 0, 0));
                        window.draw(pieceShape);
                    } else {
                        kingShape.setPosition(position.x - kingShape.getRadius(),
                                            position.y - kingShape.getRadius());
                        kingShape.setFillColor(board[row][col].color == PieceColor::White ?
                                            sf::Color::White : sf::Color(150, 0, 0));
                        window.draw(kingShape);
                    }

                    if (board[row][col].selected) {
                        sf::CircleShape selection(CELL_SIZE / 2 - 5);
                        selection.setPosition(col * CELL_SIZE + 5, row * CELL_SIZE + 5);
                        selection.setFillColor(sf::Color::Transparent);
                        selection.setOutlineThickness(3);
                        selection.setOutlineColor(sf::Color::Yellow);
                        window.draw(selection);
                    }
                }
            }
        }

        sf::Text text;
        text.setFont(font);
        text.setString("Текущий игрок: " + std::string(currentPlayer == PieceColor::White ? "Белые" : "Черные"));
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::Black);
        text.setPosition(10, 10);
        window.draw(text);

        window.display();
    }
};

int main() {
    CheckersGame game;
    game.run();
    return 0;
}
