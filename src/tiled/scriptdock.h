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
    void openScript();
    void runGame();

private:
    QAction *mOpenScript;
    QAction *mRunGame;

    Document* mDocument = nullptr;

    QTextBrowser* mTextBrowser;
};
}
