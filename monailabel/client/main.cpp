#include "monailabel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Monailabel w;
    w.show();
    return a.exec();
}
