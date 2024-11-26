#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace Ui
{
  class ServerStatus;
}

enum class KnownVariable;

class ServerStatus : public QDialog
{
  Q_OBJECT

public:
  explicit ServerStatus(const QHash<QString, QString> &status, QWidget *parent = nullptr);
  ~ServerStatus();

private:
  Ui::ServerStatus *ui;
  QLabel *icon;
  QNetworkAccessManager downloader;
  QFont variableFont;
  QFont valueFont;

private:
  QLabel *variableLabel(KnownVariable variable, const QString &label, QWidget *parent = nullptr) const;
  QLabel *variableLabel(const QString &label, QWidget *parent = nullptr) const;
  QLabel *valueLabel(KnownVariable variable, const QString &value, QWidget *parent = nullptr) const;
  QLabel *valueLabel(const QString &value, QWidget *parent = nullptr) const;
  QString translateVariable(KnownVariable variable, const QString &raw) const;

private slots:
  void displayImage(QNetworkReply *reply);
};
