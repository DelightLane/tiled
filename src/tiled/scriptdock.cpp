#include "scriptdock.h"
#include <QVBoxLayout>
#include "projectmanager.h"
#include <qtextstream.h>
#include <qtoolbar.h>
#include <utils.h>
#include <qdesktopservices.h>
#include <qmessagebox.h>

using namespace Tiled;

ScriptDock::ScriptDock(QWidget *parent): QDockWidget(parent),
    mTextBrowser(new QTextBrowser)
{
    setObjectName(QLatin1String("scriptDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    mOpenScript = new QAction(this);
    mOpenScript->setEnabled(false);
    mOpenScript->setIcon(QIcon(QLatin1String(":/images/16/document-open.png")));
    connect(mOpenScript, &QAction::triggered, this, &ScriptDock::openScript);

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(Utils::smallIconSize());
    toolBar->addAction(mOpenScript);

    QVBoxLayout *listAndToolBar = new QVBoxLayout;
    listAndToolBar->setSpacing(0);
    listAndToolBar->addWidget(mTextBrowser);
    listAndToolBar->addWidget(toolBar);

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



    mOpenScript->setEnabled(false);
    mTextBrowser->setText(QStringLiteral(""));

    if(!object)
        return;

    QVariant property = object->property(QStringLiteral("triggerEvent"));
    if(property.isNull())
        return;

    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = object && object->isPartOfTileset();
    bool enabled = object && (!isTileset || editingTileset);
    mOpenScript->setEnabled(enabled);

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

void ScriptDock::openScript()
{
    auto getEventTriggerPaths = [&](){
        auto object = mDocument->currentObject();
        QVariant property = object->property(QStringLiteral("triggerEvent"));
        if(property.isNull())
            return QStringList();

        QStringList pathList;
        pathList.append(property.toString().replace(QLatin1Char('.'), QLatin1Char('/'))+QStringLiteral(".lua"));
        pathList.append(property.toString().replace(QLatin1Char('.'), QStringLiteral("/Scripts/"))+QStringLiteral(".sc"));

        return pathList;
    };
    ////////////////////////////



    QString projectPath = ProjectManager::instance()->project().getEventFolderPath();
    QStringList scriptPathList = getEventTriggerPaths();

    for(auto scriptPath : scriptPathList)
    {
        qDebug() << scriptPath;

        auto FullPath = projectPath + scriptPath;

        bool successOpen = QDesktopServices::openUrl(QUrl(FullPath));
        if(!successOpen)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Don't Have Script File"));
            msgBox.setText(tr("Do you make and open Script File?")+QStringLiteral("\n\n")+scriptPath);
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Ok);
            auto ret = msgBox.exec();
            switch (ret)
            {
            case QMessageBox::Ok:
            {
                QFile file(FullPath);
                file.open(QIODevice::WriteOnly);
                file.close();

                QDesktopServices::openUrl(QUrl(FullPath));
                break;
            }
            case QMessageBox::Cancel:
            {
                msgBox.close();
                break;
            }
            }
        }
    }
}

void ScriptDock::retranslateUi()
{
    setWindowTitle(tr("triggerEvent"));
}
