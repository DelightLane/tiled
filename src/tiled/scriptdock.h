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
    QStringList ScriptDock::getEventTriggerPaths(QString defTriggerEvent = QString()) const;
    void changedAdditionalScipt(const QString& inChanged);

    void refreshScript();
    void openScripts();
    void openGameConfig();
    void openModeConfig();
    void runGame();

    void openScript(const QString& eventTriggerPath);
    void readScript(const QString& eventTriggerPath);

    QString convertText2Html(QString In);

private:
    QAction *mRefreshScript;
    QAction *mOpenScript;
    QAction *mOpenGameConfig;
    QAction *mOpenModeConfig;
    QAction *mRunGame;

    Document* mDocument = nullptr;

    QTextBrowser* mTextBrowser;
    QLineEdit* mLineEdit;
};
}
