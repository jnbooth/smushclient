#include "regexdialog.h"
#include "smushclient_qt/src/ffi/regex.cxx.h"
#include "ui_regexdialog.h"
#include <QtGui/QFontDatabase>

RegexDialog::RegexDialog(const RegexParse& parse, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::RegexDialog)
{
  ui->setupUi(this);
  ui->Message->setText(parse.message);
  QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  font.setPointSize(14);
  ui->Pattern->setFont(font);
  ui->Pattern->setText(parse.target);
  if (parse.offset == -1) {
    return;
  }

  ui->Offset->setText(tr("Error occurred at offset %1").arg(parse.offset));
  QTextCursor cursor = ui->Pattern->textCursor();
  QTextCharFormat highlight;
  highlight.setForeground(Qt::GlobalColor::red);
  cursor.setPosition(parse.offset);
  cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
  cursor.setCharFormat(highlight);
}

RegexDialog::~RegexDialog()
{
  delete ui;
}
