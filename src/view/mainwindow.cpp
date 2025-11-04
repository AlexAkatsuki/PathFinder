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
    resize(1000, 700);

    // Создаем центральный graphics view
    m_graphicsView = new QGraphicsView(this);
    m_scene = new GridScene(m_model, m_pathFinder, this);
    m_graphicsView->setScene(m_scene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

    setCentralWidget(m_graphicsView);

    // Создаем элементы управления
    m_widthLabel = new QLabel("Ширина:");
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setMinimum(5);
    m_widthSpinBox->setMaximum(100);
    m_widthSpinBox->setValue(20);

    m_heightLabel = new QLabel("Высота:");
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setMinimum(5);
    m_heightSpinBox->setMaximum(100);
    m_heightSpinBox->setValue(20);

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

    // Создаем layout для элементов управления
    QWidget *controlWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(controlWidget);

    // Группа размера сетки
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(m_widthLabel);
    sizeLayout->addWidget(m_widthSpinBox);

    QHBoxLayout *sizeLayout2 = new QHBoxLayout();
    sizeLayout2->addWidget(m_heightLabel);
    sizeLayout2->addWidget(m_heightSpinBox);

    // Добавляем все в основной layout
    mainLayout->addLayout(sizeLayout);
    mainLayout->addLayout(sizeLayout2);
    mainLayout->addWidget(m_generateButton);
    mainLayout->addWidget(m_findPathButton);
    mainLayout->addWidget(m_instructionsLabel);
    mainLayout->addStretch(); // Растягивающийся элемент для выравнивания

    // Создаем dock widget
    QDockWidget *controlDock = new QDockWidget("Управление", this);
    controlDock->setWidget(controlWidget);
    controlDock->setFixedWidth(200);
    addDockWidget(Qt::LeftDockWidgetArea, controlDock);
}

void MainWindow::setupConnections() {
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateClicked);
    connect(m_findPathButton, &QPushButton::clicked, this, &MainWindow::onFindPathClicked);
    connect(m_pathFinder, &PathFinder::calculationFinished, this, &MainWindow::onCalculationFinished);
    connect(m_pathFinder, &PathFinder::pathNotFound, this, &MainWindow::onPathNotFound);
}

void MainWindow::onGenerateClicked() {
    if (!validateInput())
        return;

    int width = m_widthSpinBox->value();
    int height = m_heightSpinBox->value();

    m_model->initialize(width, height);

    m_model->generateRandomWalls();
    m_scene->clearPath();

    // Принудительно обновляем view
    m_graphicsView->viewport()->update();
}

void MainWindow::onFindPathClicked() {
    qDebug() << "=== Find Path Clicked ===";
    qDebug() << "Start point exists:" << m_model->hasStartPoint() << "point:" << m_model->startPoint();
    qDebug() << "End point exists:" << m_model->hasEndPoint() << "point:" << m_model->endPoint();

    // Проверяем создана ли сетка
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

    qDebug() << "Starting path finding...";
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

    qDebug() << "Path not found between points";
}

bool MainWindow::validateInput() {
    int width = m_widthSpinBox->value();
    int height = m_heightSpinBox->value();

    if (width * height > 2500) { // 50×50
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

void MainWindow::closeEvent(QCloseEvent *event) {
    saveWindowState();
    QMainWindow::closeEvent(event);
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        // Масштабирование с Ctrl + Колесо
        double scaleFactor = 1.1;
        if (event->angleDelta().y() < 0)
            scaleFactor = 1.0 / scaleFactor;

        m_graphicsView->scale(scaleFactor, scaleFactor);
        event->accept();
    } else
        QMainWindow::wheelEvent(event);
}

void MainWindow::saveWindowState() {
    m_settings.setValue("geometry", saveGeometry());
    m_settings.setValue("windowState", saveState());
}

void MainWindow::restoreWindowState() {
    if (m_settings.contains("geometry"))
        restoreGeometry(m_settings.value("geometry").toByteArray());
    if (m_settings.contains("windowState"))
        restoreState(m_settings.value("windowState").toByteArray());
}
