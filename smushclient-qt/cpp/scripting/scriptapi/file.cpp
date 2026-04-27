#include "../../ui/dialog/regexdialog.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include "sqlite3.h"
#include <QtCore/QDir>

using std::string_view;

// Public static methods

bool
ScriptApi::ChangeDir(const QString& dir)
{
  return QDir::setCurrent(dir);
}

// Public methods

int
ScriptApi::DatabaseClose(string_view databaseID)
{
  auto search = databases.find(databaseID);
  if (search == databases.end()) [[unlikely]] {
    return DatabaseConnection::Error::IdNotFound;
  }

  const int result = search->second.close();
  databases.erase(search);
  return result;
}

int
ScriptApi::DatabaseOpen(string_view databaseID, string_view filename, int flags)
{
  auto [entry, inserted] = databases.emplace(databaseID, filename);
  DatabaseConnection& db = entry->second;
  if (!inserted) {
    return db.isFile(databaseID)
             ? SQLITE_OK
             : DatabaseConnection::Error::DatabaseAlreadyExists;
  }

  const int result = db.open(flags);
  if (result != SQLITE_OK) {
    databases.erase(entry->first);
  }

  return result;
}

QString
ScriptApi::ExportXML(size_t plugin,
                     ExportKind kind,
                     string_view name) const noexcept
{
  try {
    return client.tryExportXml(kind, plugin, name);
  } catch (const rust::Error& error) {
    return QString::fromUtf8(error.what());
  }
}

int64_t
ScriptApi::ImportXML(string_view xml) const noexcept
{
  const ParseResult result = client.importXml(xml);
  RegexDialog::handle(result, parentWidget());
  return result.code;
}

bool
ScriptApi::Save(const QString& path, bool replace)
{
  return !(path.isEmpty() ? tab.saveWorldAsNew(path, !replace)
                          : tab.saveWorld())
            .isEmpty();
}

void
ScriptApi::SetChanged(bool changed) const
{
  tab.setWindowModified(changed);
}
