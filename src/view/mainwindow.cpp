#include <QGraphicsView>
#include <QDockWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QWheelEvent>

#include "../model/gridmodel.h"
#include "../model/pathfinder.h"

#include "mainwindow.h"
#include "gridscene.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_graphicsView(nullptr)
    , m_widthSpinBox(nullptr)
    , m_heightSpinBox(nullptr)
    , m_generateButton(nullptr)
    , m_findPathButton(nullptr)
    , m_instructionsLabel(nullptr)
    , m_widthLabel(nullptr)
    , m_heightLabel(nullptr)
    , m_model(new GridModel(this))
    , m_pathFinder(new PathFinder(m_model, this))
    , m_scene(nullptr)
    , m_settings("PathFinder", "PathFindingApp") {

    setupUI();
    setupConnections();
    restoreWindowState();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    setWindowTitle("Path Finder");

    m_graphicsView = new QGraphicsView(this);
    m_scene = new GridScene(m_model, m_pathFinder, this);
    m_graphicsView->setScene(m_scene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

    setCentralWidget(m_graphicsView);

    m_widthLabel = new QLabel("Ширина:");
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setMinimum(MIN_SPINBOX_VAL);
    m_widthSpinBox->setMaximum(MAX_SPINBOX_VAL);
    m_widthSpinBox->setValue(DEFAULT_SPINBOX_VAL);

    m_heightLabel = new QLabel("Высота:");
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setMinimum(MIN_SPINBOX_VAL);
    m_heightSpinBox->setMaximum(MAX_SPINBOX_VAL);
    m_heightSpinBox->setValue(DEFAULT_SPINBOX_VAL);

    m_generateButton = new QPushButton("Генерировать");
    m_findPathButton = new QPushButton("Найти путь");

    m_instructionsLabel = new QLabel(
        "Установка точек (только левая кнопка мыши):\n"
        "• Первый клик - точка А (зеленая)\n"
        "• Второй клик - точка Б (красная)\n"
        "• Третий клик - сброс и новая точка А\n"
        "• И так далее...\n\n"
        "Ctrl + Колесо - масштабирование"
        );
    m_instructionsLabel->setAlignment(Qt::AlignCenter);
    m_instructionsLabel->setWordWrap(true);

    QWidget *controlWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(controlWidget);

    QHBoxLayout *sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(m_widthLabel);
    sizeLayout->addWidget(m_widthSpinBox);

    QHBoxLayout *sizeLayout2 = new QHBoxLayout();
    sizeLayout2->addWidget(m_heightLabel);
    sizeLayout2->addWidget(m_heightSpinBox);

    mainLayout->addLayout(sizeLayout);
    mainLayout->addLayout(sizeLayout2);
    mainLayout->addWidget(m_generateButton);
    mainLayout->addWidget(m_findPathButton);
    mainLayout->addWidget(m_instructionsLabel);
    mainLayout->addStretch();

    QDockWidget *controlDock = new QDockWidget("Управление", this);
    controlDock->setWidget(controlWidget);
    controlDock->setFixedWidth(DOCK_WIDTH);
    addDockWidget(Qt::LeftDockWidgetArea, controlDock);
}

void MainWindow::setupConnections() {
    connect(m_generateButton, &QPushButton::clicked, this,
                              &MainWindow::onGenerateClicked);
    connect(m_findPathButton, &QPushButton::clicked, this,
                              &MainWindow::onFindPathClicked);
    connect(m_pathFinder, &PathFinder::calculationFinished, this,
                          &MainWindow::onCalculationFinished);
    connect(m_pathFinder, &PathFinder::pathNotFound, this,
                          &MainWindow::onPathNotFound);
}

void MainWindow::onGenerateClicked() {
    if (!validateInput())
        return;

    int width = m_widthSpinBox->value();
    int height = m_heightSpinBox->value();

    m_model->initialize(width, height);

    m_model->generateRandomWalls();
    m_scene->clearPath();

    m_graphicsView->viewport()->update();
}

void MainWindow::onFindPathClicked() {
    if (m_model->width() == 0 || m_model->height() == 0) {
        showError("Пожалуйста, сначала создайте сетку (нажмите 'Генерировать')");
        return;
    }
    if (!m_model->hasStartPoint()) {
        showError("Пожалуйста, установите начальную точку А (левая кнопка мыши)");
        return;
    }
    if (!m_model->hasEndPoint()) {
        showError("Пожалуйста, установите конечную точку Б (левая кнопка мыши)");
        return;
    }
    m_findPathButton->setEnabled(false);
    m_pathFinder->findPath();
}

void MainWindow::onCalculationFinished() {
    m_findPathButton->setEnabled(true);
}

void MainWindow::onPathNotFound() {
    m_findPathButton->setEnabled(true);

    showError("Не удалось найти путь от точки А до точки Б!\n\n"
              "Возможные причины:\n"
              "• Одна из точек окружена препятствиями\n"
              "• Между точками нет прохода\n"
              "• Точки находятся в изолированных областях\n\n"
              "Попробуйте:\n"
              "• Перегенерировать сетку\n"
              "• Переместить точки в другие места\n"
              );
}

bool MainWindow::validateInput() {
    int width = m_widthSpinBox->value();
    int height = m_heightSpinBox->value();

    if (width * height > MAX_GRID_SIZE) {
        showError("Выбран большой размер сетки (" + QString::number(width) + "×" +
            QString::number(height) + " = " + QString::number(width * height) + " ячеек).\n\n"
            "Это может замедлить:\n"
            "• Генерацию сетки\n"
            "• Поиск пути\n"
            "• Отображение\n\n"
            "Рекомендуется использовать размер до 50×50."
        );
    }
    return true;
}

void MainWindow::showError(const QString &message) {
    QMessageBox::critical(this, "Ошибка", message);
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        double scaleFactor = 1.1;
        if (event->angleDelta().y() < 0)
            scaleFactor = 1.0 / scaleFactor;

        m_graphicsView->scale(scaleFactor, scaleFactor);
        event->accept();
    } else
        QMainWindow::wheelEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveWindowState();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveWindowState() {
    m_settings.setValue("window/geometry", saveGeometry());
    m_settings.setValue("window/state", saveState());

    m_settings.setValue("settings/width", m_widthSpinBox->value());
    m_settings.setValue("settings/height", m_heightSpinBox->value());
}

void MainWindow::restoreWindowState() {
    if (m_settings.contains("window/geometry")) {
        restoreGeometry(m_settings.value("window/geometry").toByteArray());
    } else {
        resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
        move(DEFAULT_X_POSE, DEFAULT_Y_POSE);
    }

    if (m_settings.contains("window/state"))
        restoreState(m_settings.value("window/state").toByteArray());
    if (m_settings.contains("settings/width"))
        m_widthSpinBox->setValue(m_settings.value("settings/width").toInt());
    if (m_settings.contains("settings/height"))
        m_heightSpinBox->setValue(m_settings.value("settings/height").toInt());
}
