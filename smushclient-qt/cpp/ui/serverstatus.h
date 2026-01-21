#pragma once
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>

namespace Ui {
class ServerStatus;
} // namespace Ui

enum class KnownVariable;

class ServerStatus : public QDialog
{
  Q_OBJECT

public:
  explicit ServerStatus(const QHash<QString, QString>& status,
                        QWidget* parent = nullptr);
  ~ServerStatus() override;

private:
  static QString translateVariable(KnownVariable variable, const QString& raw);

  QLabel* variableLabel(KnownVariable variable,
                        const QString& text,
                        QWidget* parent = nullptr) const;
  QLabel* variableLabel(const QString& text, QWidget* parent = nullptr) const;
  QLabel* valueLabel(KnownVariable variable,
                     const QString& text,
                     QWidget* parent = nullptr) const;
  QLabel* valueLabel(const QString& text, QWidget* parent = nullptr) const;

private slots:
  void displayImage(QNetworkReply* reply);

private:
  Ui::ServerStatus* ui;
  QLabel* icon = nullptr;
  QNetworkAccessManager downloader;
  QFont variableFont;
  QFont valueFont;
};
