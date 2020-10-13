#include "scriptdock.h"
#include <QVBoxLayout>
#include "projectmanager.h"
#include <qtextstream.h>

using namespace Tiled;

ScriptDock::ScriptDock(QWidget *parent): QDockWidget(parent),
    mTextBrowser(new QTextBrowser)
{
    setObjectName(QLatin1String("scriptDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    QVBoxLayout *listAndToolBar = new QVBoxLayout;
    listAndToolBar->setSpacing(0);
    listAndToolBar->addWidget(mTextBrowser);

    layout->addLayout(listAndToolBar);

    setWidget(widget);

    retranslateUi();
}

void ScriptDock::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (mDocument)
        mDocument->disconnect(this);

    mDocument = document;

    if (document) {
        connect(document, &Document::currentObjectChanged,
                this, &ScriptDock::currentObjectChanged);

        connect(document, &Document::propertyAdded,
                this, &ScriptDock::updateActions);
        connect(document, &Document::propertyRemoved,
                this, &ScriptDock::updateActions);

        currentObjectChanged(document->currentObject());
    } else {
        currentObjectChanged(nullptr);
    }
}

void ScriptDock::currentObjectChanged(Object *object)
{
    auto getEventTriggerPaths = [&](const QVariant& property){
        if(!object)
            return QStringList();

        QStringList pathList;

        pathList.append(property.toString().replace(QLatin1Char('.'), QStringLiteral("/Scripts/"))+QStringLiteral(".sc"));
        pathList.append(property.toString().replace(QLatin1Char('.'), QLatin1Char('/'))+QStringLiteral(".lua"));

        return pathList;
    };
    ////////////////////////////



    if(!object)
        return;

    QVariant property = object->property(QStringLiteral("triggerEvent"));
    if(property.isNull())
        return;

    QString projectPath = ProjectManager::instance()->project().getEventFolderPath();
    QStringList scriptPathList = getEventTriggerPaths(property);

    for(auto scriptPath : scriptPathList)
    {
        QFile file(projectPath+scriptPath);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            continue;

        QTextStream in(&file);
        in.setCodec("UTF-8");
        mTextBrowser->setText(in.readAll());
        file.close();

        return;
    }
}

void ScriptDock::updateActions()
{

}

void ScriptDock::retranslateUi()
{
    setWindowTitle(tr("triggerEvent"));
}
