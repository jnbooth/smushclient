#include "serverstatus.h"
#include "ui_serverstatus.h"
#include <QtCore/QDateTime>

using std::reverse_iterator;
using std::strong_ordering;
using std::vector;

constexpr int iconSize = 32;

// Private utils

enum class KnownVariable
{
  Unknown,

  // ABOUT
  Icon, // URL to icon image 32x32 or larger
  Name,
  Created, // Year the MUD was created
  Description,
  Website, // URL
  Discord, // URL
  Contact, // Email address

  // STATS
  Players, // Number of online players
  Uptime,  // Epoch seconds of server start time
  Hostname,
  IP,
  IPv6,
  Port,
  SSL,      // SSL port
  Location, // Server location in English short name code (ISO 3166)

  // DETAILS
  Codebase,         // e.g. Merc 2.1
  Family,           // Codebase family
  Status,           // Alpha, Closed Beta, Open Beta, Live
  Language,         // English, Spanish, etc.
  Gameplay,         // Adventure, Educational, Haack and Slash, etc.
  Genre,            // Adult, Fantasy, Historical, etc.
  Subgenre,         // Alternate History, Anime, Cyberpunk, etc.
  WorldOriginality, // All Stock, Mostly Stock, Mostly Original, All Original
  GameSystem,       // D&D, d20 System, World of Darkness, etc.
  EquipmentSystem,  // None, Level, Skill, Both
  QuestSystem,      // None, Immortal Run, Automated, Integrated
  TrainingSystem,   // None, Level, Skill, Both

  // POLICIES
  Multiplaying,  // None, Restricted, Full
  Playerkilling, // None, Restricted, Full
  Roleplaying,   // None, Accepted, Encouraged, Enforced
  MinimumAge,    // 0 = not applicable
  AdultMaterial, // 0 or 1

  // FEATURES
  NewbieFriendly, // 0 or 1
  Multiclassing,  // 0 or 1
  PlayerCities,   // 0 or 1
  PlayerClans,    // 0 or 1
  PlayerCrafting, // 0 or 1
  PlayerGuilds,   // 0 or 1

  // COMMERCIAL
  PayToPlay,      // 0 or 1
  PayForPerks,    // 0 or 1
  HiringBuilders, // 0 or 1
  HiringCoders,   // 0 or 1

  // DATABASE
  DbSize,            // Size of the database
  Worlds,            // Number of worlds
  Areas,             // Number of areas
  Helpfiles,         // Number of helpfiles
  Mobiles,           // Number of mobiles
  Objects,           // Number of objectrs
  Rooms,             // Number of rooms
  Classes,           // Number of classes
  Levels,            // Number of levels
  Races,             // Number of races
  Skills,            // Number of skills
  Exits,             // Number of exits
  ExtraDescriptions, // Number of extra descriptions?
  MudProgs,          // Number of MudProgs
  MudTrigs,          // Number of MudTrigs
  Resets,            // Number of resets

  // SUPPORTED
  Intermud, // AberChat, I3, IMC2, MudNet, etc.
  Charset,  // ASCII, UTF-8, etc.

  // HIDDEN
  CrawlDelay, // Preferred minimum number of hours between crawls, or -1 for default
  Referral,   // Other MUD servers with MSSP support (for crawler)
};

constexpr int knownVariablesSize = (int)KnownVariable::Referral;

constexpr bool isBoolVariable(KnownVariable variable) noexcept
{
  return variable >= KnownVariable::AdultMaterial && variable <= KnownVariable::HiringCoders;
}

constexpr bool isCounterVariable(KnownVariable variable) noexcept
{
  return variable >= KnownVariable::DbSize && variable <= KnownVariable::Resets;
}

constexpr bool isHiddenVariable(KnownVariable variable) noexcept
{
  return variable >= KnownVariable::CrawlDelay;
}

QString variableName(KnownVariable variable)
{
#define KNOWN(VALUE, NAME)   \
  case KnownVariable::VALUE: \
    return QStringLiteral(NAME);

  switch (variable)
  {
    KNOWN(Unknown, "");
    KNOWN(AdultMaterial, "ADULT MATERIAL");
    KNOWN(Areas, "AREAS");
    KNOWN(Charset, "CHARSET");
    KNOWN(Classes, "CLASSES");
    KNOWN(Codebase, "CODEBASE");
    KNOWN(Contact, "CONTACT");
    KNOWN(CrawlDelay, "CRAWL DELAY");
    KNOWN(Created, "CREATED");
    KNOWN(DbSize, "DBSIZE");
    KNOWN(Description, "DESCRIPTION");
    KNOWN(Discord, "DISCORD");
    KNOWN(EquipmentSystem, "EQUIPMENT SYSTEM");
    KNOWN(Exits, "EXITS");
    KNOWN(ExtraDescriptions, "EXTRA DESCRIPTIONS");
    KNOWN(Family, "FAMILY");
    KNOWN(Gameplay, "GAMEPLAY");
    KNOWN(GameSystem, "GAMESYSTEM");
    KNOWN(Genre, "GENRE");
    KNOWN(Helpfiles, "HELPFILES");
    KNOWN(HiringBuilders, "HIRING BUILDERS");
    KNOWN(HiringCoders, "HIRING CODERS");
    KNOWN(Hostname, "HOSTNAME");
    KNOWN(Icon, "ICON");
    KNOWN(Intermud, "INTERMUD");
    KNOWN(IP, "IP");
    KNOWN(IPv6, "IPV6");
    KNOWN(Language, "LANGUAGE");
    KNOWN(Levels, "LEVELS");
    KNOWN(Location, "LOCATION");
    KNOWN(MinimumAge, "MINIMUM AGE");
    KNOWN(Mobiles, "MOBILES");
    KNOWN(MudProgs, "MUDPROGS");
    KNOWN(MudTrigs, "MUDTRIGS");
    KNOWN(Multiclassing, "MULTICLASSING");
    KNOWN(Multiplaying, "MULTIPLAYING");
    KNOWN(Name, "NAME");
    KNOWN(NewbieFriendly, "NEWBIE FRIENDLY");
    KNOWN(Objects, "OBJECTS");
    KNOWN(PayForPerks, "PAY FOR PERKS");
    KNOWN(PayToPlay, "PAY TO PLAY");
    KNOWN(PlayerCities, "PLAYER CITIES");
    KNOWN(PlayerClans, "PLAYER CLANS");
    KNOWN(PlayerCrafting, "PLAYER CRAFTING");
    KNOWN(PlayerGuilds, "PLAYER GUILDS");
    KNOWN(Playerkilling, "PLAYERKILLING");
    KNOWN(Players, "PLAYERS");
    KNOWN(Port, "PORT");
    KNOWN(QuestSystem, "QUEST SYSTEM");
    KNOWN(Races, "RACES");
    KNOWN(Referral, "REFERRAL");
    KNOWN(Resets, "RESETS");
    KNOWN(Roleplaying, "ROLEPLAYING");
    KNOWN(Rooms, "ROOMS");
    KNOWN(Skills, "SKILLS");
    KNOWN(SSL, "SSL");
    KNOWN(Status, "STATUS");
    KNOWN(Subgenre, "SUBGENRE");
    KNOWN(TrainingSystem, "TRAINING SYSTEM");
    KNOWN(Uptime, "UPTIME");
    KNOWN(Website, "WEBSITE");
    KNOWN(WorldOriginality, "WORLD ORIGINALITY");
    KNOWN(Worlds, "WORLDS");
  }

#undef KNOWN
}

QHash<QString, KnownVariable> buildKnownVariables()
{
  QHash<QString, KnownVariable> variables;
  variables.reserve(knownVariablesSize);
  for (int i = 1; i <= knownVariablesSize; ++i)
  {
    const KnownVariable variable = (KnownVariable)i;
    variables.insert(variableName(variable), variable);
  }
  return variables;
}

static const QHash<QString, KnownVariable> knownVariables = buildKnownVariables();

struct StatusEntry
{
  bool isUnknown;
  KnownVariable variable;
  QString key;
  QString value;

  StatusEntry(KnownVariable variable, const QString &key, const QString &value)
      : isUnknown(variable == KnownVariable::Unknown),
        variable(variable),
        key(key),
        value(value) {}

  strong_ordering operator<=>(const StatusEntry &other) const = default;
};

// Public methods

ServerStatus::ServerStatus(const QHash<QString, QString> &status, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ServerStatus)
{
  ui->setupUi(this);
  variableFont.setBold(true);
  QWidget *area = ui->scrollAreaWidgetContents;
  vector<StatusEntry> entries;
  entries.reserve(status.size());
  QStringList supported;
  QStringList unsupported;
  for (auto [key, value] : status.asKeyValueRange())
  {
    if (value.isEmpty())
      continue;

    const KnownVariable variable = knownVariables[key];

    if (isHiddenVariable(variable) || (isCounterVariable(variable) && value == QStringLiteral("0")))
      continue;

    switch (variable)
    {
    case KnownVariable::Unknown:
      if (value == QStringLiteral("0"))
      {
        unsupported.push_back(key);
        continue;
      }
      if (value == QStringLiteral("1"))
      {
        supported.push_back(key);
        continue;
      }
      break;

    case KnownVariable::Icon:
      if (QUrl url(value); url.isValid())
      {
        icon = new QLabel(area);
        icon->setFixedSize(iconSize, iconSize);
        ui->form->addWidget(icon);
        downloader.get(QNetworkRequest(url));
      }
      continue;

    case KnownVariable::EquipmentSystem:
    case KnownVariable::TrainingSystem:
      if (value != QStringLiteral("Both"))
        break;
      entries.emplace_back(variable, key, QStringLiteral("Skill\x02Level"));
      continue;

    case KnownVariable::Worlds:
      if (value == QStringLiteral("1"))
        continue;
      break;

    default:
      break;
    }
    entries.emplace_back(variable, key, value);
  }

  std::sort(entries.begin(), entries.end());

  for (const StatusEntry &entry : entries)
  {
    QLabel *label = variableLabel(entry.variable, entry.key, area);
    if (!entry.value.contains(QChar(2)))
    {
      ui->form->addRow(label, valueLabel(entry.variable, entry.value, area));
      continue;
    }
    const QStringList values = entry.value.split(QChar(2));
    for (auto iter = values.crbegin(), end = values.crend(); iter < end; ++iter)
    {
      ui->form->addRow(label, valueLabel(entry.variable, *iter, area));
      label = nullptr;
    }
  }

  if (!supported.isEmpty())
  {
    QLabel *label = variableLabel(tr("Supports"), area);
    for (const QString &item : supported)
    {
      ui->form->addRow(label, valueLabel(item, area));
      label = nullptr;
    }
  }

  if (!unsupported.isEmpty())
  {
    QLabel *label = variableLabel(tr("Does Not Support"), area);
    for (const QString &item : unsupported)
    {
      ui->form->addRow(label, valueLabel(item, area));
      label = nullptr;
    }
  }

  const int formHeight = ui->form->sizeHint().height();
  if (height() > formHeight)
    setMaximumHeight(formHeight + 5);
}

ServerStatus::~ServerStatus()
{
  delete ui;
}

// Private methods

QLabel *ServerStatus::variableLabel(KnownVariable variable, const QString &var, QWidget *parent) const
{
  return variableLabel(translateVariable(variable, var), parent);
}

QLabel *ServerStatus::variableLabel(const QString &text, QWidget *parent) const
{
  QLabel *label = new QLabel(text, parent);
  label->setFont(variableFont);
  return label;
}

QLabel *ServerStatus::valueLabel(const QString &text, QWidget *parent) const
{
  QLabel *label = new QLabel(text, parent);
  label->setFont(valueFont);
  label->setTextInteractionFlags(Qt::TextInteractionFlag::TextBrowserInteraction);
  label->setOpenExternalLinks(true);
  return label;
}

QLabel *ServerStatus::valueLabel(KnownVariable variable, const QString &value, QWidget *parent) const
{
  const static QString no = tr("No");
  const static QString none = tr("None");
  const static QString yes = tr("Yes");

  if (isBoolVariable(variable))
  {
    if (value == QStringLiteral("0"))
      return valueLabel(no, parent);
    if (value == QStringLiteral("1"))
      return valueLabel(yes, parent);
    return valueLabel(value, parent);
  }

  switch (variable)
  {
  case KnownVariable::Uptime:
    return valueLabel(QDateTime::fromSecsSinceEpoch(value.toLongLong()).toString(), parent);

  case KnownVariable::Contact:
    return valueLabel(QStringLiteral("<a href=\"mailto:%1\">%1</a>").arg(value), parent);

  case KnownVariable::MinimumAge:
    if (value == QStringLiteral("0"))
      return valueLabel(none, parent);
    break;

  case KnownVariable::Discord:
  case KnownVariable::Website:
    return valueLabel(QStringLiteral("<a href=\"%1\">%1</a>").arg(value), parent);
  default:
    break;
  }
  return valueLabel(value, parent);
}

QString ServerStatus::translateVariable(KnownVariable variable, const QString &raw) const
{
  switch (variable)
  {
  case KnownVariable::Unknown:
    return raw;
  case KnownVariable::AdultMaterial:
    return tr("Adult Material");
  case KnownVariable::Areas:
    return tr("Areas");
  case KnownVariable::Charset:
    return tr("Character Set");
  case KnownVariable::Classes:
    return tr("Classes");
  case KnownVariable::Codebase:
    return tr("Codebase");
  case KnownVariable::Contact:
    return tr("Contact");
  case KnownVariable::CrawlDelay:
    return tr("CrawlDelay");
  case KnownVariable::Created:
    return tr("Created");
  case KnownVariable::DbSize:
    return tr("Database Size");
  case KnownVariable::Description:
    return tr("Description");
  case KnownVariable::Discord:
    return tr("Discord Server");
  case KnownVariable::EquipmentSystem:
    return tr("Equipment System");
  case KnownVariable::Exits:
    return tr("Exits");
  case KnownVariable::ExtraDescriptions:
    return tr("Extra Descriptions");
  case KnownVariable::Family:
    return tr("Codebase Family");
  case KnownVariable::Gameplay:
    return tr("Gameplay");
  case KnownVariable::GameSystem:
    return tr("Game System");
  case KnownVariable::Genre:
    return tr("Genre");
  case KnownVariable::Helpfiles:
    return tr("Helpfiles");
  case KnownVariable::HiringBuilders:
    return tr("Hiring Builders");
  case KnownVariable::HiringCoders:
    return tr("Hiring Coders");
  case KnownVariable::Hostname:
    return tr("Hostname");
  case KnownVariable::Icon:
    return tr("Icon");
  case KnownVariable::Intermud:
    return tr("InterMUD");
  case KnownVariable::IP:
    return tr("IP Address");
  case KnownVariable::IPv6:
    return tr("IPv6 Address");
  case KnownVariable::Language:
    return tr("Language");
  case KnownVariable::Levels:
    return tr("Levels");
  case KnownVariable::Location:
    return tr("Server Location");
  case KnownVariable::MinimumAge:
    return tr("Minimum Age");
  case KnownVariable::Mobiles:
    return tr("Mobiles");
  case KnownVariable::MudProgs:
    return tr("MUD Programs");
  case KnownVariable::MudTrigs:
    return tr("MUD Triggers");
  case KnownVariable::Multiclassing:
    return tr("Multiclassing");
  case KnownVariable::Multiplaying:
    return tr("Multiplaying");
  case KnownVariable::Name:
    return tr("Name");
  case KnownVariable::NewbieFriendly:
    return tr("Newbie Friendly");
  case KnownVariable::Objects:
    return tr("Objects");
  case KnownVariable::PayForPerks:
    return tr("Pay for Perks");
  case KnownVariable::PayToPlay:
    return tr("Pay to Play");
  case KnownVariable::PlayerCities:
    return tr("Player Cities");
  case KnownVariable::PlayerClans:
    return tr("Player Clans");
  case KnownVariable::PlayerCrafting:
    return tr("Player Crafting");
  case KnownVariable::PlayerGuilds:
    return tr("Player Guilds");
  case KnownVariable::Playerkilling:
    return tr("Playerkilling");
  case KnownVariable::Players:
    return tr("Connected Players");
  case KnownVariable::Port:
    return tr("Port");
  case KnownVariable::QuestSystem:
    return tr("Quest System");
  case KnownVariable::Races:
    return tr("Races");
  case KnownVariable::Referral:
    return tr("Related Servers");
  case KnownVariable::Resets:
    return tr("Resets");
  case KnownVariable::Roleplaying:
    return tr("Roleplaying");
  case KnownVariable::Rooms:
    return tr("Rooms");
  case KnownVariable::Skills:
    return tr("Skills");
  case KnownVariable::SSL:
    return tr("Encrypted Port");
  case KnownVariable::Status:
    return tr("Game Status");
  case KnownVariable::Subgenre:
    return tr("Subgenre");
  case KnownVariable::TrainingSystem:
    return tr("Training System");
  case KnownVariable::Uptime:
    return tr("Started");
  case KnownVariable::Website:
    return tr("Website");
  case KnownVariable::WorldOriginality:
    return tr("World Originality");
  case KnownVariable::Worlds:
    return tr("Worlds");
  }
}

// Private slots

void ServerStatus::displayImage(QNetworkReply *reply)
{
  if (!icon)
    return;
  icon->setPixmap(QPixmap::fromImage(QImage::fromData(reply->readAll()).scaledToHeight(iconSize)));
}
