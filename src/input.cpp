#include <csignal>
#include <iostream>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <vector>
#include <cstdint>
#define COLOR_RED     Color(255, 0, 0)
#define COLOR_GREEN   Color(0, 255, 0)
#define COLOR_BLUE    Color(0, 0, 255)
#define COLOR_BLACK   Color(0, 0, 0)
#define COLOR_WHITE   Color(255, 255, 255)
#define COLOR_YELLOW  Color(255, 255, 0)
#define COLOR_CYAN    Color(0, 255, 255)
#define COLOR_MAGENTA Color(255, 0, 255)
#define COLOR_GRAY    Color(128, 128, 128)
struct Color {
    bool initialised=0;
    uint8_t r,g,b;
    Color(uint8_t r,uint8_t g=0,uint8_t b=0): r(r),g(g),b(b),initialised(1) {}
    Color(): initialised(0) {}
};
namespace Terminal {
    namespace {
        struct termios orig_termios;
        bool raw_mode=0,alt=0,vis=1;
    }
    std::string bold_off="\033[22m";
    std::string italic_off="\033[23m";
    std::string underline_off="\033[24m";
    std::string blink_off="\033[25m";
    std::string reverse_off="\033[27m";
    std::string hidden_off="\033[28m";
    std::string strikethrough_off="\033[29m";
    std::string bold="\033[1m";
    std::string dim="\033[2m";
    std::string italic="\033[3m";
    std::string underline="\033[4m";
    std::string blinking="\033[5m";
    std::string reverse="\033[7m";
    std::string hidden="\033[8m";
    std::string strikethrough="\033[9m";
    std::string reset="\033[0m";
    std::string cursorvis="\033[?25h";
    std::string cursorinvis="\033[?25l";
    void cursorVisible() {
        vis=1;
        std::cout<<"\033[?25h";
    }
    void cursorInvisible() {
        vis=0;
        atexit(cursorVisible);
        std::cout<<"\033[?25l";
    }
    void closeAltBuffer() {
        std::cout<<"\033[?1049l";
    }
    void OpenAltBuffer() {
        alt=1;
        atexit(closeAltBuffer);
        std::cout<<"\033[?1049h";
    }
    void FGColor(Color c) {
        std::cout << "\033[38;2;" << int(c.r) << ";" << int(c.g) << ";" << int(c.b) << "m";
    }
    std::string FGColor_str(Color c) {
        return "\033[38;2;" + std::to_string(c.r) + ";" + std::to_string(c.g) + ";" + std::to_string(c.b) + "m";
    }
    void BGColor(Color c) {
        std::cout << "\033[48;2;" << int(c.r) << ";" << int(c.g) << ";" << int(c.b) << "m";
    }
    std::string BGColor_str(Color c) {
        return "\033[48;2;" + std::to_string(c.r) + ";" + std::to_string(c.g) + ";" + std::to_string(c.b) + "m";
    }
    void resetColor() { std::cout<<"\033[0m"; }
    void unset_raw_mode() {
        std::cout << "\033[?1003l\033[?1006l" << std::flush;
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    }
    void sigint_handler(int) {
        if (raw_mode) unset_raw_mode();
        if (alt) closeAltBuffer();
        if (!vis) cursorVisible();
        std::cout << "\nExiting...\n";
        exit(0);
    }
    void set_raw_mode() {
        bool raw_mode=1;
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(unset_raw_mode);
        signal(SIGINT, sigint_handler);
    
        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        std::cout << "\033[?1003h\033[?1006h" << std::flush;
    }
    std::vector<int> getCursorPos() {
        std::cout << "\033[6n" << std::flush;
        char buf[32];
        int i = 0;
        while (i < sizeof(buf) - 1) {
            if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
            if (buf[i] == 'R') break;
            ++i;
        }
        buf[i] = '\0';
        int rows = 0, cols = 0;
        if (sscanf(buf, "\033[%d;%dR", &rows, &cols) == 2) {
            return {rows, cols};
        } else {
            return {-1, -1}; // Error
        }
    }
    void queryCursorPos(int& x, int& y) {
        std::vector<int> pos = getCursorPos();
        y = pos[0];
        x = pos[1];
    }
    void setCursorPos(int x, int y) {
        std::cout << "\033[" << y << ";" << x << "H" << std::flush;
    }
};

enum InputType {
    NONE = 0,
    MOUSE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    HOME,
    END,
    PGUP,
    PGDOWN,
    INSERT,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    BACKSPACE,
    ENTER,
    ESCAPE,
    TAB = 128,
};

namespace Input {
    namespace {
        int mouseX = 0, mouseY = 0;
        bool leftClick = false;
        bool rightClick = false;
        bool middleClick = false;
        int scroll = 0;
    }
    struct Mouse {
        int mouseX = 0, mouseY = 0;
        bool leftClick = false;
        bool rightClick = false;
        bool middleClick = false;
        int scroll = 0;
    };
    Mouse getMouseState() {
        Mouse m;
        m.mouseX=mouseX;
        m.mouseY=mouseY;
        m.leftClick=leftClick;
        m.rightClick=rightClick;
        m.middleClick=middleClick;
        m.scroll=scroll;
        return m;
    }
    int handleInput() {
        static bool handlingF = false;
        char c;
        fd_set fds;
        struct timeval tv = {0, 0};
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0)
            return NONE;
        
        if (read(STDIN_FILENO, &c, 1) != 1)
            return NONE;
    
        if (c == '\x1B') {
            struct timeval tv = {0, 20000};
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0) {
                return ESCAPE;
            }
            char seq[32];
            int i = 0;
            while (i < 31 && read(STDIN_FILENO, &seq[i], 1) == 1) {
                if ((seq[i] >= 'a' && seq[i] <= 'z') || (seq[i] >= 'A' && seq[i] <= 'Z') || seq[i] == '~' || seq[i] == 'M' || seq[i] == 'm')
                    break;
                i++;
            }
            seq[i + 1] = '\0';
            if (seq[0] == '[' && seq[1] == '<') {
                int b, x, y;
                char type;
                sscanf(seq, "[<%d;%d;%d%c", &b, &x, &y, &type);
                mouseX = x;
                mouseY = y;
    
                if (type == 'M') {
                    leftClick = (b == 0);
                    middleClick = (b == 1);
                    rightClick = (b == 2);
                    if (b==65) scroll++;
                    else if (b==64) scroll--;
                } else if (type == 'm') {
                    leftClick = middleClick = rightClick = false;
                }
    
                return MOUSE;
            }
            if (seq[0] == '[') {
                if (seq[1] == 'A') return UP;
                if (seq[1] == 'B') return DOWN;
                if (seq[1] == 'C') return RIGHT;
                if (seq[1] == 'D') return LEFT;
                if (seq[1] == 'F') return END;
                if (seq[1] == 'H') return HOME;
                if (seq[1] >= '0' && seq[2] == '~') {
                    switch (seq[1]) {
                        case '2': return INSERT;
                        case '5': return PGUP;
                        case '6': return PGDOWN;
                        case '1': return HOME;
                        case '4': return END;
                    }
                }
                if (seq[1] == '1' && seq[3] == '~') {
                    switch (seq[2]) {
                        case '5': return F5;
                        case '7': return F6;
                        case '8': return F7;
                        case '9': return F8;
                    }
                }
                if (seq[1] == '2' && seq[3] == '~') {
                    switch (seq[2]) {
                        case '0': return F9;
                        case '1': return F10;
                        case '3': return F11;
                        case '4': return F12;
                    }
                }
            } else if (seq[0] == 'O') {
                handlingF=true;
            }
    
            return NONE;
        } else if (handlingF) {
            switch (c) {
                case 'P': return F1;
                case 'Q': return F2;
                case 'R': return F3;
                case 'S': return F4;
            }
            handlingF=false;
        }
        if (isprint(c)) return c;
        if (c == '\t') return TAB;
        if (c == 0x7f || c == 0x08) return BACKSPACE;
        if (c == '\r' || c == '\n') return ENTER;
    
        return NONE;
    }
}
