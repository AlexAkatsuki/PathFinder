#include "pathfinder.h"
#include <queue>
#include <vector>
#include <QDebug>

PathFinder::PathFinder(GridModel *model, QObject *parent)
    : QObject(parent), m_model(model) {

    this->moveToThread(&m_workerThread);
    m_workerThread.start();
}

PathFinder::~PathFinder() {
    if (m_workerThread.isRunning()) {
        m_workerThread.quit();

        if (!m_workerThread.wait(1000))
            m_workerThread.terminate();
    }
}

void PathFinder::findPath(const QPoint& endPoint, bool isPreview) {
    auto path = bfs(m_model->startPoint(), endPoint);

    if (isPreview) {
        emit pathFound(path, true);
    } else {
        if (path.empty())
            emit pathNotFound();
        else
            emit pathFound(path, false);
        emit calculationFinished();
    }
}

std::vector<QPoint> PathFinder::bfs(const QPoint &start, const QPoint &end) {
    if (start == end)
        return {start};

    if (!m_model->isValidPoint(start) || !m_model->isValidPoint(end) ||
        !m_model->isWalkable(end.x(), end.y()))
        return {};

    int width = m_model->width();
    int height = m_model->height();

    std::queue<QPoint> queue;
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::vector<std::vector<QPoint>> cameFrom(height,
                                        std::vector<QPoint>(width, QPoint(-1, -1)));

    queue.push(start);
    visited[start.y()][start.x()] = true;

    const QPoint directions[] = {
        QPoint(0, 1),   // вниз
        QPoint(1, 0),   // вправо
        QPoint(0, -1),  // вверх
        QPoint(-1, 0)   // влево
    };

    int steps = 0;

    while (!queue.empty()) {
        if (QThread::currentThread()->isInterruptionRequested())
            return {};

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
                m_model->isWalkable(nx, ny) && !visited[ny][nx]) {

                visited[ny][nx] = true;
                cameFrom[ny][nx] = current;
                queue.push(neighbor);
            }
        }
    }
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
