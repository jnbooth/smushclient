#pragma once
#include <cstdint>
#include <QString>

enum class SendTo : ::std::uint8_t;

QString encodeLink(SendTo sendto, const QString &action);

SendTo decodeLink(QString &link);
