#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class QGraphicsView;
class QSpinBox;
class QPushButton;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QWidget;
class QDockWidget;
class GridModel;
class PathFinder;
class GridScene;
class QSettings;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void onGenerateClicked();
    void onFindPathClicked();
    void onCalculationFinished();
    void onPathNotFound();
    void showError(const QString &message);

private:
    QGraphicsView *m_graphicsView;
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
    QPushButton *m_generateButton;
    QPushButton *m_findPathButton;
    QLabel *m_instructionsLabel;
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;

    GridModel *m_model;
    PathFinder *m_pathFinder;
    GridScene *m_scene;

    QDockWidget *m_controlDock;

    QSettings m_settings;

    void setupUI();
    void setupConnections();
    void saveWindowState();
    void restoreWindowState();
    bool validateInput();
};

#endif // MAINWINDOW_H
