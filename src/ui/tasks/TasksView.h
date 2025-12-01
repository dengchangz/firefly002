#ifndef TASKSVIEW_H
#define TASKSVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QVector>
#include <QString>
#include <QDateTime>

struct TaskInfo {
    QString id;
    QString name;
    QDateTime createdAt;
    QString summary;
    QString commissioner;
};

class TaskCard;

class TasksView : public QWidget
{
    Q_OBJECT
public:
    explicit TasksView(QWidget* parent = nullptr);

    void setTasks(const QVector<TaskInfo>& tasks);
    void addTaskFront(const TaskInfo& task);

signals:
    void newTaskRequested();
    void collectRequested(const QString& taskId);
    void analyzeRequested(const QString& taskId);
    void deleteRequested(const QString& taskId);

private:
    void rebuildGrid();
    QWidget* createNewTaskCard();
    QWidget* createTaskCard(const TaskInfo& info);

private:
    QScrollArea* m_scrollArea;
    QWidget* m_container;
    QGridLayout* m_grid;
    QVector<TaskInfo> m_tasks;
    int m_columns;
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // TASKSVIEW_H
