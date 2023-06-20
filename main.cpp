#include <QApplication>
#include <QPushButton>
#include <QtWidgets>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        savedText = ""; // Initialize the saved text as empty

        // Set the initial size of the main window
        resize(800, 600);

        // Create the main text editor widget
        textEdit = new QTextEdit(this);
        setCentralWidget(textEdit);

        // Create the menu bar
        createMenuBar();

        // Create the toolbar
        createToolBar();

        // Create the status bar
        createStatusBar();

        // Set window properties
        setWindowTitle("CoMiKx Text Editor");
        setWindowIcon(QIcon(R"(..\icons\pencil.png)"));
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        if (hasUnsavedChanges()) {
            QMessageBox::StandardButton response = askToSaveChanges();
            if (response == QMessageBox::Save) {
                saveFile();
            } else if (response == QMessageBox::Cancel) {
                event->ignore();  // Ignore the close event
                return;
            }
        }

        QMainWindow::closeEvent(event);  // Perform the default close event handling
    }

private:
    QTextEdit *textEdit;
    QString currentFilePath;  // Store the path of the currently opened file
    QString savedText;

    void createMenuBar() {
        auto *menuBar = new QMenuBar(this);

        // File menu
        QMenu *fileMenu = menuBar->addMenu("File");

        QAction *newAction = fileMenu->addAction("New");
        connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

        QAction *openAction = fileMenu->addAction("Open");
        connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

        QAction *saveAction = fileMenu->addAction("Save");
        connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

        QAction *saveAsAction = fileMenu->addAction("Save As");
        connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

        fileMenu->addSeparator();

        fileMenu->addAction("Exit", this, &MainWindow::onExit); // Connect the action to a slot

        // Edit menu
        QMenu *editMenu = menuBar->addMenu("Edit");
        editMenu->addAction("Find");
        editMenu->addAction("Replace");

        // View menu
        QMenu *viewMenu = menuBar->addMenu("View");
        viewMenu->addAction("Zoom In");
        viewMenu->addAction("Zoom Out");
        viewMenu->addSeparator();
        viewMenu->addAction("Toggle Fullscreen");

        // Help menu
        QMenu *helpMenu = menuBar->addMenu("Help");
        helpMenu->addAction("About");

        setMenuBar(menuBar);
    }

    void createToolBar() {
        auto* toolBar = new QToolBar(this);

        QAction* newAction = createToolbarAction(toolBar, R"(..\icons\new.png)", "New", "Create a new file");
        connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

        QAction* openAction = createToolbarAction(toolBar, R"(..\icons\open.png)", "Open", "Open an exiting file");
        connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

        QAction* saveAction = createToolbarAction(toolBar, R"(..\icons\save.png)", "Save", "Save file");
        connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

        toolBar->addSeparator();

        // Create the "Cut" action
        auto* cutAction = new QAction(QIcon(R"(..\icons\cut.png)"), "Cut", this);
        cutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
        toolBar->addAction(cutAction);
        connect(cutAction, &QAction::triggered, textEdit, &QTextEdit::cut);

        // Create the "Copy" action
        auto* copyAction = new QAction(QIcon(R"(..\icons\copy.png)"), "Copy", this);
        copyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
        toolBar->addAction(copyAction);
        connect(copyAction, &QAction::triggered, textEdit, &QTextEdit::copy);

        // Create the "Paste" action
        auto* pasteAction = new QAction(QIcon(R"(..\icons\paste.png)"), "Paste", this);
        pasteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
        toolBar->addAction(pasteAction);
        connect(pasteAction, &QAction::triggered, textEdit, &QTextEdit::paste);

        toolBar->addSeparator();

        // Create the "Undo" action
        auto* undoAction = new QAction(QIcon(R"(..\icons\undo.png)"), "Undo", this);
        undoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
        toolBar->addAction(undoAction);
        connect(undoAction, &QAction::triggered, textEdit, &QTextEdit::undo);

        // Create the "Redo" action
        auto* redoAction = new QAction(QIcon(R"(..\icons\redo.png)"), "Redo", this);
        redoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
        toolBar->addAction(redoAction);
        connect(redoAction, &QAction::triggered, textEdit, &QTextEdit::redo);

        addToolBar(toolBar);
    }

    void createStatusBar() {
        auto *statusBar = new QStatusBar(this);
        setStatusBar(statusBar);
    }

    static QAction* createToolbarAction(QToolBar* toolbar, const QString& iconPath, const QString& text, const QString& tooltip = "") {
        auto* action = new QAction(QIcon(iconPath), text, toolbar);
        action->setToolTip(tooltip);
        toolbar->addAction(action);
        return action;
    }

    void newFile() {
        // Prompt the user to save the current file if it's modified
        if (textEdit->document()->isModified()) {
            int result = QMessageBox::question(this, "Save changes", "Do you want to save changes to the current file?",
                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            if (result == QMessageBox::Save) {
                saveFile();
            } else if (result == QMessageBox::Cancel) {
                return;
            }
        }

        // Clear the text edit and reset the current file path
        textEdit->clear();
        currentFilePath.clear();
    }

    void openFile() {
        // Prompt the user to save the current file if it's modified
        if (textEdit->document()->isModified()) {
            int result = QMessageBox::question(this, "Save changes", "Do you want to save changes to the current file?",
                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            if (result == QMessageBox::Save) {
                saveFile();
            } else if (result == QMessageBox::Cancel) {
                return;
            }
        }

        // Open a file dialog to choose a file to open
        QString filePath = QFileDialog::getOpenFileName(this, "Open File");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                // Read the contents of the file
                QTextStream in(&file);
                QString contents = in.readAll();

                // Display the contents in the text edit
                textEdit->setPlainText(contents);

                // Set the current file path
                currentFilePath = filePath;

                // Reset the modification flag
                textEdit->document()->setModified(false);

                // Set the window title to the file name
                setWindowTitle(QFileInfo(filePath).fileName());

                file.close();
            } else {
                QMessageBox::warning(this, "Error", "Failed to open the file.");
            }
        }
    }

    void saveFile() { // NOLINT
        QString currentText = textEdit->toPlainText();

        savedText = currentText;

        if (currentFilePath.isEmpty()) {
            saveFileAs();
        } else {
            QFile file(currentFilePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << textEdit->toPlainText();

                textEdit->document()->setModified(false);

                file.close();
            } else {
                QMessageBox::warning(this, "Error", "Failed to save the file.");
            }
        }
    }

    void saveFileAs() { // NOLINT
        QString suggestedFileName = "Untitled.txt";
        QString selectedFilter;

        QString fileName = QFileDialog::getSaveFileName(this, "Save File As", suggestedFileName, "Text Files (*.txt);;Batch Files (*.bat);;All Files (*.*)", &selectedFilter);
        if (!fileName.isEmpty()) {
            currentFilePath = fileName;
            saveFile();
            setWindowTitle(QFileInfo(fileName).fileName());
        }
    }

    void onExit() {
        if (hasUnsavedChanges()) {
            QMessageBox::StandardButton response = askToSaveChanges();
            if (response == QMessageBox::Save) {
                saveFile();
            } else if (response == QMessageBox::Cancel) {
                return;  // Don't exit if the user cancels
            }
        }

        QApplication::quit();  // Exit the application
    }

    static QMessageBox::StandardButton askToSaveChanges() {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Question);
        messageBox.setWindowTitle("Save Changes");
        messageBox.setText("There are unsaved changes. Do you want to save?");
        messageBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Save);

        return static_cast<QMessageBox::StandardButton>(messageBox.exec());
    }

    bool hasUnsavedChanges() {
        // Implement the logic to check if there are any unsaved changes
        // For example, you can compare the current text with the saved text

        // Assuming you have a member variable to store the saved text
        QString Text = savedText;  // Replace this with the actual saved text

        // Compare the current text with the saved text
        return textEdit->toPlainText() != Text;
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
}