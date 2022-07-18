#ifndef MONAILABEL_H
#define MONAILABEL_H

#include <QWidget>
#include <QUrl>
#include <QDir>
#include "monailabelclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Monailabel; }
QT_END_NAMESPACE

class Monailabel : public QWidget
{
    Q_OBJECT

public:
    Monailabel(QWidget *parent = nullptr);
    ~Monailabel();

//    void set_value(int);

private slots:
    void on_nextSampleButton_clicked();

    void on_fetchServerInfoButton_clicked();

//    void on_activeLearningProgressBar_valueChanged(int value);

    void on_segmentationButton_clicked();

    void on_uploadImageButton_clicked();

    void on_trainingButton_clicked();

private:
    Ui::Monailabel *ui;

    QUrl url;
    std::string tmp_dir;
    std::string client_id;

    json info;
    json sample;
};
#endif // MONAILABEL_H
