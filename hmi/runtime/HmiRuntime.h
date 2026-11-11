#pragma once
// HmiRuntime — boots one display process: configuration, platform services, the
// single-source translator, QML singletons (import CIVI.Platform), and placement of
// the root window on its Screen display.
namespace civi {
int runDisplay(int argc, char **argv, const char *role, const char *mainQml);
}
