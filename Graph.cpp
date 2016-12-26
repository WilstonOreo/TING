#include "Graph.h"

#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QThread>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

namespace ting {
  /// Return content of file from a file name
  QString fileToStr(const QString& _filename)
  {
    QFile _f(_filename);
    _f.open(QIODevice::ReadOnly | QIODevice::Text);
    return _f.readAll();
  }

  void Node::fromJson(QJsonValue const& v) {
    if (!v.isObject()) return;
    auto obj = v.toObject();

    id_ = obj["id"].toString();
    name_ = obj["name"].toString();
    createdAt_ = QDateTime::fromTime_t(obj["createdAt"].toInt());
  }

  void Link::fromJson(QJsonValue const& v) {
    if (!v.isObject()) return;
    auto obj = v.toObject();
    QString targetId_ = obj["target"].toString();
    QString sourceId_ = obj["source"].toString();
  }

  Graph::Graph(QObject* parent) : QObject(parent) {}

  Graph::~Graph() {
    disconnect();
  }

  void Graph::clear() {
    nodes_.clear();
    links_.clear();
  }

  void Graph::fromJson(QJsonObject const& obj) {
      auto nodeArray = obj["nodes"].toArray();
      auto linkArray = obj["links"].toArray();

      for (auto nodeVal : nodeArray) {
        Node node(nodeVal.toObject());
        nodes_[node.id()] = node;
      }

      for (auto linkVal : linkArray) {
        links_.emplace_back(linkVal);
      }
  }

  void Graph::connect(QUrl const& _url, int _port) {
    disconnect();
    //lastRequestSend_ = QDateTime::currentDateTime().toTime_t();// QTime::currentTime().elapsed() - 5000;

    manager_ = new QNetworkAccessManager(this);
    QObject::connect(manager_, &QNetworkAccessManager::finished,
                     this, &Graph::replyFinished);

    thread_ = new QThread(this);
    timer_ = new QTimer(this);
    QObject::connect(timer_, &QTimer::timeout, this, &Graph::sendRequest);
    timer_->setInterval(pollingIntervalInMs_);
    timer_->moveToThread(thread_);
    QObject::connect(thread_, SIGNAL(started()), timer_, SLOT(start()));
    thread_->start();

    url_ = _url;
    port_ = _port;
  }

  void Graph::disconnect() {
    delete thread_;
    delete manager_;
    delete timer_;
  }

  QByteArray Graph::generatePostData(uint64_t _since) const {
    QString _token;
    QString _repo;
    _token = readValueFromConfig("token");
    _repo = readValueFromConfig("repo");
    if (_since == 0) {
      _since = readValueFromConfig("since").toULongLong();
    }
    QString _postData;
    _postData += "{";
    _postData += "\"token\" : \"" + _token + "\", ";
    _postData += "\"repo\" : \"" + _repo + "\" , ";
    _postData += "\"since\" : " + QString("%1").arg(_since) + " } ";

    qDebug() <<  QString("%1").arg(_since) << _since;
    return _postData.toUtf8();
  }

  void Graph::sendRequest() {
    if (!manager_) return; // No connection
    QNetworkRequest _request;
    _request.setUrl(url_);
    _request.setRawHeader("User-Agent", "ting-fountain"); ///@todo change

    QSslConfiguration _ssl = QSslConfiguration::defaultConfiguration();
    _ssl.setProtocol(QSsl::TlsV1_2);

    _request.setSslConfiguration(_ssl);
    _request.setHeader(QNetworkRequest::ServerHeader, "text/json");
    _request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            QVariant( QString("text/json") )
            );

    manager_->post(_request,generatePostData());
  }

  QString Graph::readValueFromConfig(QString const& _key) {
    QByteArray _fileContents = fileToStr("toto.cfg").toUtf8();
    QJsonDocument _json = QJsonDocument::fromJson(_fileContents);

    QJsonObject _jsonObject = _json.object();

    return _jsonObject.value(_key).toString();
  }

  void Graph::replyFinished(QNetworkReply* _reply) {
    QString _text(_reply->readAll());

    QJsonDocument doc = QJsonDocument::fromJson(_text.toUtf8());
    fromJson(doc.object());

    emit graphChanged();
  }
}
