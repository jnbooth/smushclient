#pragma once
#include <QtCore/QFlags>

class AbstractDocument;
class AbstractTimekeeper;
class Alias;
enum class AliasOutcome;
using AliasOutcomes = QFlags<AliasOutcome>;
class SmushClientBase;
class Timer;
class Trigger;
class World;
