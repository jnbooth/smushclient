#pragma once
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>

namespace Ui {
class ServerStatus;
} // namespace Ui

class ServerStatus : public QDialog
{
  Q_OBJECT

public:
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
    CrawlDelay, // Preferred minimum number of hours between crawls, or -1 for
                // default
    Referral,   // Other MUD servers with MSSP support (for crawler)
  };

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
