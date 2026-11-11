// Entry point shared by civi-cluster / civi-center / civi-passenger / civi-hud.
#include "HmiRuntime.h"
#include <QtQml/qqmlextensionplugin.h>

Q_IMPORT_QML_PLUGIN(CIVI_ControlsPlugin)

int main(int argc, char **argv)
{
    return civi::runDisplay(argc, argv, CIVI_ROLE, CIVI_MAIN_QML);
}
