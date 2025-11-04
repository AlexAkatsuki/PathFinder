#include <QThread>

#include <queue>
#include <vector>

#include "pathfinder.h"

PathFinder::PathFinder(GridModel *model, QObject *parent)
    : QObject(parent), m_model(model) {
}

PathFinder::~PathFinder() {
    if (m_workerThread.isRunning()) {
        m_workerThread.quit();
        m_workerThread.wait();
    }
}

void PathFinder::findPath() {
    if (!m_model->isValidPoint(m_model->startPoint()) ||
        !m_model->isValidPoint(m_model->endPoint())) {
        emit calculationFinished();
        return;
    }

    auto path = bfs(m_model->startPoint(), m_model->endPoint());
    emit pathFound(path);
    emit calculationFinished();
}

void PathFinder::findPathTo(const QPoint &target) {
    if (!m_model->isValidPoint(m_model->startPoint()) ||
        !m_model->isValidPoint(target) ||
        !m_model->isWalkable(target.x(), target.y())) {
        emit pathToFound({});
        return;
    }

    auto path = bfsWithLimit(m_model->startPoint(), target, BFS_PATH_LIMIT_cnt);
    emit pathToFound(path);
}

std::vector<QPoint> PathFinder::bfs(const QPoint &start, const QPoint &end) {
    if (start == end)
        return {start};

    int width = m_model->width();
    int height = m_model->height();

    if (width * height > GRID_SIZE_LIMIT)
        return {};

    std::queue<QPoint> queue;
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::vector<std::vector<QPoint>> cameFrom(height, std::vector<QPoint>(width, QPoint(-1, -1)));

    queue.push(start);
    visited[start.y()][start.x()] = true;

    const QPoint directions[] = {
        QPoint(0, 1),   // down
        QPoint(1, 0),   // right
        QPoint(0, -1),  // up
        QPoint(-1, 0)   // left
    };

    int steps = 0;
    const int maxSteps = width * height * 2; // Защита от бесконечного цикла

    while (!queue.empty() && steps < maxSteps) {
        QPoint current = queue.front();
        queue.pop();
        steps++;

        // Периодически проверяем, не нужно ли прервать выполнение
        if (steps % 1000 == 0)
            QThread::msleep(1); // Даем дыхать главному потоку

        if (current == end)
            return reconstructPath(cameFrom, current);

        for (const auto &dir : directions) {
            QPoint neighbor = current + dir;
            int nx = neighbor.x();
            int ny = neighbor.y();

            if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                m_model->isWalkable(nx, ny) &&
                !visited[ny][nx]) {

                visited[ny][nx] = true;
                cameFrom[ny][nx] = current;
                queue.push(neighbor);
            }
        }
    }
    return {};
}

std::vector<QPoint> PathFinder::bfsWithLimit(const QPoint &start,
                                             const QPoint &end,
                                             int stepLimit) {
    if (start == end)
        return {start};

    int width = m_model->width();
    int height = m_model->height();

    std::queue<QPoint> queue;
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::vector<std::vector<QPoint>> cameFrom(height, std::vector<QPoint>(width, QPoint(-1, -1)));

    queue.push(start);
    visited[start.y()][start.x()] = true;

    const QPoint directions[] = {
        QPoint(0, 1),   // down
        QPoint(1, 0),   // right
        QPoint(0, -1),  // up
        QPoint(-1, 0)   // left
    };

    int steps = 0;

    while (!queue.empty() && steps < stepLimit) {
        QPoint current = queue.front();
        queue.pop();
        steps++;

        if (current == end)
            return reconstructPath(cameFrom, current);

        for (const auto &dir : directions) {
            QPoint neighbor = current + dir;
            int nx = neighbor.x();
            int ny = neighbor.y();

            if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                m_model->isWalkable(nx, ny) &&
                !visited[ny][nx]) {

                visited[ny][nx] = true;
                cameFrom[ny][nx] = current;
                queue.push(neighbor);
            }
        }
    }

    // Для preview возвращаем частичный путь если нашли
    if (!queue.empty()) // Возвращаем путь до последней обработанной точки
        return reconstructPath(cameFrom, queue.front());

    return {};
}

std::vector<QPoint> PathFinder::reconstructPath(
                            const std::vector<std::vector<QPoint>> &cameFrom,
                            const QPoint &current) {
    std::vector<QPoint> path;
    QPoint currentPoint = current;

    while (currentPoint != QPoint(-1, -1)) {
        path.push_back(currentPoint);
        currentPoint = cameFrom[currentPoint.y()][currentPoint.x()];
    }

    std::reverse(path.begin(), path.end());
    return path;
}
