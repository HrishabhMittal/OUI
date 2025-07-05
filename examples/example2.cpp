#include "../src/elements.cpp"
#include <iostream>
#include <ostream>
#include <memory>
#include <string>
#include <utility>

int click_count = 0;

Label* label_ptr = nullptr;
Button* main_button_ptr = nullptr;

void onMainButtonClick(Button* btn) {
    ++click_count;
    btn->text = "Clicked!";
    btn->fg = {255, 255, 255};
    btn->bg = {255, 0, 128};

    if (label_ptr) {
        label_ptr->text = "You've clicked " + std::to_string(click_count) + " times!";
        if (click_count % 2 == 0) {
            label_ptr->fg = {0, 255, 0};
            label_ptr->bg = {0, 0, 0};
        } else {
            label_ptr->fg = {255, 255, 0};
            label_ptr->bg = {0, 0, 255};
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

    auto label = std::make_unique<Label>("Welcome to Terminal UI!", Rect{1, 1, 48, 3});
    label->fg = {0, 255, 255};
    label->bg = {32, 32, 32};
    label_ptr = label.get();
    //std::cout<<"Label:"<<label.get();

    auto div = std::make_unique<Div>();
    //std::cout<<"div:"<<div.get();

    auto main_button = std::make_unique<Button>("Press Me!");
    main_button->onlick = &onMainButtonClick;
    main_button->fg = {0, 0, 255};
    main_button->bg = {255, 255, 0};
    //std::cout<<"main_button:"<<main_button.get();

    auto quit_button = std::make_unique<Button>("Quit");
    quit_button->onlick = &onQuitButtonClick;
    quit_button->fg = {255, 255, 255};
    quit_button->bg = {255, 64, 64};
    //std::cout<<"quit_button:"<<quit_button.get();
    
    div->add(std::move(main_button),0.75);
    div->add(std::move(quit_button),0.25);
    //std::cout<<"after pushing\n"<<"div:"<<div.get()<<"main_button:"<<div->children[0].second.get();
    //std::cout<<"quit_button:"<<div->children[1].second.get();
    app.add(std::move(label),8);
    app.add(std::move(div),4);
    //std::cout<<"after pushing\n"<<"div:"<<app.children[1].second.get()<<"main_button:"<<app.children[1].second.get()->children[0].second.get();
    //std::cout<<"quit_button:"<<app.children[1].second.get()->children[1].second.get()<<std::flush;
        
    do {
        ButtonHandler::update();
        app.render();
    } while (!quit_requested && Input::handleInput() != InputType::ENTER);
    Terminal::resetColor();
    return 0;
}
