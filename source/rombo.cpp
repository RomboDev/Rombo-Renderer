#include "rombo.h"

MainWindow::MainWindow (int argc, char *argv[])
{
	 // main GL renderer view
	 rGLDevice = new GLViewer (argc,argv);

	 rGLDevice->setBackgroundRole (QPalette::Dark);
	 //QPalette p( rGLDevice->palette() );
	 //p.setColor( QPalette::Window, Qt::black );
	 //rGLDevice->setPalette( p );

	 glSplitter = new GLSplitter();
	 glSplitter->setOrientation(Qt::Vertical);
	 glSplitter->setHandleWidth(2);
	 glSplitter->setOpaqueResize(false);

	 glSplitter->addWidget (rGLDevice);
	 glSplitter->addWidget (new QPlainTextEdit);

#ifndef GFXVIEW
	 rGLDevice->setMinimumSize(512,360);
	 setCentralWidget( glSplitter );	//add it to the window
#else

	 glView = new RenderGLView;
	 glView->setFocusPolicy(Qt::StrongFocus);
	 glView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	 glView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	 glView->setMinimumSize(512,360);
	 glView->setGLScene( rGLDevice );
	 //glView->setGLScenePointer( rGLDevice );
	 glView->setAlignment(Qt::AlignBottom);
	 setCentralWidget( glView );
#endif



    // add docked text editor to display/modify scene files
    textEdit = new QPlainTextEdit;
    sceneEditDockWidget = new QDockWidget(tr("Scene Editor"));
    sceneEditDockWidget->setMinimumWidth(280);
    sceneEditDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    sceneEditDockWidget->setWidget( textEdit );
    addDockWidget(Qt::RightDockWidgetArea, sceneEditDockWidget);

    // add terminal (read-only) emulator for verbosity
    outConsole = new QPlainTextEdit;
    outConsole->setReadOnly(true);

    consoleDockWidget = new QDockWidget(tr("Render console"));
    consoleDockWidget->setMinimumWidth(240);
    consoleDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    consoleDockWidget->setWidget( outConsole );
    addDockWidget(Qt::RightDockWidgetArea, consoleDockWidget);


    // rig slots and signaling
    connect(	rGLDevice, 	SIGNAL (splitterMoved (int)),
    			glSplitter, SLOT   (splitting (int)) );

    connect(	rGLDevice, 	SIGNAL (verboseStream (QString)),
   		 		outConsole, SLOT   (appendPlainText (QString)) );

    connect(	rGLDevice, 	SIGNAL (rendererStatus (int)),
   		 		this, 		SLOT   (setRenderStatusBar (int)) );

    connect(	this, 		SIGNAL (sceneFileLoaded (std::string)),
   		 		rGLDevice, 	SLOT   (parseSceneAndRender (std::string)) );

    connect(	this, 		SIGNAL (renderVerbosityChanged (int)),
   		 		rGLDevice, 	SLOT   (setRenderVerbosity (int)) );

    connect(	this, 		SIGNAL (renderStatusChanged (int)),
   		 		rGLDevice, 	SLOT   (setRenderStatus (int)) );

    connect(	this, 		SIGNAL (renderStatusChanged (int)),
   		 		this, 		SLOT   (setRenderStatusBar (int)) );

    connect(	this, 		SIGNAL (sceneUnloading ()),
   		 		rGLDevice, 	SLOT   (flushScene ()) );
#ifdef GFXVIEW
    connect(	rGLDevice, 	SIGNAL (forceresize ()),
    			glView, 	SLOT   (doresizing ()) );
#endif

    // create GUI
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    // read saved application settings
    settings = new QSettings("ctrlstudio", "rombo");
    readSettings();

    // connect just buildt actions
    connect( sceneEditDockWidget, 	SIGNAL( visibilityChanged(bool) ),
   		 	 showSceneEditAct, 		SLOT( setChecked(bool) ) );
    connect( consoleDockWidget, 	SIGNAL( visibilityChanged(bool) ),
   		 	 showConsoleAct, 		SLOT( setChecked(bool) ) );

    // setup content changed rig
    connect(textEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

    //just set 'untitled' for the opening win
    setCurrentFile("", false);
}


// Menu:File
void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile("");
    }
}

void MainWindow::open()
{
    /*if(rGLDevice==NULL)
    {
   	 int argc=1; char* argv[] = {(char*)"",(char*)""};
   	 rGLDevice = new GLViewer (argc,argv);
   	 rGLDevice->setMinimumSize(320,360);
   	 setCentralWidget( rGLDevice );	//add it to the window
    }*/
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::closeScene()
{
	 //disable render buttons
	 renderPlayAct->setEnabled(false);
	 renderPauseAct->setEnabled(false);
	 renderStopAct->setEnabled(false);

	 emit sceneUnloading();

	 rGLDevice->deleteLater();
	 //delete rGLDevice;
	 rGLDevice = NULL;
}

// Menu:render
void MainWindow::renderPlay()
{
	 emit renderStatusChanged(1);
}
void MainWindow::renderPause()
{
	 emit renderStatusChanged(0);
}
void MainWindow::renderStop()
{
	 emit renderStatusChanged(-1);
}
void MainWindow::renderVerbose(QString iV)
{
	 emit renderVerbosityChanged(iV.toInt());
}

// Menu:About
void MainWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("<b>Rombo Studio</b>\n"
               "@ ctrlstudio "));
}

// Others
void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}

void MainWindow::setRenderStatusBar(int iStatus)
{
	 switch(iStatus)
	 {
	 case 0:
    statusBar()->showMessage(tr("Renderer : stopped")); break;
	 case 1:
    statusBar()->showMessage(tr("Renderer : working")); break;
	 case 2:
    statusBar()->showMessage(tr("Renderer : paused")); break;
	 }
}

// Create GUI /////////////////////////////////////////////////////////////
void MainWindow::createActions()
{
	 // Menu ::

	 // File
    newAct = new QAction(QIcon(":/images/scene_new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/scene_open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/scene_save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    closeAct = new QAction(tr("&Close Scene"), this);
    closeAct->setShortcuts(QKeySequence::SaveAs);
    closeAct->setStatusTip(tr("Unload the curret scene"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(closeScene()));


    // add recent file list
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    // Edit
    cutAct = new QAction(QIcon(":/images/scene_edit_cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/scene_edit_copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/scene_edit_paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    // View
    showSceneEditAct = new QAction(tr("Show Scene Editor"), this);
    showSceneEditAct->setStatusTip(tr("Show\\Hide the scene editor panel"));
    showSceneEditAct->setCheckable(true);
    connect(showSceneEditAct, SIGNAL(toggled(bool)), sceneEditDockWidget, SLOT(setVisible(bool)));

    showConsoleAct = new QAction(tr("Show Console"), this);
    showConsoleAct->setStatusTip(tr("Show\\Hide the main console panel"));
    showConsoleAct->setCheckable(true);
    connect(showConsoleAct, SIGNAL(toggled(bool)), consoleDockWidget, SLOT(setVisible(bool)));

    // Render
    renderPlayAct = new QAction(QIcon(":/images/render_start.png"), tr("Start render"), this);
    renderPlayAct->setStatusTip(tr("Start the render engine"));
    connect(renderPlayAct, SIGNAL(triggered()), this, SLOT(renderPlay()));
	 renderPlayAct->setEnabled(false);

    renderPauseAct = new QAction(QIcon(":/images/render_pause.png"), tr("Pause render"), this);
    renderPauseAct->setStatusTip(tr("Pause the render engine"));
    connect(renderPauseAct, SIGNAL(triggered()), this, SLOT(renderPause()));
	 renderPauseAct->setEnabled(false);

    renderStopAct = new QAction(QIcon(":/images/render_stop.png"), tr("Stop render"), this);
    renderStopAct->setStatusTip(tr("Stop the render engine"));
    connect(renderStopAct, SIGNAL(triggered()), this, SLOT(renderStop()));
	 renderStopAct->setEnabled(false);

    // About
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));


    // add some logic
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setTearOffEnabled(true);
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(closeAct);
    fileMenu->addSeparator();
    separatorAct = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);

    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);


    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->setTearOffEnabled(true);
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->setTearOffEnabled(true);
    viewMenu->addAction(showSceneEditAct);
    viewMenu->addAction(showConsoleAct);

    renderMenu = menuBar()->addMenu(tr("&Render"));
    renderMenu->setTearOffEnabled(true);
    renderMenu->addAction(renderPlayAct);
    renderMenu->addAction(renderPauseAct);
    renderMenu->addAction(renderStopAct);

    renderVMenu = renderMenu->addMenu(tr("&Verbosity"));
    renderVMenu->setTearOffEnabled(true);
    //we map the render verbosity here as in the actions, - it crashes
    vMapper = new QSignalMapper(this);
    const char* vModes[] = {// "disp name"	"mode"
								"&None", 	"0",
								"&Info", 	"1",
								"&Progress","2",
								"&Debug", 	"3" };
    for (size_t i = 0; i < sizeof(vModes) / sizeof(const char*); i += 2)
    {
		QAction* vAct = renderVMenu->addAction(tr(vModes[i]));
		vMapper->setMapping(vAct, vModes[i + 1]);
		this->connect(vAct, SIGNAL(triggered()), vMapper, SLOT(map()));
    }
    connect(vMapper, SIGNAL(mapped(QString)), this, SLOT(renderVerbose(QString)));
    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
	 QSize iSize;
	 iSize.setHeight(28);
	 iSize.setWidth(28);

	 fileToolBar = new QToolBar;
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->setOrientation( Qt::Vertical );
    fileToolBar->setIconSize(iSize);
    this->addToolBar( Qt::LeftToolBarArea, fileToolBar);

    editToolBar = new QToolBar;
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->setOrientation( Qt::Vertical );
    editToolBar->setIconSize(iSize);
    this->addToolBar( Qt::LeftToolBarArea, editToolBar);

    renderToolBar = new QToolBar;
    renderToolBar->addAction(renderPlayAct);
    renderToolBar->addAction(renderPauseAct);
    renderToolBar->addAction(renderStopAct);
    renderToolBar->setOrientation( Qt::Vertical );
    renderToolBar->setIconSize(iSize);
    this->addToolBar( Qt::LeftToolBarArea, renderToolBar);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

// Open/Save routines /////////////////////////////////////////////////////
bool MainWindow::maybeSave()
{
    if (textEdit->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

	 //!< disable render buttons
	 renderPlayAct->setEnabled(true);
	 renderPauseAct->setEnabled(true);
	 renderStopAct->setEnabled(true);

    //!< EMIT signal for the renderer slot to load the scene
    emit sceneFileLoaded( std::string(fileName.toLocal8Bit().constData()) );


    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

// set current file (and update recent file list)
void MainWindow::setCurrentFile(const QString &fileName, bool recfilelist)
{
    curFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    if(recfilelist){
    // recent file list
    files = settings->value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings->setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
    }
    QString shownName = curFile;
    shownName += curFile;
    if (curFile.isEmpty())
        shownName = "Rombo: untitled.txt";
    setWindowFilePath(shownName);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        loadFile(action->data().toString());
}

// update recent file list ////////////////////////////////////////////////
void MainWindow::updateRecentFileActions()
{
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}


// Read/write settings //////////////////////////////////////////////
void MainWindow::readSettings()
{
    QPoint pos = settings->value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings->value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);

    files = settings->value("recentFileList").toStringList();
    updateRecentFileActions();
}

void MainWindow::writeSettings()
{
    QSettings settings("ctrlstudio", "rombo");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}


// Close event
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

