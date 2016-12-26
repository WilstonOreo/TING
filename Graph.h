#pragma once

#include <map>
#include <vector>
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QUrl>
#include <QJsonValue>

class QThread;
class QTimer;
class QNetworkAccessManager;
class QNetworkReply;

namespace ting {
  /// Return content of file from a file name
  QString fileToStr(const QString& _filename);

  struct Node {
    Node() {}
    Node(QJsonValue const& v) {
      fromJson(v);
    }

    QString const& id() const {
      return id_;
    }

    QString const& name() const {
      return name_;
    }

    QDateTime const& createdAt() const {
      return createdAt_;
    }

    void fromJson(QJsonValue const& v);

  private:
    QString id_;
    QString name_;
    QDateTime createdAt_;
  };

  struct Link {
    Link() {}
    Link(QJsonValue const& v) {
      fromJson(v);
    }

    void fromJson(QJsonValue const& v);

  private:
    QString targetId_;
    QString sourceId_;
  };

  class Graph : public QObject {
    Q_OBJECT
  public:
    Graph(QObject* parent);
    ~Graph();

    void connect(QUrl const& _url, int _port = 80);

    void disconnect();

    void clear();

    void fromJson(QJsonObject const& v);

  signals:
    void graphChanged();

  public slots:
    void replyFinished(QNetworkReply*);

    void sendRequest();


  private:
    QByteArray generatePostData(uint64_t _since = 0) const;

    static QString readValueFromConfig(QString const& _key);

    std::map<QString,Node> nodes_;
    std::vector<Link> links_;

    /// Network manager for handling request
    QNetworkAccessManager* manager_ = nullptr;
    QTimer* timer_ = nullptr;
    QThread* thread_ = nullptr;

    int pollingIntervalInMs_ = 1000;

    QUrl url_;
    int port_ = 80;
  };
}
