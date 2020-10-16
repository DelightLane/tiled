#include "scriptdock.h"
#include <QVBoxLayout>
#include "projectmanager.h"
#include <qtextstream.h>
#include <qtoolbar.h>
#include <utils.h>
#include <qdesktopservices.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qdir.h>

using namespace Tiled;

ScriptDock::ScriptDock(QWidget *parent): QDockWidget(parent)
{
    setObjectName(QLatin1String("scriptDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    mRefreshScript = new QAction(this);
    mRefreshScript->setEnabled(true);
    mRefreshScript->setIcon(QIcon(QLatin1String(":/images/16/help-about.png")));
    connect(mRefreshScript, &QAction::triggered, this, &ScriptDock::refreshScript);

    mOpenScript = new QAction(this);
    mOpenScript->setEnabled(false);
    mOpenScript->setIcon(QIcon(QLatin1String(":/images/16/document-open.png")));
    connect(mOpenScript, &QAction::triggered, this, &ScriptDock::openScripts);

    mOpenGameConfig = new QAction(this);
    mOpenGameConfig->setEnabled(true);
    mOpenGameConfig->setIcon(QIcon(QLatin1String(":/images/16/document-page-setup.png")));
    connect(mOpenGameConfig, &QAction::triggered, this, &ScriptDock::openGameConfig);

    mOpenModeConfig = new QAction(this);
    mOpenModeConfig->setEnabled(true);
    mOpenModeConfig->setIcon(QIcon(QLatin1String(":/images/16/document-properties.png")));
    connect(mOpenModeConfig, &QAction::triggered, this, &ScriptDock::openModeConfig);

    mRunGame = new QAction(this);
    mRunGame->setEnabled(true);
    mRunGame->setIcon(QIcon(QLatin1String(":/images/16/simulate.png")));
    connect(mRunGame, &QAction::triggered, this, &ScriptDock::runGame);

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(Utils::smallIconSize());
    toolBar->addAction(mRefreshScript);
    toolBar->addAction(mOpenScript);
    toolBar->addAction(mOpenGameConfig);
    toolBar->addAction(mOpenModeConfig);
    toolBar->addAction(mRunGame);

    mTextBrowser = new QTextBrowser();
    mLineEdit = new QLineEdit();
    mLineEdit->setPlaceholderText(tr("load additional eventTrigger"));
    connect(mLineEdit, &QLineEdit::textChanged, this, &ScriptDock::changedAdditionalScipt);
    QVBoxLayout *listAndToolBar = new QVBoxLayout;
    listAndToolBar->setSpacing(0);
    listAndToolBar->addWidget(mTextBrowser);
    listAndToolBar->addWidget(mLineEdit);
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

void ScriptDock::changedAdditionalScipt(const QString& inChangedTriggerEvent)
{
    QStringList scriptPathList = getEventTriggerPaths(inChangedTriggerEvent);


    for(auto scriptPath : scriptPathList)
    {
        QString projectPath = ProjectManager::instance()->project().getEventFolderPath();
        auto FullPath = projectPath + scriptPath;

        QFile file(FullPath);
        if(!file.open(QIODevice::ReadOnly))
            continue; // 스크립트 없으면 그냥 무시
    }

    readScript(inChangedTriggerEvent);
}

void ScriptDock::readScript(const QString& eventTriggerPath)
{
    auto sortMethod = [](const QString& lhs, const QString& rhs){
            if(lhs.contains(QStringLiteral("sc")))
                return true;

            return false;
    };
    ////////////////////////////



    QString projectPath = ProjectManager::instance()->project().getEventFolderPath();
    QStringList scriptPathList = getEventTriggerPaths(eventTriggerPath);

    std::sort(scriptPathList.begin(), scriptPathList.end(), sortMethod);

    for(auto scriptPath : scriptPathList)
    {
        QFile file(projectPath+scriptPath);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            continue;

        QTextStream in(&file);
        in.setCodec("UTF-8");
        mTextBrowser->setHtml(convertText2Html(in.readAll()));
        file.close();

        return;
    }
}

void ScriptDock::currentObjectChanged(Object *object)
{
    auto sortMethod = [](const QString& lhs, const QString& rhs){
            if(lhs.contains(QStringLiteral("sc")))
                return true;

            return false;
    };
    ////////////////////////////



    mOpenScript->setEnabled(false);
    mTextBrowser->setHtml(QStringLiteral(""));

    if(!object)
        return;

    QVariant property = object->property(QStringLiteral("triggerEvent"));
    if(property.isNull())
        return;

    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = object && object->isPartOfTileset();
    bool enabled = object && (!isTileset || editingTileset);
    mOpenScript->setEnabled(enabled);

    readScript(QString());
}

void ScriptDock::updateActions()
{

}

QStringList ScriptDock::getEventTriggerPaths(QString defTriggerEvent/* = QString()*/) const
{
    if(defTriggerEvent.isEmpty())
    {
        auto object = mDocument->currentObject();
        QVariant property = object->property(QStringLiteral("triggerEvent"));
        if(property.isNull())
            return QStringList();

        defTriggerEvent = property.toString();
    }

    QStringList pathList;
    pathList.append(QString(defTriggerEvent).replace(QLatin1Char('.'), QLatin1Char('/'))+QStringLiteral(".lua"));
    pathList.append(QString(defTriggerEvent).replace(QLatin1Char('.'), QStringLiteral("/Scripts/"))+QStringLiteral(".sc"));

    return pathList;
};

void ScriptDock::refreshScript()
{
    currentObjectChanged(mDocument->currentObject());
}

void ScriptDock::openScript(const QString& eventTriggerPath)
{
    QString projectPath = ProjectManager::instance()->project().getEventFolderPath();
    auto FullPath = projectPath + eventTriggerPath;

    bool successOpen = QDesktopServices::openUrl(QUrl(FullPath));
    if(!successOpen)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Don't Have Script File"));
        msgBox.setText(tr("Do you make and open Script File?")+QStringLiteral("\n\n")+eventTriggerPath);
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

void ScriptDock::openScripts()
{
    QStringList scriptPathList = getEventTriggerPaths();

    for(auto scriptPath : scriptPathList)
    {
        openScript(scriptPath);
    }
}

void ScriptDock::openGameConfig()
{
    QDesktopServices::openUrl(QUrl(ProjectManager::instance()->project().getProjectFolderPath() + QStringLiteral("gameConfig.lua")));
}

void ScriptDock::openModeConfig()
{
    QDesktopServices::openUrl(QUrl(ProjectManager::instance()->project().getProjectFolderPath() + QStringLiteral("modeConfig.lua")));
}

void ScriptDock::runGame()
{
    QString arg = QStringLiteral("\"") + ProjectManager::instance()->project().getProjectFolderPath() + QStringLiteral("\"");

    QDir::setCurrent(ProjectManager::instance()->project().mCoronaSdkPath);
    QProcess::startDetached(QStringLiteral("Corona.Shell.exe ") + arg);
}

QString ScriptDock::convertText2Html(QString in)
{
    auto insertTag = [](QString in, QString fromHead, QString toHtmlTagHead, QString fromTail, QString toHtmlTagTail){
        int findIdx = in.indexOf(fromHead, 0);
        while(findIdx != -1)
        {
            bool isHead = findIdx == 0 || (in.at(findIdx - 1) == QStringLiteral(">") && in.at(findIdx - 2) == QStringLiteral("r")); // 제일 앞이거나 <br> 뒤 일때만
            if(isHead)
            {
                in = in.replace(findIdx, fromHead.size(), toHtmlTagHead);

                findIdx = in.indexOf(fromTail, findIdx);
                if(findIdx != -1)
                    in = in.replace(findIdx, fromTail.size(), toHtmlTagTail);
                else
                    in += toHtmlTagTail;
            }
            else
            {
                ++findIdx;
            }

            findIdx = in.indexOf(fromHead, findIdx);
        }

        return in;
    };



    //////////////////////////

    in = in.replace(QStringLiteral("\n"), QStringLiteral("<br>"));
    in = insertTag(in, QStringLiteral("?"), QStringLiteral("<font color=\"DodgerBlue\">?"), QStringLiteral("<br>"), QStringLiteral("</font><br>"));
    in = insertTag(in, QStringLiteral("@"), QStringLiteral("<font color=\"Tomato\">@"), QStringLiteral("<br>"), QStringLiteral("</font><br>"));
    in = insertTag(in, QStringLiteral("+"), QStringLiteral("<font color=\"Orange\">+"), QStringLiteral("<br>"), QStringLiteral("</font><br>"));

    qDebug() << in;

    return in;
}

void ScriptDock::retranslateUi()
{
    setWindowTitle(tr("triggerEvent"));
}
