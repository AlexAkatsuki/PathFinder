#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QObject>
#include <QPoint>
#include <QThread>

#include <vector>

#include "gridmodel.h"

class PathFinder : public QObject {
    Q_OBJECT

public:
    explicit PathFinder(GridModel *model, QObject *parent = nullptr);
    ~PathFinder();

public slots:
    void findPath(const QPoint& endPoint, bool isPreview = false);

signals:
    void pathFound(const std::vector<QPoint> &path, bool isPreview);
    void calculationFinished();
    void pathNotFound();

private:
    GridModel *m_model;
    QThread m_workerThread;

    std::vector<QPoint> bfs(const QPoint &start, const QPoint &end);
    std::vector<QPoint> reconstructPath(const std::vector<std::vector<QPoint>> &cameFrom,
                                        const QPoint &current);
};

#endif // PATHFINDER_H
