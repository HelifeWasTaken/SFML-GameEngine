#include "Engine.hpp"
#include <iostream>

engine::Game *game;

class TestState : public engine::State
{
  private:
    sf::RectangleShape _rectangle;

  public:
    TestState() = default;

    void init() override
    {
        sf::Vector2u s = game->window().getSize();
        _rectangle.setPosition(0, 0);
        _rectangle.setSize(sf::Vector2f(s.x, s.y));
        _rectangle.setFillColor(sf::Color::Green);
    }

    void handleInput(sf::Event &event) override
    {
        (void)event;

        if (game->isReleased(sf::Keyboard::F))
        {
            game->changeState(1, 3000);
        }
        else if (game->isReleased(sf::Keyboard::Escape))
        {
            game->changeState(-1);
        }
    }

    void update() override
    {
        ImGui::BeginLock lock("Test");
        ImGui::Text("Hello World the first");
    }

    void draw(sf::RenderWindow &window) override
    {
        window.draw(_rectangle);
    }

    ENGINE_EMPTY_STATE_STOP;
};

class TestState2 : public engine::State
{
  public:
    TestState2() = default;

    ENGINE_EMPTY_STATE_INIT;

    void handleInput(sf::Event &event) override
    {
        (void)event;
        if (game->isReleased(sf::Keyboard::F))
        {
            game->changeState(0, 3000);
        }
        if (game->isReleased(sf::Keyboard::Escape))
        {
            game->changeState(-1);
        }
    }

    void update() override
    {
        bool some;
        ImGui::BeginLock lockBegin("Test2", &some, ImGuiWindowFlags_MenuBar);
        {
            ImGui::EngineMenuBar([]() {
                ImGui::EngineMenu("menu", []() {
                    if (ImGui::MenuItem("Open..", "Ctrl+O")) {
                        std::cout << "No Open..!" << std::endl;
                    }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {
                        std::cout << "No Save!" << std::endl;
                    }
                    if (ImGui::MenuItem("Close", "Ctrl+W")) {
                        std::cout << "Close!" << std::endl;
                        game->changeState(-1);
                    }
                });
            });
        }
    }

    ENGINE_EMPTY_STATE_DRAW;

    ENGINE_EMPTY_STATE_STOP;
};

// Sample code
int main()
{
    engine::Game::construct();

    game = engine::Game::gameInstance();

    game->addState(new TestState());
    game->addState(new TestState2());
    game->changeState(0);
    game->run();

    engine::Game::deconstruct();
}
