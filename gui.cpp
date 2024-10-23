#include <QApplication>
#include <QFileSystemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>

class FileSystemExplorer : public QWidget {
public:
    FileSystemExplorer(QWidget *parent = nullptr) : QWidget(parent) {
        // Layout
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Label to show current directory
        QLabel *directoryLabel = new QLabel("Current Directory:", this);
        layout->addWidget(directoryLabel);

        // File system model to handle file browsing
        QFileSystemModel *model = new QFileSystemModel(this);
        model->setRootPath(QDir::rootPath());

        // Tree view to display the file system
        QTreeView *treeView = new QTreeView(this);
        treeView->setModel(model);
        treeView->setRootIndex(model->index(QDir::rootPath()));
        treeView->setSortingEnabled(true);  // Enable sorting by name, size, etc.

        // Add the tree view to the layout
        layout->addWidget(treeView);

        // Button to open a selected file (dummy action for now)
        QPushButton *openButton = new QPushButton("Open File", this);
        layout->addWidget(openButton);

        connect(openButton, &QPushButton::clicked, [=]() {
            QModelIndex index = treeView->currentIndex();
            if (index.isValid()) {
                QString filePath = model->filePath(index);
                // You can handle opening the file here (e.g., using QFile or QProcess)
                directoryLabel->setText("Selected File: " + filePath);
            }
        });

        setLayout(layout);
        setWindowTitle("Simple File Explorer");
        resize(600, 400);  // Set initial window size
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    FileSystemExplorer explorer;
    explorer.show();

    return app.exec();
}
