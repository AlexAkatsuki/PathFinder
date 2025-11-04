#include "gridscene.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QDebug>

GridScene::GridScene(GridModel *model, PathFinder *pathFinder, QObject *parent)
    : QGraphicsScene(parent), m_model(model), m_pathFinder(pathFinder) {

    m_previewTimer.setSingleShot(true);
    m_previewTimer.setInterval(INTERVAL_ms);
    connect(&m_previewTimer, &QTimer::timeout, this, &GridScene::onPreviewTimerTimeout);

    connect(m_model, &GridModel::gridChanged, this, &GridScene::onGridChanged);
    connect(m_pathFinder, &PathFinder::pathFound, this, &GridScene::onPathFound);

    setBackgroundBrush(QBrush(Qt::lightGray));
}

GridScene::~GridScene() {
    // На всякий случай чистим все элементы
    clearAllPathItems();
}

void GridScene::drawGrid() {
    clear();

    m_mainPathItems.clear();
    m_previewPathItems.clear();

    if (m_model->width() <= 0 || m_model->height() <= 0)
        return;

    for (int y = 0; y < m_model->height(); ++y) {
        for (int x = 0; x < m_model->width(); ++x) {
            auto cellType = m_model->getCell(x, y);
            auto color = getCellColor(cellType);

            QGraphicsRectItem *rect = new QGraphicsRectItem(
                x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE);

            rect->setBrush(QBrush(color));
            rect->setPen(QPen(Qt::black, 1));
            rect->setData(0, QPoint(x, y));

            addItem(rect);

            if (cellType == CellType::Start || cellType == CellType::End) {
                QString letter = (cellType == CellType::Start) ? tr("A") : tr("Б");
                QColor textColor = (cellType == CellType::Start) ? Qt::black : Qt::white;

                QGraphicsTextItem *textItem = new QGraphicsTextItem(letter);
                textItem->setDefaultTextColor(textColor);
                textItem->setFont(QFont("Arial", 12, QFont::Bold));

                QRectF textRect = textItem->boundingRect();
                textItem->setPos(
                    x * CELL_SIZE + (CELL_SIZE - textRect.width()) / 2,
                    y * CELL_SIZE + (CELL_SIZE - textRect.height()) / 2
                    );

                addItem(textItem);
            }
        }
    }

    QRectF sceneRect(0, 0, m_model->width() * CELL_SIZE, m_model->height() * CELL_SIZE);
    setSceneRect(sceneRect);
}

void GridScene::clearPath() {
    m_currentPath.clear();
    m_previewPath.clear();
    clearAllPathItems();
}

void GridScene::clearMainPathItems() {
    for (auto* item : m_mainPathItems) {
        removeItem(item);
        delete item;
    }
    m_mainPathItems.clear();
}

void GridScene::clearPreviewPathItems() {
    for (auto* item : m_previewPathItems) {
        removeItem(item);
        delete item;
    }
    m_previewPathItems.clear();
}

void GridScene::clearAllPathItems() {
    clearMainPathItems();
    clearPreviewPathItems();
}

QGraphicsRectItem* GridScene::createMainPathItem(const QPoint& point) {
    QGraphicsRectItem *pathRect = new QGraphicsRectItem(
        point.x() * CELL_SIZE, point.y() * CELL_SIZE, CELL_SIZE, CELL_SIZE);

    pathRect->setBrush(QBrush(Qt::blue));
    pathRect->setPen(QPen(Qt::black, 1));

    return pathRect;
}

QGraphicsRectItem* GridScene::createPreviewPathItem(const QPoint& point) {
    QGraphicsRectItem *previewRect = new QGraphicsRectItem(
        point.x() * CELL_SIZE, point.y() * CELL_SIZE, CELL_SIZE, CELL_SIZE);

    previewRect->setBrush(QBrush(QColor(173, 216, 230, 150)));
    previewRect->setPen(QPen(Qt::blue, 1));

    return previewRect;
}

bool GridScene::shouldSkipPathPoint(const QPoint& point, bool isPreview) const {
    CellType cellType = m_model->getCell(point.x(), point.y());

    if (cellType == CellType::Start || cellType == CellType::End)
        return true;

    if (isPreview) {
        if (point == m_previewPath.back())
            return true;
        if (std::find(m_currentPath.begin(), m_currentPath.end(), point) != m_currentPath.end())
            return true;
    }

    return false;
}

void GridScene::onGridChanged() {
    drawGrid();
}

void GridScene::onPathFound(const std::vector<QPoint> &path, bool isPreview) {
    if (isPreview) {
        m_previewPath = path;
        updatePreviewPath();
    } else {
        m_currentPath = path;
        updateMainPathDisplay();
    }
}

void GridScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QPoint gridPos = sceneToGrid(event->scenePos());

    if (m_model->isValidPoint(gridPos) && event->button() == Qt::LeftButton) {
        if (!m_model->isWalkable(gridPos.x(), gridPos.y())) {
            QGraphicsScene::mousePressEvent(event);
            return;
        }

        m_previewTimer.stop();

        // Можно заменить на Свитч по битовой маске, но возможно потеряем в читаемости
        bool hasStart = m_model->hasStartPoint();
        bool hasEnd = m_model->hasEndPoint();
        if (!hasStart) {
            m_model->setStartPoint(gridPos);
        }
        else if (!hasEnd) {
            m_model->setEndPoint(gridPos);
        }
        else {
            m_model->clearPoints();
            m_model->setStartPoint(gridPos);
        }

        clearPath();
    }

    QGraphicsScene::mousePressEvent(event);
}

void GridScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QPoint gridPos = sceneToGrid(event->scenePos());

    if (!m_model->isValidPoint(m_model->startPoint())) {
        if (!m_previewPath.empty()) {
            m_previewPath.clear();
            updatePreviewPath();
        }
        m_previewTimer.stop();
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }

    if (m_model->isValidPoint(gridPos) &&
        m_model->isWalkable(gridPos.x(), gridPos.y()) &&
        gridPos != m_model->startPoint()) {

        m_pendingPreviewPoint = gridPos;
        if (!m_previewTimer.isActive())
            m_previewTimer.start();
    }
    else {
        m_previewTimer.stop();
        if (!m_previewPath.empty()) {
            m_previewPath.clear();
            updatePreviewPath();
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
}

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

void GridScene::updatePreviewPath() {
    clearPreviewPathItems();

    if (m_previewPath.empty())
        return;

    for (const auto &point : m_previewPath) {
        if (shouldSkipPathPoint(point, true)) {
            continue;
        }

        QGraphicsRectItem* previewRect = createPreviewPathItem(point);
        addItem(previewRect);
        m_previewPathItems.push_back(previewRect);
    }
}

void GridScene::updateMainPathDisplay() {
    clearMainPathItems();

    if (m_currentPath.empty())
        return;

    for (const auto &point : m_currentPath) {
        if (shouldSkipPathPoint(point, false)) {
            continue;
        }

        QGraphicsRectItem* pathRect = createMainPathItem(point);
        addItem(pathRect);
        m_mainPathItems.push_back(pathRect);
    }
}

void GridScene::onPreviewTimerTimeout() {
    if (m_model->isValidPoint(m_pendingPreviewPoint) &&
        m_model->isValidPoint(m_model->startPoint()) &&
        m_model->isWalkable(m_pendingPreviewPoint.x(), m_pendingPreviewPoint.y()) &&
        m_pendingPreviewPoint != m_model->startPoint()) {

        QMetaObject::invokeMethod(m_pathFinder, "findPath", Qt::QueuedConnection,
                                  Q_ARG(QPoint, m_pendingPreviewPoint),
                                  Q_ARG(bool, true));
    } else {
        if (!m_previewPath.empty()) {
            m_previewPath.clear();
            updatePreviewPath();
        }
    }
}
