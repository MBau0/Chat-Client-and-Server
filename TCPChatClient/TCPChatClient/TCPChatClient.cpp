#include "TCPChatClient.h"

#include "Client.h"
#include "Packet.h"

#include <iostream>

#include <qplaintextedit.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qscrollbar.h>

#define MAX_MESSAGE_LENGTH 1000
#define MAX_NAME_LENGTH 50

TCPChatClient::TCPChatClient(Client* client, QWidget *parent) :
    QMainWindow     ( parent ),
    _client         ( client),
    _update_timer   ( this )
{
    _client->set_client_window(this);
    ui.setupUi(this);

    connect(ui.pushButton, &QPushButton::clicked, this, &TCPChatClient::send);
    connect(ui.actionExit, &QAction::triggered, this, &TCPChatClient::exit);
    connect(ui.actionName, &QAction::triggered, this, &TCPChatClient::name);
    connect(ui.actionRed, &QAction::triggered, this, &TCPChatClient::red);
    connect(ui.actionGreen, &QAction::triggered, this, &TCPChatClient::green);
    connect(ui.actionBlue, &QAction::triggered, this, &TCPChatClient::blue);
    connect(ui.actionMagenta, &QAction::triggered, this, &TCPChatClient::magenta);
    connect(ui.actionYellow, &QAction::triggered, this, &TCPChatClient::yellow);
    connect(ui.actionCyan, &QAction::triggered, this, &TCPChatClient::cyan);
    connect(ui.actionBlack, &QAction::triggered, this, &TCPChatClient::black);
    connect(ui.actionWhite, &QAction::triggered, this, &TCPChatClient::white);
    connect(ui.actionGray, &QAction::triggered, this, &TCPChatClient::gray);
    connect(ui.actionClear, &QAction::triggered, this, &TCPChatClient::clear);

    connect(ui.action8, &QAction::triggered, this, &TCPChatClient::font_8);
    connect(ui.action10, &QAction::triggered, this, &TCPChatClient::font_10);
    connect(ui.action12, &QAction::triggered, this, &TCPChatClient::font_12);
    connect(ui.action14, &QAction::triggered, this, &TCPChatClient::font_14);
    connect(ui.action16, &QAction::triggered, this, &TCPChatClient::font_16);
    connect(ui.action18, &QAction::triggered, this, &TCPChatClient::font_18);

    connect(ui.textBrowser->verticalScrollBar(), &QScrollBar::rangeChanged, this, &TCPChatClient::update_scroll);

    connect(&_update_timer, &QTimer::timeout, this, &TCPChatClient::update);

    _update_timer.start(1000);
    ui.textEdit->installEventFilter(this);
}

TCPChatClient::~TCPChatClient() {
    _client = nullptr;
}

void TCPChatClient::send() {
    const auto msg = ui.textEdit->document()->toPlainText().toStdString();
    if (msg.size() > MAX_MESSAGE_LENGTH) {
        ui.textEdit->clear();

        const std::string error_msg = "Message limit reached (" + std::to_string(MAX_MESSAGE_LENGTH) + ").";
        ui.textBrowser->append(error_msg.c_str());
        return;
    }

    PacketData packet("new_message", _client->get_id(), msg.c_str());

    int len = packet.length();
    _client->c_send(packet.c_str(), &len);

    ui.textEdit->clear();
}

bool TCPChatClient::eventFilter(QObject* watched, QEvent* event) {
    if(watched == ui.textEdit && event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key::Key_Return) {
            send();
            return true;
        }
    }

    return false;
}

void TCPChatClient::display_new_message(const char* name, const char* message) {
    std::string msg;

    if (ui.actionShow_Time->isChecked()) {
        auto time = QDateTime::currentDateTime();
        msg.append("[" + time.toString(Qt::LocalDate).toStdString() + "] ");
    }

    msg.append(name);
    msg.append(" : ");
    msg.append(message);

    ui.textBrowser->append(msg.c_str());
}

void TCPChatClient::prompt_new_name() {
    const std::string error_msg = "Name exceeded max length (" + std::to_string(MAX_NAME_LENGTH) + ").";
    QString name = QInputDialog::getText(this, "Input", "Enter Name");
    while(name.length() > MAX_NAME_LENGTH) {
        QMessageBox msg_box;
        msg_box.setText(error_msg.c_str());
        msg_box.exec();

        name = QInputDialog::getText(this, "Input", "Enter Name");
    }

    _name = name.toStdString();

    PacketData name_packet("set_name", _client->get_id(), _name.c_str());
    int len = name_packet.length();
    _client->c_send(name_packet.c_str(), &len);
}

void TCPChatClient::disconnected() {
    QMessageBox msg_box;
    msg_box.setText("Disconnected from server");
    msg_box.exec();
}

void TCPChatClient::update() {
    if (!_client->get_connected()) {
        if (_client->get_exit_flag() == -1) {
            QMessageBox msg_box;
            msg_box.setText("Server Disconnected");
            msg_box.exec();
            this->close();
        }
    }
}

void TCPChatClient::update_scroll() {
    ui.textBrowser->verticalScrollBar()->setValue(ui.textBrowser->verticalScrollBar()->maximum());
}

void TCPChatClient::exit() {
    this->close();
}

void TCPChatClient::name() {
    prompt_new_name();
}

void TCPChatClient::clear() {
    ui.textBrowser->clear();
}

void TCPChatClient::red() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::red);
    ui.actionRed->setChecked(true);
}

void TCPChatClient::green() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::green);
    ui.actionGreen->setChecked(true);
}

void TCPChatClient::blue() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::blue);
    ui.actionBlue->setChecked(true);
}

void TCPChatClient::magenta() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::magenta);
    ui.actionMagenta->setChecked(true);
}

void TCPChatClient::yellow() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::yellow);
    ui.actionYellow->setChecked(true);
}

void TCPChatClient::cyan() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::cyan);
    ui.actionCyan->setChecked(true);
}

void TCPChatClient::black() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::black);
    ui.actionBlack->setChecked(true);
}

void TCPChatClient::white() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::white);
    ui.actionWhite->setChecked(true);
}

void TCPChatClient::gray() {
    uncheck_colors();
    ui.textBrowser->setTextColor(Qt::gray);
    ui.actionGray->setChecked(true);
}

void TCPChatClient::font_8() {
    uncheck_fonts();
    ui.textBrowser->setFontPointSize(8);
    ui.action8->setChecked(true);
}

void TCPChatClient::font_10() {
    uncheck_fonts();
    ui.textBrowser->setFontPointSize(10);
    ui.action10->setChecked(true);
}

void TCPChatClient::font_12() {
    uncheck_fonts();
    ui.textBrowser->setFontPointSize(12);
    ui.action12->setChecked(true);
}

void TCPChatClient::font_14() {
    uncheck_fonts();
    ui.textBrowser->setFontPointSize(14);
    ui.action14->setChecked(true);
}

void TCPChatClient::font_16() {
    uncheck_fonts();
    ui.textBrowser->setFontPointSize(16);
    ui.action16->setChecked(true);
}

void TCPChatClient::font_18() {
    uncheck_fonts();
    ui.textBrowser->setFontPointSize(18);
    ui.action18->setChecked(true);
}

void TCPChatClient::uncheck_colors() {
    ui.actionRed->setChecked(false);
    ui.actionGreen->setChecked(false);
    ui.actionBlue->setChecked(false);
    ui.actionMagenta->setChecked(false);
    ui.actionYellow->setChecked(false);
    ui.actionCyan->setChecked(false);
    ui.actionBlack->setChecked(false);
    ui.actionWhite->setChecked(false);
    ui.actionGray->setChecked(false);
}

void TCPChatClient::uncheck_fonts() {
    ui.action8->setChecked(false);
    ui.action10->setChecked(false);
    ui.action12->setChecked(false);
    ui.action14->setChecked(false);
    ui.action16->setChecked(false);
    ui.action18->setChecked(false);
}