#include "TCPChatClient.h"
#include <QtWidgets/QApplication>
#include <qdebug.h>

#include "Client.h"

#include <Windows.h>
#include <cstdlib>
#include <qmessagebox.h>

int main(int argc, char *argv[])
{
    Client client;
    auto started = client.c_startup();

    QApplication a(argc, argv);

    if(started == -1) {
        QMessageBox msg_box;
        msg_box.setText("Could not connect to server");
        msg_box.exec();
    }
    else {
        TCPChatClient w(&client);

        w.show();
        w.prompt_new_name();
        a.exec();
    }


    return 0;
}
