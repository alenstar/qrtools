#include "qrdialog.h"
#include <QApplication>

#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);
    el::Loggers::removeFlag(el::LoggingFlag::NewLineForContainer);
    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format,
                                       "%datetime %func[%fbase] %level: %msg");
    QApplication a(argc, argv);
    QrDialog w;
    w.show();

    return a.exec();
}
