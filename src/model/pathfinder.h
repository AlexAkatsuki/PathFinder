#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QObject>
#include <QPoint>
#include <QThread>

#include <vector>

//#include "gridmodel.h"

class PathFinder : public QObject {
    Q_OBJECT

    static constexpr uint16_t BFS_PATH_LIMIT_cnt = 1000;

    static constexpr uint16_t GRID_SIZE_LIMIT = 10000;

public:
    explicit PathFinder(GridModel *model, QObject *parent = nullptr);
    ~PathFinder();

public slots:
    void findPath();
    void findPathTo(const QPoint &target);

signals:
    void pathFound(const std::vector<QPoint> &path);
    void pathToFound(const std::vector<QPoint> &path);
    void calculationFinished();

private:
    GridModel *m_model;

    QThread m_workerThread;

    std::vector<QPoint> bfs(const QPoint &start, const QPoint &end);
    std::vector<QPoint> bfsWithLimit(const QPoint &start, const QPoint &end, int stepLimit);
    std::vector<QPoint> reconstructPath(const std::vector<std::vector<QPoint>> &cameFrom, const QPoint &current);
};

#endif // PATHFINDER_H
