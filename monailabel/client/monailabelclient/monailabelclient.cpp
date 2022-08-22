#include "monailabelclient.h"
#include "monailabel.h"
#include <QDebug>
#include <QMimeDatabase>
#include <QByteArray>
#include <QProgressDialog>

MONAILabelClient::MONAILabelClient(QObject *parent, QUrl url, std::string tmp_dir, std::string client_id)
    : QObject(parent)
    , url(url), tmp_dir(tmp_dir), client_id(client_id)
    , cli(url.host().toStdString(), url.port())
{
    cli.set_read_timeout(3600000);
//    connect(this, &MONAILabelClient::value_change, [parent](int value){
//        dynamic_cast<Widget*>(parent)->set_value(value);
//    });
}

void MONAILabelClient::setUrl(QString u)
{
    auto HP = u.split(':');
    url.setHost(HP[0]);
    url.setPort(HP[1].toInt());
    std::cout << url.toString().toStdString() << std::endl;
}

json MONAILabelClient::info()
{
    httplib::Headers headers{ {"accept","application/json"}};
    if(auto res = cli.Get("/info/", headers)){
        printf("status:%d\n",res->status);
        if(res->status == 200) {
            json Response{ json::parse(res->body) };
            return Response;
        }
    }
    else
        printf("url connect failed! error = %d\n",(int)res.error());
    return {};
}

json MONAILabelClient::next_sample(const char *selector, std::string body)
{
    httplib::Headers headers{ {"content-type","application/json"} };

    auto res = cli.Post(selector, headers, body, "application/json");
    if(res->status == 200){
        std::cout << "Request succeeded." << std::endl;
        json Response{ json::parse(res->body) };
        download_image("/datastore", Response["name"].get<std::string>().c_str());
        return Response;
    }
    else
        printf("url connect failed! error = %d\n",(int)res.error());
    return {};
}

void MONAILabelClient::download_image(const char *selector, std::string image)
{
    QProgressDialog dialog("To be continued", "Cancel", 1, 100);
    dialog.setMinimumDuration(0);
    dialog.setWindowModality(Qt::WindowModal);
    QString name = QString(image.c_str()).split('.')[0];
    if(auto res = cli.Get((QString(selector) + "/image?image=" + name).toUtf8(),
                          [&](uint64_t len, uint64_t total) {
//                          emit value_change(len*100/total);
                          dialog.show();
                          dialog.setValue(len*100/total);
                          return true;
}))
    {
        qDebug("status:%d", res->status);
        if(res->status == 200)
            save(tmp_dir + '/' + image, res->body);
        else
            qDebug("url connect failed! error = %d",(int)res.error());
    }
}

void MONAILabelClient::upload(std::string path, std::string image)
{
    auto selector = "/datastore/?image=" + image;

    json fields = json::parse("{\"params\":{\"client_id\": null}}");
    json files = json::parse("{\"file\":\"" + path + "\"}");

    auto [body, content_type] = encode_multipart_formdata(fields, files);

    httplib::Headers headers{ {"content-length", QString::number(body.size()).toStdString()} };

    auto res = cli.Put(selector.c_str(), headers, body, content_type.c_str());


    if(res->status == 200){
        qDebug() << "Request succeeded.";
        json Response{ json::parse(res->body) };
        qDebug() << Response.dump().c_str();
    }
    else
        qDebug("url connect failed! error = %d\n",(int)res.error());

}

json MONAILabelClient::infer(std::string model, std::string image_in, std::string label_in)
{

    std::string selector = "/infer/" + model + "?image=" + image_in;

    json fields = json::parse("{\"params\":{\"client_id\": null}}");
    json files = QFile::exists("label_in")?json::parse("{\"label\":\"" + label_in + "\"}"):R"({})"_json;

    std::cout << files << std::endl;

    auto [body, content_type] = encode_multipart_formdata(fields, files);

    httplib::Headers headers{{"content-type", content_type}, {"content-length", QString::number(body.size()).toStdString()}};

    qDebug() << selector.c_str();
    qDebug() << body.c_str();

    json labelpaths;

    auto res = cli.Post(selector.c_str(), headers, body, content_type.c_str());
    if(res->status == 200){
        qDebug() << "Request succeeded.";

        bool is_file = false;
        std::string formdata = res->body, name;

        std::string partition = formdata.substr(0, formdata.find("\r\n"));

        int start = 0, end = 0, sum = 0;
        for(std::string fg = res->body; ;){
            int mid = fg.find("\r\n");
            if(mid == fg.npos)
                break;

            std::string line = fg.substr(0, mid);
            qDebug() << line.c_str();


            if(int t = line.find("filename"); t != line.npos){
                name = line.substr(t);
                t = name.find("\"") + 1;
                name = name.substr(t, name.rfind("\"") - t);
            }
            else if(line == "Content-Type: application/octet-stream"){
                start = sum + mid + 4;
                is_file = true;
            }
            else if(line.find(partition) != line.npos && is_file){
                end = sum;

                //auto namepath = g_GlobalVal.FileFullPath.toStdString();
                auto labelpath = tmp_dir + '/' + label_in + "_Label.nii.gz";
                labelpaths.push_back(labelpath);
                save(labelpath, formdata.substr(start, end));

                is_file = false;
            }

            fg = fg.substr(mid + 2);
            sum += (mid + 2);
        }
        qDebug() << start << end;

    }
    else
        qDebug("url connect failed! error = %d",(int)res.error());

    return labelpaths;
}

void MONAILabelClient::train_start(std::string model, std::string params)
{
    auto selector = "/train/" + model;

    httplib::Headers headers{ {"content-length", QString::number(params.size()).toStdString()} };

    auto res = cli.Post(selector.c_str(), headers, params, "application/json");

    if(res->status == 200){
        qDebug() << "Request succeeded.";
        json Response{ json::parse(res->body) };
        qDebug() << Response.dump().c_str();
    }
    else
        qDebug("url connect failed! error = %d\n",(int)res.error());
}

void MONAILabelClient::train_stop()
{
    auto selector = "/train/";

    std::string body;

    httplib::Headers headers{ {"content-length", QString::number(body.size()).toStdString()} };

    auto res = cli.Delete(selector, headers, body, "application/json");

    if (res->status == 200) {
        qDebug() << "Request succeeded.";
        json Response{ json::parse(res->body) };
        qDebug() << Response.dump().c_str();
    }
    else
        qDebug("url connect failed! error = %d\n", (int)res.error());
}

json MONAILabelClient::train_status(bool check_if_running)
{
    std::string selector = "/train/";
    if (check_if_running)
        selector += "?check_if_running=true";

    if (auto res = cli.Get(selector.c_str())) {
        printf("status:%d\n", res->status);
        if (res->status == 200) {
            json Response{ json::parse(res->body) };
            return Response;
        }
    }
    else
        printf("url connect failed! error = %d\n", (int)res.error());

    return {};
}



void MONAILabelClient::save_label(std::string image_id, std::string label_in, std::string tag)
{
    auto selector = "/datastore/label?image=" + image_id;
    if (tag != "")
        selector += ("&tag=" + tag);

    json fields = json::parse("{\"params\":{\"client_id\": null}}");
    json files = json::parse("{\"label\":\"" + label_in + "\"}");

    auto [body, content_type] = encode_multipart_formdata(fields, files);

    httplib::Headers headers{ {"content-length", QString::number(body.size()).toStdString()} };

    auto res = cli.Put(selector.c_str(), headers, body, content_type.c_str());

    if (res->status == 200) {
        qDebug() << "Request succeeded.";
        json Response{ json::parse(res->body) };
        qDebug() << Response.dump().c_str();
    }
    else
        qDebug("url connect failed! error = %d\n", (int)res.error());
}


std::pair<std::string, std::string> MONAILabelClient::encode_multipart_formdata(json fields, json files)
{
    std::string limit = "----------lImIt_of_THE_fIle_eW_$";
    std::list<std::string> lines;

    std::string body;

    for(auto item : fields.items()){
        lines.push_back("--" + limit);
        lines.push_back("Content-Disposition: form-data; name=\"" + item.key() + "\"");
        lines.push_back("");
        lines.push_back(item.value().dump());
    }
    for(auto item : files.items()){
        lines.push_back("--" + limit);
        lines.push_back("Content-Disposition: form-data; name=\"" + item.key() + "\"; filename=\"" + item.value().get<std::string>() + "\"");
        lines.push_back("Content-Type: " + get_content_type(item.value().get<std::string>()));
        lines.push_back("");

        std::ifstream myfile(item.value().get<std::string>(), std::ios::in | std::ios::binary);
        std::stringstream CodeStrstream;
        if (!myfile.is_open())
            std::cout << "未成功打开文件" << std::endl;
        else
            CodeStrstream << myfile.rdbuf();

        std::string CodeFilestr = CodeStrstream.str();
        myfile.close();

        lines.push_back(CodeFilestr);
        lines.push_back("");
    }
    lines.push_back("--" + limit + "--");
    lines.push_back("");

    for(auto line : lines)
        body += line += "\r\n";

    qDebug() << body.c_str();
    return {body, "multipart/form-data; boundary=" + limit};
}

std::string MONAILabelClient::get_content_type(std::string path)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(QString(path.c_str()));  //根据前面定义的文件名（含后缀）
    return mime.name().toStdString();  //使用name()将MimeType类型转为字符串类型
}

bool MONAILabelClient::save(std::string path, std::string file)
{
    std::ofstream out;
    out.open(path, std::ios_base::binary | std::ios::out);
    qDebug() << "savefile!";
    qDebug() << path.c_str();

    if(out.is_open()) {
        out << file;
        out.close();
        qDebug() << "down load file finished!";
        return true;
    }
    else{
        qDebug() << "open file error!";
        return false;
    }
}
