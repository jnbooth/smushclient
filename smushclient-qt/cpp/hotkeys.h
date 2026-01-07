#pragma once
#include <QtCore/QString>

class World;

struct Keypad
{
  QString key0;
  QString key1;
  QString key2;
  QString key3;
  QString key4;
  QString key5;
  QString key6;
  QString key7;
  QString key8;
  QString key9;
  QString keyPeriod;
  QString keySlash;
  QString keyAsterisk;
  QString keyMinus;
  QString keyPlus;

  QString get(Qt::Key key) const noexcept;
};

class Hotkeys
{
public:
  void applyWorld(const World& world);
  QString get(Qt::Key key, bool modified = false) const noexcept;

private:
  Keypad normal{};
  Keypad modified{};
};
