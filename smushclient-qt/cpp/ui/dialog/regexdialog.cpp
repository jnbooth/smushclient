#include "regexdialog.h"
#include "smushclient_qt/src/ffi/regex.cxx.h"
#include "ui_regexdialog.h"
#include <QtGui/QFontDatabase>
#include <QtWidgets/QErrorMessage>

// static methods
bool
RegexDialog::handle(const ParseResult& parse, QWidget* parent)
{
  if (parse.code >= 0) {
    return true;
  }
  if (parse.target.isEmpty()) {
    QErrorMessage::qtHandler()->showMessage(parse.message);
    return false;
  }
  RegexDialog dialog(parse, parent);
  dialog.exec();
  return false;
}

// public methods

RegexDialog::RegexDialog(const ParseResult& parse, QWidget* parent)
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
