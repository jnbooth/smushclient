#include "regexdialog.h"
#include "ui_regexdialog.h"
#include <QtGui/QFontDatabase>

RegexDialog::RegexDialog(const QString& message,
                         int offset,
                         const QString& pattern,
                         QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::RegexDialog)
{
  ui->setupUi(this);
  ui->Message->setText(message);
  QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  font.setPointSize(14);
  ui->Pattern->setFont(font);
  ui->Pattern->setText(pattern);
  if (offset == -1)
    return;

  ui->Offset->setText(tr("Error occurred at offset %1").arg(offset));
  QTextCursor cursor = ui->Pattern->textCursor();
  QTextCharFormat highlight;
  highlight.setForeground(Qt::GlobalColor::red);
  cursor.setPosition(offset);
  cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
  cursor.setCharFormat(highlight);
}

RegexDialog::~RegexDialog()
{
  delete ui;
}
