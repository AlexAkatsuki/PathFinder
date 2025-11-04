#ifndef GRIDMODEL_H
#define GRIDMODEL_H

#include <QObject>
#include <QPoint>

#include <vector>

enum class CellType {
    Empty,
    Wall,
    Start,
    End,
    Path,
    Visited
};

class GridModel : public QObject {
    Q_OBJECT

    static constexpr int8_t MAX_WIDTH_cnt = 100;
    static constexpr int8_t MAX_HEIGHT_cnt = 100;

    static constexpr int8_t MIN_WIDTH_cnt = 1;
    static constexpr int8_t MIN_HEIGHT_cnt = 1;

public:
    GridModel(QObject *parent = nullptr);

    void initialize(int width, int height);

    void generateRandomWalls(double wallProbability = 0.3);

    void setCell(int x, int y, CellType type);
    CellType getCell(int x, int y) const;

    int width() const;
    int height() const;

    QPoint startPoint() const { return m_start; }
    QPoint endPoint() const { return m_end; }

    void clearPoints();

    bool hasStartPoint() const;
    bool hasEndPoint() const;

    void setStartPoint(const QPoint &point);
    void setEndPoint(const QPoint &point);

    bool isValidPoint(const QPoint &point) const;
    bool isWalkable(int x, int y) const;

signals:
    void gridChanged();
    void startPointChanged(const QPoint &point);
    void endPointChanged(const QPoint &point);

private:
    int m_width = 0;
    int m_height = 0;

    std::vector<std::vector<CellType>> m_grid;

    QPoint m_start = QPoint(-1, -1);
    QPoint m_end = QPoint(-1, -1);
};

#endif // GRIDMODEL_H
