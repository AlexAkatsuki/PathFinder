#ifndef GRIDSCENE_H
#define GRIDSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include "../model/gridmodel.h"
#include "../model/pathfinder.h"

class GridScene : public QGraphicsScene {
    Q_OBJECT

    static constexpr int CELL_SIZE = 30;

public:
    explicit GridScene(GridModel *model, PathFinder *pathFinder, QObject *parent = nullptr);

    void drawGrid();
    void clearPath();

public slots:
    void onGridChanged();
    void onPathFound(const std::vector<QPoint> &path);
    void onPathToFound(const std::vector<QPoint> &path);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    //void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    GridModel *m_model;

    PathFinder *m_pathFinder;

    std::vector<QPoint> m_currentPath;
    std::vector<QPoint> m_previewPath;

    QColor getCellColor(CellType type) const;

    QPoint sceneToGrid(const QPointF &scenePos) const;

    void updatePathDisplay();
    void updatePreviewPath();
};

#endif // GRIDSCENE_H
