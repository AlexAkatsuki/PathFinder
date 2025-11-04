#include <random>

#include "gridmodel.h"

GridModel::GridModel(QObject *parent) : QObject(parent) {
}

void GridModel::initialize(int width, int height) {
    if (width < MIN_WIDTH_cnt) width = MIN_WIDTH_cnt;
    if (height < MIN_HEIGHT_cnt) height = MIN_HEIGHT_cnt;
    if (width > MAX_WIDTH_cnt) width = MAX_WIDTH_cnt;
    if (height > MAX_HEIGHT_cnt) height = MAX_HEIGHT_cnt;

    m_width = width;
    m_height = height;

    m_grid.clear();
    m_grid.resize(m_height);
    for (int i = 0; i < m_height; ++i) {
        m_grid[i].resize(m_width);
        for (int j = 0; j < m_width; ++j) {
            m_grid[i][j] = CellType::Empty;
        }
    }

    m_start = QPoint(-1, -1);
    m_end = QPoint(-1, -1);

    emit gridChanged();
}

void GridModel::generateRandomWalls(double wallProbability) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (dis(gen) < wallProbability)
                m_grid[y][x] = CellType::Wall;
            else
                m_grid[y][x] = CellType::Empty;
        }
    }

    if (isValidPoint(m_start) && !isWalkable(m_start.x(), m_start.y())) {
        m_start = QPoint(-1, -1);
        emit startPointChanged(m_start);
    }
    if (isValidPoint(m_end) && !isWalkable(m_end.x(), m_end.y())) {
        m_end = QPoint(-1, -1);
        emit endPointChanged(m_end);
    }
    emit gridChanged();
}

void GridModel::setCell(int x, int y, CellType type) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_grid[y][x] = type;
    }
}

CellType GridModel::getCell(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height)
        return m_grid[y][x];
    return CellType::Wall;
}

int GridModel::width() const {
    return m_width;
}

int GridModel::height() const {
    return m_height;
}

void GridModel::clearPoints() {
    // Очищаем старые точки на сетке
    if (isValidPoint(m_start)) {
        m_grid[m_start.y()][m_start.x()] = CellType::Empty;
    }
    if (isValidPoint(m_end)) {
        m_grid[m_end.y()][m_end.x()] = CellType::Empty;
    }

    m_start = QPoint(-1, -1);
    m_end = QPoint(-1, -1);

    emit startPointChanged(m_start);
    emit endPointChanged(m_end);
    emit gridChanged();  // Явно испускаем сигнал;
}

void GridModel::setStartPoint(const QPoint &point) {
    qDebug() << "GridModel::setStartPoint called with:" << point;
    if (isValidPoint(point) && isWalkable(point.x(), point.y())) {
        // Очищаем старую точку А
        if (isValidPoint(m_start)) {
            qDebug() << "Clearing old start point:" << m_start;
            m_grid[m_start.y()][m_start.x()] = CellType::Empty;
        }

        m_start = point;
        m_grid[point.y()][point.x()] = CellType::Start;
        qDebug() << "New start point set:" << m_start;
        emit startPointChanged(point);
        emit gridChanged();
    } else {
        qDebug() << "setStartPoint failed - point not valid or not walkable";
    }
}

void GridModel::setEndPoint(const QPoint &point) {
    qDebug() << "GridModel::setEndPoint called with:" << point;
    if (isValidPoint(point) && isWalkable(point.x(), point.y())) {
        // Очищаем старую точку Б
        if (isValidPoint(m_end)) {
            qDebug() << "Clearing old end point:" << m_end;
            m_grid[m_end.y()][m_end.x()] = CellType::Empty;
        }

        m_end = point;
        m_grid[point.y()][point.x()] = CellType::End;
        qDebug() << "New end point set:" << m_end;
        emit endPointChanged(point);
        emit gridChanged();
    } else {
        qDebug() << "setEndPoint failed - point not valid or not walkable";
    }
}

bool GridModel::hasStartPoint() const {
    bool hasStart = (m_start != QPoint(-1, -1));
    qDebug() << "hasStartPoint() returning:" << hasStart << "point:" << m_start;
    return hasStart;
}

bool GridModel::hasEndPoint() const {
    bool hasEnd = (m_end != QPoint(-1, -1));
    qDebug() << "hasEndPoint() returning:" << hasEnd << "point:" << m_end;
    return hasEnd;
}

bool GridModel::isValidPoint(const QPoint &point) const {
    return point.x() >= 0 && point.x() < m_width &&
           point.y() >= 0 && point.y() < m_height;
}

bool GridModel::isWalkable(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return false;
    return m_grid[y][x] != CellType::Wall;
}
