#include <SFML/Graphics.hpp>
#include <iostream>

const int WINDOW_SIZE = 800;
const int CELL_SIZE = WINDOW_SIZE / 8;
const float PIECE_RADIUS = CELL_SIZE * 0.4f;

class CheckersGame {
private:
    sf::RenderWindow window;
    sf::RectangleShape board[8][8];
    sf::CircleShape pieces[8][8];
    bool isWhiteTurn = true;

    void initializeBoard() {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {

                board[i][j].setSize(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                board[i][j].setPosition(i * CELL_SIZE, j * CELL_SIZE);

                if ((i + j) % 2 == 0) {
                    board[i][j].setFillColor(sf::Color(210, 180, 140)); // Бежевый
                } else {
                    board[i][j].setFillColor(sf::Color(139, 69, 19));   // Коричневый
                }

                if ((i + j) % 2 != 0 && j < 3) {
                    pieces[i][j].setRadius(PIECE_RADIUS);
                    pieces[i][j].setFillColor(sf::Color::White);
                    pieces[i][j].setPosition(i * CELL_SIZE + (CELL_SIZE - 2*PIECE_RADIUS)/2,
                                           j * CELL_SIZE + (CELL_SIZE - 2*PIECE_RADIUS)/2);
                    pieces[i][j].setOutlineThickness(2);
                    pieces[i][j].setOutlineColor(sf::Color::Black);
                }
                else if ((i + j) % 2 != 0 && j > 4) {
                    pieces[i][j].setRadius(PIECE_RADIUS);
                    pieces[i][j].setFillColor(sf::Color::Red);
                    pieces[i][j].setPosition(i * CELL_SIZE + (CELL_SIZE - 2*PIECE_RADIUS)/2,
                                           j * CELL_SIZE + (CELL_SIZE - 2*PIECE_RADIUS)/2);
                    pieces[i][j].setOutlineThickness(2);
                    pieces[i][j].setOutlineColor(sf::Color::Black);
                }
            }
        }
    }

public:
    CheckersGame() : window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Шашки") {
        window.setFramerateLimit(60);
        initializeBoard();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            render();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int x = event.mouseButton.x / CELL_SIZE;
                    int y = event.mouseButton.y / CELL_SIZE;

                    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                        std::cout << "Клетка: " << x << ", " << y << std::endl;
                    }
                }
            }
        }
    }

    void render() {
        window.clear(sf::Color::White);

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                window.draw(board[i][j]);
            }
        }

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (pieces[i][j].getRadius() > 0) {
                    window.draw(pieces[i][j]);
                }
            }
        }

        window.display();
    }
};

int main() {
    CheckersGame game;
    game.run();
    return 0;
}
