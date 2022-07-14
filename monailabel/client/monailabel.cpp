#include "monailabel.h"
#include "ui_monailabel.h"
#include <QMessageBox>
#include <QDebug>

Monailabel::Monailabel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Monailabel)
    , tmp_dir((QCoreApplication::applicationDirPath() + "/Download").toUtf8())
    , client_id("user-xyz")
{
    ui->setupUi(this);
    if (QDir tmp = QDir(tmp_dir.c_str()); !tmp.exists()) tmp.mkdir(tmp.absolutePath());
}

Monailabel::~Monailabel()
{
    delete ui;
}

//void Widget::set_value(int value)
//{
//    ui->activeLearningProgressBar->setValue(value);
//}

void Monailabel::on_nextSampleButton_clicked()
{
    if(url.toString().toStdString() == ""){
        QMessageBox::warning(NULL, "warning", "Please enter IP and port first.");
        std::cout << this->url.toString().toStdString() << std::endl;
        return;
    }
    sample = MONAILabelClient(this, url, tmp_dir, client_id).next_sample("/activelearning/" +  ui->strategyBox->currentText().toLocal8Bit(), "{\"client_id\":\"" + client_id + "\"}");
    ui->segmentationButton->setEnabled(true);

    ui->inputSelector->clear();
    ui->inputSelector->addItem(sample["id"].get<std::string>().data());
}

void Monailabel::on_fetchServerInfoButton_clicked()
{
    auto HP = ui->serverComboBox->currentText().split(':');
    if(HP.size() != 2)
        QMessageBox::warning(NULL, "warning", "Enter the correct IP address and port number.");
    else{
        url.setHost(HP[0]);
        url.setPort(HP[1].toInt());
        std::cout << this->url.toString().toStdString() << std::endl;
    }

    info = MONAILabelClient(this, url, tmp_dir, client_id).info();
    std::cout << info << std::endl;
    ui->appComboBox->addItem(info["name"].get<std::string>().data());

    ui->strategyBox->clear();
    for(auto strategy : info["strategies"].items())
        ui->strategyBox->addItem(strategy.key().data());

    ui->trainerBox->clear();
    auto trainers = info["trainers"];
    if(!trainers.empty())
        ui->trainerBox->addItem("ALL");
    for(auto t : trainers.items())
        ui->trainerBox->addItem(t.key().data());

    ui->segmentationModelSelector->clear();
    auto models = info["models"];
    for(auto model : models.items())
        if(model.value()["type"] == "segmentation")
            ui->segmentationModelSelector->addItem(model.key().data());

    ui->inputSelector->clear();
    sample.clear();
}

void Monailabel::on_segmentationButton_clicked()
{
    QString model = ui->segmentationModelSelector->currentText();
    QString image_in = ui->inputSelector->currentText();
    std::string image_file = sample["id"];
    qDebug() << image_file.c_str();
//    MONAILabelClient(this, url, tmp_dir, client_id).infer(model.toStdString(), "C:/Users/shixu/datasets/Task09_Spleen/imagesTr/spleen_10.nii.gz");
    MONAILabelClient(this, url, tmp_dir, client_id).infer(model.toStdString(), image_in.toStdString());
}

void Monailabel::on_uploadImageButton_clicked()
{
    //MONAILabelClient(this, url, tmp_dir, client_id).upload("C:\\Users\\shixu\\datasets\\Task09_Spleen\\imagesTr\\fuck.15.18).bmp","test");
    MONAILabelClient(this, url, tmp_dir, client_id).upload("C:/Users/shixu/Desktop/test.jpg", "test");
}

void Monailabel::on_trainingButton_clicked()
{
    QString model = ui->trainerBox->currentText();
    if(model == "ALL")
        model.clear();
    MONAILabelClient(this, url, tmp_dir, client_id).train_start(model.toStdString());
}

