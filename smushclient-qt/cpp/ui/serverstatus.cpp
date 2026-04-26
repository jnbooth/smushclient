#include "serverstatus.h"
#include "ui_serverstatus.h"
#include <QtCore/QDateTime>
#include <algorithm>

using Qt::StringLiterals::operator""_L1;
using Qt::StringLiterals::operator""_s;

constexpr int iconSize = 32;

using KnownVariable = ServerStatus::KnownVariable;

// Private utils

namespace {
constexpr bool
isBoolVariable(KnownVariable variable) noexcept
{
  return variable >= KnownVariable::AdultMaterial &&
         variable <= KnownVariable::HiringCoders;
}

constexpr bool
isCounterVariable(KnownVariable variable) noexcept
{
  return variable >= KnownVariable::DbSize && variable <= KnownVariable::Resets;
}

constexpr bool
isHiddenVariable(KnownVariable variable) noexcept
{
  return variable >= KnownVariable::CrawlDelay;
}

const QHash<QString, KnownVariable> knownVariables{
  { u"ADULT MATERIAL"_s, KnownVariable::AdultMaterial },
  { u"AREAS"_s, KnownVariable::Areas },
  { u"CHARSET"_s, KnownVariable::Charset },
  { u"CLASSES"_s, KnownVariable::Classes },
  { u"CODEBASE"_s, KnownVariable::Codebase },
  { u"CONTACT"_s, KnownVariable::Contact },
  { u"CRAWL DELAY"_s, KnownVariable::CrawlDelay },
  { u"CREATED"_s, KnownVariable::Created },
  { u"DBSIZE"_s, KnownVariable::DbSize },
  { u"DESCRIPTION"_s, KnownVariable::Description },
  { u"DISCORD"_s, KnownVariable::Discord },
  { u"EQUIPMENT SYSTEM"_s, KnownVariable::EquipmentSystem },
  { u"EXITS"_s, KnownVariable::Exits },
  { u"EXTRA DESCRIPTIONS"_s, KnownVariable::ExtraDescriptions },
  { u"FAMILY"_s, KnownVariable::Family },
  { u"GAMEPLAY"_s, KnownVariable::Gameplay },
  { u"GAMESYSTEM"_s, KnownVariable::GameSystem },
  { u"GENRE"_s, KnownVariable::Genre },
  { u"HELPFILES"_s, KnownVariable::Helpfiles },
  { u"HIRING BUILDERS"_s, KnownVariable::HiringBuilders },
  { u"HIRING CODERS"_s, KnownVariable::HiringCoders },
  { u"HOSTNAME"_s, KnownVariable::Hostname },
  { u"ICON"_s, KnownVariable::Icon },
  { u"INTERMUD"_s, KnownVariable::Intermud },
  { u"IP"_s, KnownVariable::IP },
  { u"IPV6"_s, KnownVariable::IPv6 },
  { u"LANGUAGE"_s, KnownVariable::Language },
  { u"LEVELS"_s, KnownVariable::Levels },
  { u"LOCATION"_s, KnownVariable::Location },
  { u"MINIMUM AGE"_s, KnownVariable::MinimumAge },
  { u"MOBILES"_s, KnownVariable::Mobiles },
  { u"MUDPROGS"_s, KnownVariable::MudProgs },
  { u"MUDTRIGS"_s, KnownVariable::MudTrigs },
  { u"MULTICLASSING"_s, KnownVariable::Multiclassing },
  { u"MULTIPLAYING"_s, KnownVariable::Multiplaying },
  { u"NAME"_s, KnownVariable::Name },
  { u"NEWBIE FRIENDLY"_s, KnownVariable::NewbieFriendly },
  { u"OBJECTS"_s, KnownVariable::Objects },
  { u"PAY FOR PERKS"_s, KnownVariable::PayForPerks },
  { u"PAY TO PLAY"_s, KnownVariable::PayToPlay },
  { u"PLAYER CITIES"_s, KnownVariable::PlayerCities },
  { u"PLAYER CLANS"_s, KnownVariable::PlayerClans },
  { u"PLAYER CRAFTING"_s, KnownVariable::PlayerCrafting },
  { u"PLAYER GUILDS"_s, KnownVariable::PlayerGuilds },
  { u"PLAYERKILLING"_s, KnownVariable::Playerkilling },
  { u"PLAYERS"_s, KnownVariable::Players },
  { u"PORT"_s, KnownVariable::Port },
  { u"QUEST SYSTEM"_s, KnownVariable::QuestSystem },
  { u"RACES"_s, KnownVariable::Races },
  { u"REFERRAL"_s, KnownVariable::Referral },
  { u"RESETS"_s, KnownVariable::Resets },
  { u"ROLEPLAYING"_s, KnownVariable::Roleplaying },
  { u"ROOMS"_s, KnownVariable::Rooms },
  { u"SKILLS"_s, KnownVariable::Skills },
  { u"SSL"_s, KnownVariable::SSL },
  { u"STATUS"_s, KnownVariable::Status },
  { u"SUBGENRE"_s, KnownVariable::Subgenre },
  { u"TRAINING SYSTEM"_s, KnownVariable::TrainingSystem },
  { u"UPTIME"_s, KnownVariable::Uptime },
  { u"WEBSITE"_s, KnownVariable::Website },
  { u"WORLD ORIGINALITY"_s, KnownVariable::WorldOriginality },
  { u"WORLDS"_s, KnownVariable::Worlds },
};
} // namespace

struct StatusEntry
{
  bool isUnknown; // for ordering
  KnownVariable variable;
  QString key;
  QString value;

  StatusEntry(KnownVariable variable,
              const QString& key,
              const QString& value) noexcept
    : isUnknown(variable == KnownVariable::Unknown)
    , variable(variable)
    , key(key)
    , value(value)
  {
  }

  std::strong_ordering operator<=>(const StatusEntry& other) const noexcept =
    default;
};

// Public methods

ServerStatus::ServerStatus(const QHash<QString, QString>& status,
                           QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::ServerStatus)
{
  ui->setupUi(this);
  variableFont.setBold(true);
  QWidget* area = ui->scrollAreaWidgetContents;
  std::vector<StatusEntry> entries;
  entries.reserve(status.size());
  QStringList supported;
  QStringList unsupported;
  for (auto [key, value] : status.asKeyValueRange()) {
    if (value.isEmpty()) {
      continue;
    }

    const KnownVariable variable = knownVariables[key];

    if (isHiddenVariable(variable) ||
        (isCounterVariable(variable) && value == "-1"_L1)) {
      continue;
    }

    switch (variable) {
      case KnownVariable::Unknown:
        if (value == "0"_L1) {
          unsupported.push_back(key);
          continue;
        }
        if (value == "1"_L1) {
          supported.push_back(key);
          continue;
        }
        break;

      case KnownVariable::Icon:
        if (QUrl url(value); url.isValid()) {
          icon = new QLabel(area);
          icon->setFixedSize(iconSize, iconSize);
          ui->form->addWidget(icon);
          downloader.get(QNetworkRequest(url));
        }
        continue;

      case KnownVariable::EquipmentSystem:
      case KnownVariable::TrainingSystem:
        if (value != "Both"_L1) {
          break;
        }
        entries.emplace_back(variable, key, "Skill\x02Level"_L1);
        continue;

      case KnownVariable::Worlds:
        if (value == "1"_L1) {
          continue;
        }
        break;

      default:
        break;
    }
    entries.emplace_back(variable, key, value);
  }

  std::ranges::sort(entries);

  for (const StatusEntry& entry : entries) {
    QLabel* label = variableLabel(entry.variable, entry.key, area);
    if (!entry.value.contains(u'\x02')) {
      ui->form->addRow(label, valueLabel(entry.variable, entry.value, area));
      continue;
    }
    const QStringList values = entry.value.split(u'\x02');
    for (const QString& text : values) {
      ui->form->addRow(label, valueLabel(entry.variable, text, area));
      label = nullptr;
    }
  }

  if (!supported.isEmpty()) {
    QLabel* label = variableLabel(tr("Supports"), area);
    for (const QString& item : supported) {
      ui->form->addRow(label, valueLabel(item, area));
      label = nullptr;
    }
  }

  if (!unsupported.isEmpty()) {
    QLabel* label = variableLabel(tr("Does Not Support"), area);
    for (const QString& item : unsupported) {
      ui->form->addRow(label, valueLabel(item, area));
      label = nullptr;
    }
  }

  const int formHeight = ui->form->sizeHint().height();
  if (height() > formHeight) {
    setMaximumHeight(formHeight + 5);
  }
}

ServerStatus::~ServerStatus()
{
  delete ui;
}

// Private static methods

QString
ServerStatus::translateVariable(KnownVariable variable, const QString& raw)
{
  switch (variable) {
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

// Private methods

QLabel*
ServerStatus::variableLabel(KnownVariable variable,
                            const QString& text,
                            QWidget* parent) const
{
  return variableLabel(translateVariable(variable, text), parent);
}

QLabel*
ServerStatus::variableLabel(const QString& text, QWidget* parent) const
{
  QLabel* label = new QLabel(text, parent);
  label->setFont(variableFont);
  return label;
}

QLabel*
ServerStatus::valueLabel(const QString& text, QWidget* parent) const
{
  QLabel* label = new QLabel(text, parent);
  label->setFont(valueFont);
  label->setTextInteractionFlags(
    Qt::TextInteractionFlag::TextBrowserInteraction);
  label->setOpenExternalLinks(true);
  return label;
}

QLabel*
ServerStatus::valueLabel(KnownVariable variable,
                         const QString& text,
                         QWidget* parent) const
{
  if (isBoolVariable(variable)) {
    if (text == "0"_L1) {
      return valueLabel(tr("No"), parent);
    }
    if (text == "1"_L1) {
      return valueLabel(tr("Yes"), parent);
    }
    return valueLabel(text, parent);
  }

  switch (variable) {
    case KnownVariable::Uptime:
      return valueLabel(
        QDateTime::fromSecsSinceEpoch(text.toLongLong()).toString(), parent);

    case KnownVariable::Contact:
      return valueLabel("<a href=\"mailto:%1\">%1</a>"_L1.arg(text), parent);

    case KnownVariable::MinimumAge:
      if (text == "0"_L1) {
        return valueLabel(text, parent);
      }
      break;

    case KnownVariable::Discord:
    case KnownVariable::Website:
      return valueLabel("<a href=\"%1\">%1</a>"_L1.arg(text), parent);
    default:
      break;
  }
  return valueLabel(text, parent);
}

// Private slots

void
ServerStatus::displayImage(QNetworkReply* reply)
{
  if (icon == nullptr) {
    return;
  }
  icon->setPixmap(QPixmap::fromImage(
    QImage::fromData(reply->readAll()).scaledToHeight(iconSize)));
}
