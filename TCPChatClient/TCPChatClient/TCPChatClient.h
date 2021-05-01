
#include <QtWidgets/QMainWindow>
#include "ui_TCPChatClient.h"

#include "Client.h"

#include <qtimer.h>

class TCPChatClient : public QMainWindow
{
    Q_OBJECT

public:
    TCPChatClient(Client* client, QWidget *parent = Q_NULLPTR);
    ~TCPChatClient();
    
    bool eventFilter(QObject* watched, QEvent* event);

    void display_new_message(const char* name, const char* message);

    void prompt_new_name();

    void disconnected();

    void update_scroll();

    void update();

    void uncheck_colors();
    void uncheck_fonts();
private slots:
    void send();
    void exit();
    void clear();
    void name();
    void red();
    void green();
    void blue();
    void magenta();
    void yellow();
    void cyan();
    void black();
    void white();
    void gray();

    void font_8();
    void font_10();
    void font_12();
    void font_14();
    void font_16();
    void font_18();
private:
    Ui::TCPChatClientClass  ui;
    QTimer                  _update_timer;
    std::string             _name;
    Client*                 _client;
};
