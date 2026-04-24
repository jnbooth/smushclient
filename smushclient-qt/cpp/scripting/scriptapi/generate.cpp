#include "../scriptapi.h"
#include "smushclient_qt/src/ffi/regex.cxx.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include <QtCore/QCryptographicHash>

using std::string_view;

// Public static methods

QByteArray
ScriptApi::GetUniqueID()
{
  const QByteArray uuid =
    QUuid::createUuid().toByteArray(QUuid::StringFormat::Id128);
  QByteArray digest =
    QCryptographicHash::hash(uuid, QCryptographicHash::Algorithm::Sha1);
  digest.truncate(12);
  return digest.toHex();
}

int64_t
ScriptApi::GetUniqueNumber() noexcept
{
  static int64_t uniqueNumber = -1;
  return ++uniqueNumber;
}

QByteArray
ScriptApi::Hash(QByteArrayView bytes)
{
  return QCryptographicHash::hash(bytes, QCryptographicHash::Algorithm::Sha1)
    .toHex();
}

QString
ScriptApi::MakeRegularExpression(string_view pattern) noexcept
{
  return ffi::regex::from_wildcards(pattern);
}
