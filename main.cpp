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

public:
    CheckersGame() : window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Шашки") {
        window.setVerticalSyncEnabled(true);

        // Инициализация доски
        board.resize(BOARD_SIZE, std::vector<Piece>(BOARD_SIZE));

        // Настройка фигур
        pieceShape.setRadius(CELL_SIZE / 2 - 10);
        pieceShape.setOutlineThickness(2);
        pieceShape.setOutlineColor(sf::Color::Black);

        kingShape.setRadius(CELL_SIZE / 2 - 10);
        kingShape.setOutlineThickness(2);
        kingShape.setOutlineColor(sf::Color::Black);
        kingShape.setPointCount(6); // Шестиугольник для дамок

        // Загрузка стандартного шрифта
        if (!font.loadFromFile("arial.ttf")) {
            // Если шрифт не загружен, создаем базовый
            font.loadFromMemory(NULL, 0);
        }

        initializeBoard();
    }

    void initializeBoard() {
        // Расстановка белых шашек
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                if ((row + col) % 2 == 1) {
                    board[row][col] = {PieceType::Man, PieceColor::White};
                }
            }
        }

        // Расстановка черных шашек
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

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
        }
    }

    void handleMouseClick(int x, int y) {
        int col = x / CELL_SIZE;
        int row = y / CELL_SIZE;

        if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) {
            return;
        }

        checkForMandatoryCaptures();

        if (board[row][col].color == currentPlayer) {
            if (mustCapture && !canPieceCapture(row, col)) {
                return;
            }

            selectedPiecePos = {row, col};
            board[row][col].selected = true;
            calculatePossibleMoves(row, col);
        }
        else if (isMoving && isPossibleMove(row, col)) {
            movePiece(selectedPiecePos.x, selectedPiecePos.y, row, col);

            if (abs(selectedPiecePos.x - row) == 2) {
                if (canContinueCapturing(row, col)) {
                    selectedPiecePos = {row, col};
                    board[row][col].selected = true;
                    calculatePossibleMoves(row, col);
                    return;
                }
            }

            currentPlayer = (currentPlayer == PieceColor::White) ? PieceColor::Black : PieceColor::White;
            isMoving = false;
            mustCapture = false;
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
            int direction = (board[row][col].color == PieceColor::White) ? 1 : -1;

            if (canCapture(row, col, row + direction, col - 1, row + 2*direction, col - 2) ||
                canCapture(row, col, row + direction, col + 1, row + 2*direction, col + 2)) {
                return true;
            }
        }
        else if (board[row][col].type == PieceType::King) {
            for (int dr = -1; dr <= 1; dr += 2) {
                for (int dc = -1; dc <= 1; dc += 2) {
                    int r = row + dr;
                    int c = col + dc;
                    bool foundOpponent = false;

                    while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
                        if (board[r][c].color != PieceColor::None) {
                            if (board[r][c].color == currentPlayer) {
                                break;
                            }
                            else {
                                if (foundOpponent) break;
                                foundOpponent = true;
                                int nextR = r + dr;
                                int nextC = c + dc;
                                if (nextR >= 0 && nextR < BOARD_SIZE && nextC >= 0 && nextC < BOARD_SIZE &&
                                    board[nextR][nextC].color == PieceColor::None) {
                                    return true;
                                }
                                break;
                            }
                        }
                        r += dr;
                        c += dc;
                    }
                }
            }
        }
        return false;
    }

    bool canContinueCapturing(int row, int col) {
        if (board[row][col].type == PieceType::Man) {
            int direction = (board[row][col].color == PieceColor::White) ? 1 : -1;

            if (canCapture(row, col, row + direction, col - 1, row + 2*direction, col - 2) ||
                canCapture(row, col, row + direction, col + 1, row + 2*direction, col + 2) ||
                canCapture(row, col, row - direction, col - 1, row - 2*direction, col - 2) ||
                canCapture(row, col, row - direction, col + 1, row - 2*direction, col + 2)) {
                return true;
            }
        }
        else if (board[row][col].type == PieceType::King) {
            for (int dr = -1; dr <= 1; dr += 2) {
                for (int dc = -1; dc <= 1; dc += 2) {
                    int r = row + dr;
                    int c = col + dc;
                    bool foundOpponent = false;

                    while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
                        if (board[r][c].color != PieceColor::None) {
                            if (board[r][c].color == currentPlayer) {
                                break;
                            }
                            else {
                                if (foundOpponent) break;
                                foundOpponent = true;
                                int nextR = r + dr;
                                int nextC = c + dc;
                                if (nextR >= 0 && nextR < BOARD_SIZE && nextC >= 0 && nextC < BOARD_SIZE &&
                                    board[nextR][nextC].color == PieceColor::None) {
                                    return true;
                                }
                                break;
                            }
                        }
                        r += dr;
                        c += dc;
                    }
                }
            }
        }
        return false;
    }

    void calculatePossibleMoves(int row, int col) {
        possibleMoves.clear();
        captureMoves.clear();
        isMoving = true;

        if (board[row][col].type == PieceType::Man) {
            int direction = (board[row][col].color == PieceColor::White) ? 1 : -1;

            if (!mustCapture) {
                checkMove(row, col, row + direction, col - 1);
                checkMove(row, col, row + direction, col + 1);
            }

            checkCapture(row, col, row + direction, col - 1, row + 2*direction, col - 2);
            checkCapture(row, col, row + direction, col + 1, row + 2*direction, col + 2);
        }
        else if (board[row][col].type == PieceType::King) {
            for (int dr = -1; dr <= 1; dr += 2) {
                for (int dc = -1; dc <= 1; dc += 2) {
                    int r = row + dr;
                    int c = col + dc;
                    bool foundOpponent = false;

                    while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
                        if (board[r][c].color != PieceColor::None) {
                            if (board[r][c].color == currentPlayer) {
                                break;
                            }
                            else {
                                if (foundOpponent) break;
                                foundOpponent = true;
                                int nextR = r + dr;
                                int nextC = c + dc;
                                if (nextR >= 0 && nextR < BOARD_SIZE && nextC >= 0 && nextC < BOARD_SIZE &&
                                    board[nextR][nextC].color == PieceColor::None) {
                                    if (mustCapture) {
                                        captureMoves.push_back({nextR, nextC});
                                    }
                                    else {
                                        possibleMoves.push_back({nextR, nextC});
                                    }
                                }
                                break;
                            }
                        }
                        else if (!mustCapture && !foundOpponent) {
                            possibleMoves.push_back({r, c});
                        }
                        r += dr;
                        c += dc;
                    }
                }
            }
        }

        if (!captureMoves.empty()) {
            possibleMoves = captureMoves;
        }
    }

    bool canCapture(int fromRow, int fromCol, int midRow, int midCol, int toRow, int toCol) {
        if (toRow >= 0 && toRow < BOARD_SIZE && toCol >= 0 && toCol < BOARD_SIZE) {
            if (board[midRow][midCol].color != PieceColor::None &&
                board[midRow][midCol].color != board[fromRow][fromCol].color &&
                board[toRow][toCol].color == PieceColor::None) {
                return true;
            }
        }
        return false;
    }

    void checkMove(int fromRow, int fromCol, int toRow, int toCol) {
        if (toRow >= 0 && toRow < BOARD_SIZE && toCol >= 0 && toCol < BOARD_SIZE) {
            if (board[toRow][toCol].color == PieceColor::None) {
                possibleMoves.push_back({toRow, toCol});
            }
        }
    }

    void checkCapture(int fromRow, int fromCol, int midRow, int midCol, int toRow, int toCol) {
        if (canCapture(fromRow, fromCol, midRow, midCol, toRow, toCol)) {
            captureMoves.push_back({toRow, toCol});
        }
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

        // Отрисовка доски
        for (int row = 0; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);
                cell.setFillColor((row + col) % 2 == 0 ? sf::Color(210, 180, 140) : sf::Color(139, 69, 19));
                window.draw(cell);
            }
        }

        // Отрисовка возможных ходов
        for (const auto& pos : possibleMoves) {
            sf::RectangleShape highlight(sf::Vector2f(CELL_SIZE - 10, CELL_SIZE - 10));
            highlight.setPosition(pos.y * CELL_SIZE + 5, pos.x * CELL_SIZE + 5);
            highlight.setFillColor(sf::Color(0, 255, 0, 100));
            window.draw(highlight);
        }

        // Отрисовка фигур
        for (int row = 0; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                if (board[row][col].color != PieceColor::None) {
                    sf::Vector2f position(col * CELL_SIZE + CELL_SIZE / 2,
                                        row * CELL_SIZE + CELL_SIZE / 2);

                    if (board[row][col].type == PieceType::Man) {
                        pieceShape.setPosition(position.x - pieceShape.getRadius(),
                                            position.y - pieceShape.getRadius());
                        pieceShape.setFillColor(board[row][col].color == PieceColor::White ?
                                            sf::Color::White : sf::Color(150, 0, 0)); // Темно-красный
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

        // Отображение текущего игрока
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
