#ifndef GRIDSCENE_H
#define GRIDSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

#include <vector>

#include "../model/gridmodel.h"
#include "../model/pathfinder.h"

class GridScene final : public QGraphicsScene {
    Q_OBJECT

    static constexpr int CELL_SIZE = 30;
    static constexpr int INTERVAL_ms = 50;

public:
    explicit GridScene(GridModel *model, PathFinder *pathFinder, QObject *parent = nullptr);
    ~GridScene();

    void drawGrid();
    void clearPath();

public slots:
    void onGridChanged();
    void onPathFound(const std::vector<QPoint> &path, bool isPreview);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    GridModel *m_model;
    PathFinder *m_pathFinder;

    // Вектора путей для отрисовки
    std::vector<QPoint> m_currentPath;
    std::vector<QPoint> m_previewPath;

    // Вектора указателей на элементы путей для обработки
    std::vector<QGraphicsRectItem*> m_mainPathItems;
    std::vector<QGraphicsRectItem*> m_previewPathItems;

    QTimer m_previewTimer;
    QPoint m_pendingPreviewPoint;

    QColor getCellColor(CellType type) const;
    QPoint sceneToGrid(const QPointF &scenePos) const;

    void updatePreviewPath();
    void updateMainPathDisplay();

    void clearMainPathItems();
    void clearPreviewPathItems();
    void clearAllPathItems();
    QGraphicsRectItem* createMainPathItem(const QPoint& point);
    QGraphicsRectItem* createPreviewPathItem(const QPoint& point);
    bool shouldSkipPathPoint(const QPoint& point, bool isPreview) const;

    void onPreviewTimerTimeout();
};

#endif // GRIDSCENE_H
