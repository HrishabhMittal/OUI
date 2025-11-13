#include "../src/elements.cpp"
#include <iostream>
#include <ostream>
#include <memory>
#include <string>
#include <utility>

int click_count = 0;

std::weak_ptr<Label> label_ptr;
Button* main_button_ptr = nullptr;

void onMainButtonClick(Button* btn) {
    ++click_count;
    btn->text = "Clicked!";
    btn->fg = {255, 255, 255};
    btn->bg = {255, 0, 128};

    if (!label_ptr.expired()) {
        label_ptr.lock()->text = "You've clicked " + std::to_string(click_count) +Terminal::FGColor_str({255,0,0})+ " times!";
        if (click_count % 2 == 0) {
            label_ptr.lock()->fg = {0, 255, 0};
            label_ptr.lock()->bg = {0, 0, 0};
        } else {
            label_ptr.lock()->fg = {255, 255, 0};
            label_ptr.lock()->bg = {0, 0, 255};
        }
    }
}

bool quit_requested = false;
void onQuitButtonClick(Button* btn) {
    quit_requested = true;
}
std::ostream& operator<<(std::ostream& out,const Div* d) {
    return out<<"{"<<d->r.x<<","<<d->r.y<<","<<d->r.w<<","<<d->r.h<<"}"<<std::endl;
}
int main() {
    Terminal::set_raw_mode();
    Terminal::OpenAltBuffer();
    Terminal::cursorInvisible();

    Application app(Rect{1, 1, 50, 12}, false, {255,255,255}, {0,0,0});

    auto label = std::make_shared<Label>("Welcome to Terminal UI!", Rect{1, 1, 48, 3});
    label->fg = {0, 255, 255};
    label->bg = {32, 32, 32};
    label_ptr = label;

    auto div = std::make_shared<Div>();

    auto main_button = std::make_shared<Button>("Press Me!");
    main_button->onlick = &onMainButtonClick;
    main_button->fg = {0, 0, 255};
    main_button->bg = {255, 255, 0};

    auto quit_button = std::make_shared<Button>("Quit");
    quit_button->onlick = &onQuitButtonClick;
    quit_button->fg = {255, 255, 255};
    quit_button->bg = {255, 64, 64};
    
    div->add(std::move(main_button),0.75);
    div->add(std::move(quit_button),0.25);
    app.add(std::move(label),8);
    app.add(std::move(div),4);
        
    do {
        ButtonHandler::update();
        app.render();
    } while (!quit_requested && Input::handleInput() != InputType::ENTER);
    Terminal::resetColor();
    return 0;
}
