#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"
#include <QtWidgets/QColorDialog>

using std::string_view;

// Public static methods

QColor
ScriptApi::AdjustColour(const QColor& color, ColorAdjust method)
{
  if (!color.isValid()) {
    return color;
  }
  switch (method) {
    case ColorAdjust::Invert: {
      int r, g, b, a;
      color.getRgb(&r, &g, &b, &a);
      return QColor::fromRgb(255 - r, 255 - g, 255 - b, a);
    }
    case ColorAdjust::Lighter: {
      int h, s, l, a;
      color.getHsl(&h, &s, &l, &a);
      return QColor::fromHsl(h, s, std::min(l + 5, 255), a);
    }
    case ColorAdjust::Darker: {
      int h, s, l, a;
      color.getHsl(&h, &s, &l, &a);
      return QColor::fromHsl(h, s, std::max(l - 5, 0), a);
    }
    case ColorAdjust::Desaturate: {
      int h, s, l, a;
      color.getHsl(&h, &s, &l, &a);
      return QColor::fromHsl(h, std::max(s - 13, 0), l, a);
    }
    case ColorAdjust::Saturate: {
      int h, s, l, a;
      color.getHsl(&h, &s, &l, &a);
      return QColor::fromHsl(h, std::min(s + 13, 255), l, a);
    }
  }
}

QColor
ScriptApi::GetSysColor(SysColor sysColor)
{
  const QPalette palette = QGuiApplication::palette();
  switch (sysColor) {
    case SysColor::Background:
      return palette.color(QPalette::ColorRole::Base);
    case SysColor::Window:
      return palette.color(QPalette::ColorRole::Window);
    case SysColor::WindowText:
      return palette.color(QPalette::ColorRole::WindowText);
    case SysColor::ButtonFace:
      return palette.color(QPalette::ColorRole::Button);
    case SysColor::ButtonShadow:
      return palette.color(QPalette::ColorRole::Shadow);
    case SysColor::ButtonText:
      return palette.color(QPalette::ColorRole::ButtonText);
    case SysColor::ButtonHighlight:
      return palette.color(QPalette::ColorRole::BrightText);
    case SysColor::Dark:
      return palette.color(QPalette::ColorRole::Dark);
    case SysColor::Light:
      return palette.color(QPalette::Light);
    default:
      return QColor();
  }
}

// Public methods

QColor
ScriptApi::GetMapColour(const QColor& color) const noexcept
{
  return client.getMappedColor(color);
}

QList<QPair<QColor, QColor>>
ScriptApi::MapColourList() const noexcept
{
  return client.colorMap();
}

QColor
ScriptApi::GetTermColour(uint8_t i) const noexcept
{
  return client.getTermColor(i);
}

void
ScriptApi::MapColour(const QColor& color, const QColor& mapped) const noexcept
{
  client.setMappedColor(color, mapped);
}

QColor
ScriptApi::PickColour(const QColor& hint) const
{
  return QColorDialog::getColor(hint, &tab);
}

QColor
ScriptApi::SetBackgroundColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Base);
  if (color == Qt::GlobalColor::black) {
    palette.setColor(QPalette::Base, Qt::GlobalColor::transparent);
  } else if (color.isValid()) {
    palette.setColor(QPalette::Base, color);
  } else {
    palette.setColor(QPalette::Base, palette.color(QPalette::AlternateBase));
  }
  background->setPalette(palette);
  return oldColor == Qt::GlobalColor::transparent ? Qt::GlobalColor::black
                                                  : oldColor;
}

QColor
ScriptApi::SetForegroundColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Text);
  palette.setColor(QPalette::Text, color);
  palette.setColor(QPalette::HighlightedText, color);
  background->setPalette(palette);
  return oldColor;
}

QColor
ScriptApi::SetHighlightColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Highlight);
  palette.setColor(QPalette::Highlight, color);
  background->setPalette(palette);
  return oldColor;
}

void
ScriptApi::SetTermColour(uint8_t i, const QColor& color) const noexcept
{
  client.setTermColor(i, color);
}
