#ifndef ROMBO_H
#define ROMBO_H

#include <QtGui/QMainWindow>

#include <QApplication>
#include <QAction>
#include <QSignalMapper>
#include <QSettings>

#include <QDockWidget>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>


#include "glviewer.h"
#ifdef GFXVIEW
#include "glmanager.h"
#endif


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( int argc, char *argv[] );


protected:
    void closeEvent(QCloseEvent *event);

signals:
	 void sceneFileLoaded(std::string);
	 void sceneUnloading();
	 void renderVerbosityChanged(int);
	 void renderStatusChanged(int);

private slots:
	 //menu related
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void closeScene();

    void renderPlay();	//TODO: make one for play/pause/stop
    void renderStop();
    void renderPause();
    void renderVerbose(QString);

    void about();

    //others
    void documentWasModified();
    void setRenderStatusBar(int iStatus);
    void openRecentFile();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName, bool recfilelist=true);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    GLSplitter *glSplitter; //TODO:manage delete
    GLViewer* rGLDevice;	//GL viewer
#ifdef GFXVIEW
    RenderGLView* glView;
#endif


    QDockWidget *sceneEditDockWidget;
    QPlainTextEdit *textEdit;
    QString curFile;

    QDockWidget *consoleDockWidget;
    QPlainTextEdit *outConsole;

    QSignalMapper * vMapper;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *renderMenu;
    QMenu *renderVMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *renderToolBar;

    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *closeAct;
    QAction *exitAct;

    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;

    QAction *showSceneEditAct;
    QAction *showConsoleAct;

    QAction *renderPlayAct;
    QAction *renderPauseAct;
    QAction *renderStopAct;

    QAction *aboutAct;

    QStringList files;
    QAction *separatorAct;
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];

    QSettings *settings;
};

#endif // ROMBO_H
