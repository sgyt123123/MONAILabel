#ifndef MONAILABELCLIENT_H
#define MONAILABELCLIENT_H

#include <QObject>
#include <string>
#include <QUrl>
#include <QDir>
#include <QString>
#include "json.hpp"
#include "httplib.h"
using json = nlohmann::json;

class MONAILabelClient : public QObject
{
    Q_OBJECT
public:
    explicit MONAILabelClient(QObject *parent, QUrl url, std::string tmp_dir, std::string client_id);

    void setUrl(QString);


    json info();
    json next_sample(const char* selector, std::string body);
    void download_image(const char* selector, std::string image);
    void upload(std::string path, std::string image);
    void infer(std::string model, std::string image_in, std::string label_in = "");
    void train_start(std::string model, std::string params = "");

private:

    std::pair<std::string, std::string> encode_multipart_formdata(json fields, json files);
    std::string get_content_type(std::string path);
    bool save(std::string path, std::string file);

    QUrl url;
    std::string tmp_dir;
    std::string client_id;

    httplib::Client cli;
signals:
//    void value_change(int);
};

#endif // MONAILABELCLIENT_H
