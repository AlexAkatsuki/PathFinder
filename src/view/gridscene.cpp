#include "gridscene.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QDebug>

GridScene::GridScene(GridModel *model, PathFinder *pathFinder, QObject *parent)
    : QGraphicsScene(parent), m_model(model), m_pathFinder(pathFinder) {

    connect(m_model, &GridModel::gridChanged, this, &GridScene::onGridChanged);
    connect(m_pathFinder, &PathFinder::pathFound, this, &GridScene::onPathFound);
    connect(m_pathFinder, &PathFinder::pathToFound, this, &GridScene::onPathToFound);

    setBackgroundBrush(QBrush(Qt::lightGray));
}

void GridScene::drawGrid()
{
    qDebug() << "Drawing grid:" << m_model->width() << "x" << m_model->height();

    // Полностью очищаем сцену
    clear();

    if (m_model->width() <= 0 || m_model->height() <= 0) {
        return;
    }

    const int cellSize = 30;

    // Рисуем все ячейки
    for (int y = 0; y < m_model->height(); ++y) {
        for (int x = 0; x < m_model->width(); ++x) {
            auto cellType = m_model->getCell(x, y);
            auto color = getCellColor(cellType);

            QGraphicsRectItem *rect = new QGraphicsRectItem(
                x * cellSize, y * cellSize, cellSize, cellSize);

            rect->setBrush(QBrush(color));
            rect->setPen(QPen(Qt::black, 1));
            rect->setData(0, QPoint(x, y));

            addItem(rect);

            // Добавляем буквы для точек А и Б
            if (cellType == CellType::Start || cellType == CellType::End) {
                QString letter = (cellType == CellType::Start) ? "A" : "Б";
                QColor textColor = (cellType == CellType::Start) ? Qt::black : Qt::white;

                QGraphicsTextItem *textItem = new QGraphicsTextItem(letter);
                textItem->setDefaultTextColor(textColor);
                textItem->setFont(QFont("Arial", 12, QFont::Bold));

                // Центрируем текст в ячейке
                QRectF textRect = textItem->boundingRect();
                textItem->setPos(
                    x * cellSize + (cellSize - textRect.width()) / 2,
                    y * cellSize + (cellSize - textRect.height()) / 2
                    );

                addItem(textItem);
            }
        }
    }

    // Устанавливаем размер сцены
    QRectF sceneRect(0, 0, m_model->width() * cellSize, m_model->height() * cellSize);
    setSceneRect(sceneRect);
}

void GridScene::clearPath() {
    m_currentPath.clear();
    m_previewPath.clear();
}

void GridScene::onGridChanged() {
    drawGrid();
}

void GridScene::onPathFound(const std::vector<QPoint> &path) {
    m_currentPath = path;
    updatePathDisplay();
}

void GridScene::onPathToFound(const std::vector<QPoint> &path) {
    m_previewPath = path;
    updatePreviewPath();
}

void GridScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QPoint gridPos = sceneToGrid(event->scenePos());
    qDebug() << "=== Mouse Press ===";
    qDebug() << "Grid position:" << gridPos;
    qDebug() << "Current hasStart:" << m_model->hasStartPoint() << "hasEnd:" << m_model->hasEndPoint();

    if (m_model->isValidPoint(gridPos) && event->button() == Qt::LeftButton) {

        if (!m_model->isWalkable(gridPos.x(), gridPos.y())) {
            qDebug() << "Cannot set point on wall";
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        // Простая логика
        if (!m_model->hasStartPoint()) {
            m_model->setStartPoint(gridPos);
        }
        else if (!m_model->hasEndPoint()) {
            m_model->setEndPoint(gridPos);
        }
        else {
            m_model->clearPoints();
            m_model->setStartPoint(gridPos);
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

// void GridScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
//     QPoint gridPos = sceneToGrid(event->scenePos());
//     if (m_model->isValidPoint(gridPos) && m_model->isValidPoint(m_model->startPoint()))
//         m_pathFinder->findPathTo(gridPos);
//     else
//         onPathToFound({});

//     QGraphicsScene::mouseMoveEvent(event);
// }

QColor GridScene::getCellColor(CellType type) const {
    switch (type) {
        case CellType::Empty:   return Qt::white;
        case CellType::Wall:    return Qt::darkGray;
        case CellType::Start:   return Qt::green;
        case CellType::End:     return Qt::red;
        case CellType::Path:    return Qt::blue;
        case CellType::Visited: return QColor(255, 255, 200);
        default:                return Qt::white;
    }
}

QPoint GridScene::sceneToGrid(const QPointF &scenePos) const {
    int x = static_cast<int>(scenePos.x()) / CELL_SIZE;
    int y = static_cast<int>(scenePos.y()) / CELL_SIZE;
    return QPoint(x, y);
}

void GridScene::updatePathDisplay() {
    if (m_currentPath.empty())
        return;

    for (const auto &point : m_currentPath) {
        // Не перекрашиваем старт и финиш
        if (m_model->getCell(point.x(), point.y()) == CellType::Start ||
            m_model->getCell(point.x(), point.y()) == CellType::End) {
            continue;
        }

        // Создаем прямоугольник для пути
        QGraphicsRectItem *pathRect = new QGraphicsRectItem(
            point.x() * CELL_SIZE, point.y() * CELL_SIZE, CELL_SIZE, CELL_SIZE);

        pathRect->setBrush(QBrush(Qt::blue));
        pathRect->setPen(QPen(Qt::black, 1));

        addItem(pathRect);
    }
}

void GridScene::updatePreviewPath() {
    // Простой способ - перерисовать сетку и добавить preview
    // Это не оптимально, но гарантированно работает
    drawGrid(); // Перерисовываем базовую сетку

    if (m_previewPath.empty()) return;

    for (const auto &point : m_previewPath) {
        // Не перекрашиваем старт, финиш и текущую целевую точку
        if (m_model->getCell(point.x(), point.y()) == CellType::Start ||
            m_model->getCell(point.x(), point.y()) == CellType::End ||
            point == m_previewPath.back()) {
            continue;
        }

        QGraphicsRectItem *previewRect = new QGraphicsRectItem(
            point.x() * CELL_SIZE, point.y() * CELL_SIZE, CELL_SIZE, CELL_SIZE);

        previewRect->setBrush(QBrush(QColor(173, 216, 230, 150))); // Полупрозрачный голубой
        previewRect->setPen(QPen(Qt::blue, 1));

        addItem(previewRect);
    }

    // Также отображаем основной путь если он есть
    if (!m_currentPath.empty())
        updatePathDisplay();
}
