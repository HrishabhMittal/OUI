#include "input.cpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

struct Rect {
    int x,y,w,h;
    Rect(int x=0,int y=0,int w=0,int h=0): x(x), y(y), w(w), h(h) {}
};
bool pointInRect(int px, int py, const Rect& r) {
    return px >= r.x && px < r.x + r.w &&
           py >= r.y && py < r.y + r.h;
}
bool operator==(const Rect& a,const Rect& b) {
    return (a.x==b.x&&a.y==b.y&&a.w==b.w&&a.h==b.h);
}
void printInRect(const std::string& s, Rect r,Color fg={},Color bg={}) {
    if (fg.initialised) {
        Terminal::FGColor(fg);
        Terminal::BGColor(bg);
    }
    int x=0,y=0;
    Terminal::setCursorPos(r.x, r.y);
    for (size_t idx=0;idx<s.size()&&y<r.h;++idx) {
        char c=s[idx];
        if (c=='\n'||x==r.w) {
            while (x<r.w) {
                std::cout<<' ';
                ++x;
            }
            x=0;
            ++y;
            if (y==r.h) break;
            Terminal::setCursorPos(r.x,r.y+y);
            if (c=='\n') continue;
        }
        std::cout<<c;
        ++x;
    }
    while (x<r.w&&y<r.h) {
        std::cout<<' ';
        ++x;
    }
    ++y;
    while (y<r.h) {
        Terminal::setCursorPos(r.x,r.y+y);
        for (int i=0;i<r.w;++i) std::cout<<' ';
        ++y;
    }
    Terminal::resetColor();
    std::cout << std::flush;
}

class Div {
public:
    Color fg,bg;
    Rect r;
    std::vector<std::unique_ptr<Div>> children;
    bool horizontal;
    int occupied = 0;
    Div(Rect r={}, bool h=1,Color fg={},Color bg={}) : r(r), horizontal(h),fg(fg),bg(bg) {}
    virtual void add(std::unique_ptr<Div> b, int size) {
        int maxSize = horizontal ? r.w : r.h;
        if (occupied + size > maxSize) size = maxSize - occupied;
        if (size <= 0) return;
        if (horizontal)
            b->r = Rect(r.x + occupied, r.y, size, r.h);
        else
            b->r = Rect(r.x, r.y + occupied, r.w, size);
        if (!b->fg.initialised) b->fg=fg;
        if (!b->bg.initialised) b->bg=bg;
        occupied += size;
        children.push_back(std::move(b));
    }
    virtual void render() const {
        printInRect("",r,fg,bg);
        for (auto&& i : children) i->render();
    }
};

class Application : public Div {
public:
    Application(Rect r, bool h,Color fg={},Color bg={}) : Div(r,h,fg,bg) {}
};

class Label : public Div {
public:
    std::string text;
    Label(const std::string& text, Rect r = {}, bool h = true)
        : Div(r, h), text(text) {}
    void add(std::unique_ptr<Div> b, int size) override {}
    void render() const override {
        printInRect(text,r,fg,bg);
        Terminal::resetColor();
    }
};
class Button;
namespace ButtonHandler {
    namespace {
        std::vector<Button*> b;
    }
    void add(Button* bu) {
        b.push_back(bu);
    }
};
class Button : public Div {
public:
    std::string text;
    Button(const std::string& text, Rect r = {}, bool h = true): Div(r, h), text(text) {
        ButtonHandler::add(this);
    }
    void add(std::unique_ptr<Div> b, int size) override {}
    void render() const override {
        printInRect(text,r,fg,bg);
        Terminal::resetColor();
    }
    void (*onlick)(Button*) = nullptr;
};
namespace ButtonHandler {
    void update() {
        Input::Mouse m=Input::getMouseState();
        if (m.leftClick) {
            for (auto i:b) {
                if (pointInRect(m.mouseX, m.mouseY, i->r)) i->onlick(i);
            }
        }
    }
}
