#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

namespace engine
{
template <typename T> class ResourceHolder
{
  private:
    std::map<std::string, std::unique_ptr<T>> _resourceMap;

  public:
    void load(const std::string &name, const std::string &filename)
    {
        std::unique_ptr<T> resource(new T());

        if (!resource->loadFromFile(filename))
            throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);
        _resourceMap[name] = std::move(resource);
    }

    T &get(const std::string &name)
    {
        auto found = _resourceMap.find(name);
        if (found == _resourceMap.end())
            throw std::runtime_error("ResourceHolder::get - Resource not found: " + name);
        return *found->second;
    }

    void clear()
    {
        _resourceMap.clear();
    }
};

class ResourceManager
{
  private:
    ResourceHolder<sf::Texture> _textures;
    ResourceHolder<sf::Font> _fonts;
    ResourceHolder<sf::SoundBuffer> _soundBuffers;
    ResourceHolder<sf::Image> _images;

  public:
    void loadTexture(const std::string &name, const std::string &filename)
    {
        _textures.load(name, filename);
    }
    void loadFont(const std::string &name, const std::string &filename)
    {
        _fonts.load(name, filename);
    }
    void loadSoundBuffer(const std::string &name, const std::string &filename)
    {
        _soundBuffers.load(name, filename);
    }
    void loadImage(const std::string &name, const std::string &filename)
    {
        loadTexture(name, filename);
    }

    sf::Texture &getTexture(const std::string &name)
    {
        return _textures.get(name);
    }
    sf::Font &getFont(const std::string &name)
    {
        return _fonts.get(name);
    }
    sf::SoundBuffer &getSoundBuffer(const std::string &name)
    {
        return _soundBuffers.get(name);
    }

    void clear()
    {
        _textures.clear();
        _fonts.clear();
        _soundBuffers.clear();
        _images.clear();
    }
};

class Animator
{
  private:
    sf::Sprite &_sprite;
    sf::Texture &_texture;
    std::unordered_map<std::string, std::vector<sf::IntRect>> _frames;
    size_t _currentFrame = 0;
    std::string _animation = "";
    double _lastFrameChange;
    double _delay;
    sf::Clock _clock;

  public:
    Animator(sf::Sprite &sprite, sf::Texture &texture, double delay, double offset = 0)
        : _sprite(sprite), _texture(texture), _lastFrameChange(-offset), _delay(delay)
    {
        _sprite.setTexture(_texture);
        _sprite.setTextureRect(_frames.at(0).at(0));
    }

    Animator &addAnimation(const std::string &s, const std::vector<sf::IntRect> &frames)
    {
        if (s == "")
            throw std::runtime_error("Can not add an animation with an empty name");
        _frames[s] = frames;
        return *this;
    }

    Animator &update()
    {
        if (_clock.getElapsedTime().asMilliseconds() - _lastFrameChange >= _delay)
        {
            _currentFrame = (_currentFrame + 1) % _frames.size();
            _sprite.setTextureRect(_frames.at(_animation).at(_currentFrame));
            _lastFrameChange = _clock.getElapsedTime().asMilliseconds();
        }
        return *this;
    }

    Animator &setAnimation(const std::string &animation)
    {
        _animation = animation;
        _currentFrame = 0;
        _sprite.setTextureRect(_frames.at(_animation).at(0));
        _lastFrameChange = _clock.getElapsedTime().asMilliseconds() - _delay;
        return *this;
    }

    static std::vector<sf::IntRect> getSpriteRects(const sf::Vector2u &frameSize, const sf::Texture &texture,
                                                   const unsigned int &frames,
                                                   const sf::Vector2u &startPos = sf::Vector2u(0, 0),
                                                   const sf::Vector2u &spacing = sf::Vector2u(0, 0))
    {
        const sf::Vector2u textureSize = texture.getSize();
        std::vector<sf::IntRect> rects;

        for (unsigned int i = 0, x = startPos.x, y = startPos.y; i < frames; i++)
        {
            rects.push_back(sf::IntRect(x, y, frameSize.x, frameSize.y));
            x += frameSize.x + spacing.x;
            if (x + frameSize.x > textureSize.x)
            {
                x = startPos.x;
                y += frameSize.y + spacing.y;
            }
        }
        return rects;
    }

    sf::Sprite &sprite()
    {
        return _sprite;
    }
};

class InputManager
{
  public:
    enum KeyState
    {
        Idle,
        Released,
        Pressed,
        Held
    };

  private:
    KeyState _kbkeys[sf::Keyboard::KeyCount] = {Idle};
    KeyState _mskeys[sf::Mouse::ButtonCount] = {Idle};

  public:
    void update(sf::Event &event)
    {
        (void)event;
        for (int k = 0; k < sf::Keyboard::KeyCount; k++)
        {
            if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(k)))
            {
                if (_kbkeys[k] == Idle)
                {
                    _kbkeys[k] = Pressed;
                }
                else
                {
                    _kbkeys[k] = Held;
                }
            }
            else if (_kbkeys[k] == Pressed || _kbkeys[k] == Held)
            {
                _kbkeys[k] = Released;
            }
            else
            {
                _kbkeys[k] = Idle;
            }
        }
        for (int k = 0; k < sf::Mouse::ButtonCount; k++)
        {
            if (sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(k)))
            {
                if (_mskeys[k] == Idle)
                {
                    _mskeys[k] = Pressed;
                }
                else
                {
                    _mskeys[k] = Held;
                }
            }
            else if (_mskeys[k] == Pressed || _mskeys[k] == Held)
            {
                _mskeys[k] = Released;
            }
            else
            {
                _mskeys[k] = Idle;
            }
        }
    }

    KeyState getKeyState(sf::Keyboard::Key key)
    {
        return _kbkeys[key];
    }
    KeyState getKeyState(sf::Mouse::Button key)
    {
        return _mskeys[key];
    }

    sf::Vector2i getMousePos()
    {
        return sf::Mouse::getPosition();
    }

    sf::Vector2i getMousePosPixel(const sf::View &view);
};

#define ENGINE_EMPTY_STATE_INIT                                                                                        \
    void init() override                                                                                               \
    {                                                                                                                  \
    }
#define ENGINE_EMPTY_STATE_HANDLE_INPUT                                                                                \
    void handleInput(sf::Event &event) override                                                                        \
    {                                                                                                                  \
        (void)event;                                                                                                   \
    }
#define ENGINE_EMPTY_STATE_UPDATE                                                                                      \
    void update() override                                                                                             \
    {                                                                                                                  \
    }
#define ENGINE_EMPTY_STATE_DRAW                                                                                        \
    void draw(sf::RenderWindow &window) override                                                                       \
    {                                                                                                                  \
        (void)window;                                                                                                  \
    }
#define ENGINE_EMPTY_STATE_STOP                                                                                        \
    void stop() override                                                                                               \
    {                                                                                                                  \
    }

class State
{
  public:
    virtual void init() = 0;
    virtual void handleInput(sf::Event &event) = 0;
    virtual void update() = 0;
    virtual void draw(sf::RenderWindow &window) = 0;
    virtual void stop() = 0;
    virtual ~State() = default;
};

class StateMachine
{
  private:
    std::vector<std::unique_ptr<State>> _states;
    ssize_t _currentState = -1;
    ssize_t _pendingState = -1;
    sf::Clock _clock;
    double _startTime;
    double _transitionTime;
    bool _enableFade;
    bool _shouldStop = false;
    sf::RectangleShape _fade;

  public:
    ~StateMachine() = default;

    void addState(std::unique_ptr<State> &&state)
    {
        _states.emplace_back(std::move(state));
    }
    void addState(std::unique_ptr<State> &state)
    {
        addState(std::move(state));
    }
    void addState(State *state)
    {
        addState(std::unique_ptr<State>(state));
    }

    bool hasPendingState()
    {
        return _pendingState != _currentState;
    }

    void changeState(ssize_t state, double transitionTime)
    {
        if (state == -1)
        {
            _shouldStop = true;
        }
        else if (_pendingState == _currentState)
        {
            _pendingState = state;
            _startTime = _clock.getElapsedTime().asMilliseconds();
            _transitionTime = transitionTime;
            _enableFade = _transitionTime > 0;
        }
    }

    void handleInput(sf::Event &event)
    {
        _states.at(_currentState)->handleInput(event);
    }

    bool update();

    void draw(sf::RenderWindow &window)
    {
        _states.at(_currentState)->draw(window);
    }

    void start()
    {
        if (_currentState != -1)
        {
            throw std::runtime_error("StateMachine::start() - StateMachine already started");
        }
        else if (_pendingState == -1)
        {
            throw std::runtime_error("No state added to StateMachine");
        }
        else
        {
            _currentState = _pendingState;
            _states.at(_currentState)->init();
        }
    }
};

class Game
{
  private:
    static inline sf::RenderWindow *_window = nullptr;
    static inline sf::Event *_event = nullptr;
    static inline StateMachine *_stateMachine = nullptr;
    static inline ResourceManager *_resourceManager = nullptr;
    static inline InputManager *_inputManager = nullptr;
    static inline bool _builded = false;
    static inline Game *_game = nullptr;

  public:
    static sf::RenderWindow &window()
    {
        return *_window;
    }
    static sf::Event &event()
    {
        return *_event;
    }
    static StateMachine &stateMachine()
    {
        return *_stateMachine;
    }
    static ResourceManager &resourceManager()
    {
        return *_resourceManager;
    }
    static InputManager &inputManager()
    {
        return *_inputManager;
    }
    static Game *gameInstance()
    {
        return _game;
    }

    static void construct()
    {
        if (_game == nullptr)
            _game = new Game;
        if (_window == nullptr)
            _window = new sf::RenderWindow;
        if (_event == nullptr)
            _event = new sf::Event;
        if (_stateMachine == nullptr)
            _stateMachine = new StateMachine;
        if (_resourceManager == nullptr)
            _resourceManager = new ResourceManager;
        if (_inputManager == nullptr)
            _inputManager = new InputManager;
        if (ImGui::SFML::Init(*_window) == false)
            throw std::runtime_error("Failed to initialize ImGui");
        _builded = true;
    }

    static void deconstruct()
    {
        _builded = false;
        delete _window;
        delete _event;
        delete _stateMachine;
        delete _resourceManager;
        delete _inputManager;
        delete _game;
        ImGui::SFML::Shutdown();
    }

    Game() = default;

    ~Game() = default;

    void run()
    {
        sf::Clock imGuiClockDeltaTime;

        if (!_builded)
            throw std::runtime_error("Game is not constructed");
        _window->create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Game", sf::Style::Close);
        _window->setFramerateLimit(60);
        _stateMachine->start();
        while (_window->isOpen())
        {
            while (_window->pollEvent(*_event))
            {
                _inputManager->update(*_event);
                ImGui::SFML::ProcessEvent(*_event);
                if (_event->type == sf::Event::Closed)
                {
                    _stateMachine->changeState(-1, 0);
                    break;
                }
                else
                {
                    _stateMachine->handleInput(*_event);
                }
            }
            _inputManager->update(*_event);
            ImGui::SFML::Update(*_window, imGuiClockDeltaTime.restart());
            if (_stateMachine->update() == false)
            {
                _window->close();
                return;
            }
            _window->display();
            _window->clear();
            _stateMachine->draw(*_window);
            ImGui::SFML::Render(*_window);
        }
    }

    void addState(State *state)
    {
        if (state == nullptr)
            throw std::runtime_error("Cannot add null state");
        _stateMachine->addState(state);
    }

    void changeState(ssize_t state, double transitionTime = 2000)
    {
        _stateMachine->changeState(state, transitionTime);
    }

    InputManager::KeyState getKeyState(sf::Keyboard::Key k)
    {
        return _inputManager->getKeyState(k);
    }
    bool isPressed(sf::Keyboard::Key k)
    {
        return getKeyState(k) == InputManager::Pressed;
    }
    bool isReleased(sf::Keyboard::Key k)
    {
        return getKeyState(k) == InputManager::Released;
    }
    bool isHeld(sf::Keyboard::Key k)
    {
        return getKeyState(k) == InputManager::Held;
    }
    bool isIdle(sf::Keyboard::Key k)
    {
        return getKeyState(k) == InputManager::Idle;
    }
    bool isPressedOrHeld(sf::Keyboard::Key k)
    {
        return isPressed(k) || isHeld(k);
    }
};

inline bool StateMachine::update()
{
    if (_pendingState != _currentState)
    {
        double percent = ((double)_clock.getElapsedTime().asMilliseconds() - _startTime) / _transitionTime;
        if (_enableFade)
        {
            const double alpha = (float)255 * percent;
            _fade.setFillColor(sf::Color(0, 0, 0, alpha));
            _fade.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
            Game::window().draw(_fade);
        }
        if (_startTime + _transitionTime < _clock.getElapsedTime().asMilliseconds())
        {
            Game::window().clear();
            _states.at(_currentState)->stop();
            _states.at(_pendingState)->init();
            _currentState = _pendingState;
        }
    }
    _states.at(_currentState)->update();
    if (_shouldStop)
    {
        _states.at(_currentState)->stop();
        return false;
    }
    return true;
}

template <typename T, typename U> sf::Vector2<T> vto(const sf::Vector2<U> &other)
{
    return sf::Vector2<T>(other.x, other.y);
}

inline sf::Vector2i InputManager::getMousePosPixel(const sf::View &view = Game::window().getView())
{
    return Game::window().mapCoordsToPixel(vto<float, int>(getMousePos()), view);
}

}; // namespace engine

namespace ImGui {

    class BeginLock {
    private:
        const std::string _tag;
    public:
        template<typename ...Args>
        BeginLock(const std::string& tag, Args&& ...args) : _tag(tag)
        {
            ImGui::Begin(_tag.c_str(), std::forward<Args>(args)...);
        }

        ~BeginLock()
        {
            ImGui::End();
        }
    };

    class BeginChildLock {
    private:
        const std::string _tag;
    public:
        template<typename ...Args>
        BeginChildLock(const std::string& tag, Args&& ...args) : _tag(tag)
        {
            ImGui::BeginChild(_tag.c_str(), std::forward<Args>(args)...);
        }

        ~BeginChildLock()
        {
            ImGui::EndChild();
        }
    };

    template<typename F>
    static inline void EngineMenuBar(const F& f)
    {
        if (ImGui::BeginMenuBar() == false)
            return;
        f();
        ImGui::EndMenuBar();
    }

    template<typename F>
    static inline void EngineMenu(const std::string& tag, const F& f, bool enabled=true)
    {
        if (ImGui::BeginMenu(tag.c_str(), enabled) == false)
            return;
        f();
        ImGui::EndMenu();
    }

}
