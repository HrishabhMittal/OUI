#include "../src/elements.cpp"
#include <iostream>
#include <ostream>

void changeColor(Button* ts) {
    ts->text="clicked!";
}

int main() {
    Terminal::set_raw_mode();
    Terminal::OpenAltBuffer();
    Terminal::cursorInvisible();
    
    Application app(Rect{1, 1, 50, 10}, true, {255,255,255}, {0,0,0});
    
    auto label = std::make_shared<Label>("Welcome to Terminal UI!", Rect{1, 1, 30, 3});
    label->fg={0,255,255};
    label->bg={255,0,0};
    
    auto button = std::make_shared<Button>("Press Me!", Rect{1, 4, 15, 3});
    button->onlick=&changeColor;
    button->fg={0,0,255};
    button->bg={255,255,0};
    
    app.add(std::move(label), 30);
    app.add(std::move(button), 15);
    do {
        ButtonHandler::update();
        app.render();
    } while (Input::handleInput() != InputType::ENTER);
    Terminal::resetColor();
    return 0;
}
