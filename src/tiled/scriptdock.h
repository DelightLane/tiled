#pragma once

#include "mapdocument.h"

#include <QDockWidget>
#include <QTextBrowser>

namespace Tiled {

class ScriptDock : public QDockWidget
{
    Q_OBJECT

public:
    ScriptDock(QWidget *parent = nullptr);
    ~ScriptDock() override {}

    void setDocument(Document *document);

private:
    void retranslateUi();

    void currentObjectChanged(Object *object);
    void updateActions();
    QStringList ScriptDock::getEventTriggerPaths() const;

    void refreshScript();
    void openScript();
    void openGameConfig();
    void openModeConfig();
    void runGame();

    QString convertText2Html(QString In);

private:
    QAction *mRefreshScript;
    QAction *mOpenScript;
    QAction *mOpenGameConfig;
    QAction *mOpenModeConfig;
    QAction *mRunGame;

    Document* mDocument = nullptr;

    QTextBrowser* mTextBrowser;
};
}
