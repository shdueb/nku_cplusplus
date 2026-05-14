#include "SnakeGame.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QKeyEvent>

SnakeGame::SnakeGame(QWidget *parent)
    : QWidget(parent)
{
    // 初始化基础参数
    m_gridSize = 20;
    m_blockSize = 20;
    resize(m_gridSize * m_blockSize, m_gridSize * m_blockSize);
    setWindowTitle("贪吃蛇");

    // 初始化蛇：初始长度3，默认向右移动
    m_dir = RIGHT;
    m_snake.clear();
    m_snake << QPoint(9, 9) << QPoint(8, 9) << QPoint(7, 9);
    m_gameOver = false;
    m_paused = false;

    // 生成第一个食物
    generateFood();

    // 初始化定时器：150ms刷新一次，控制移动速度
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SnakeGame::moveSnake);
    m_timer->start(150);
}

SnakeGame::~SnakeGame() {}

void SnakeGame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 绘制背景和网格
    painter.fillRect(rect(), Qt::white);
    painter.setPen(QPen(Qt::lightGray, 1));
    for (int i = 0; i <= m_gridSize; i++) {
        painter.drawLine(i * m_blockSize, 0, i * m_blockSize, m_gridSize * m_blockSize);
        painter.drawLine(0, i * m_blockSize, m_gridSize * m_blockSize, i * m_blockSize);
    }

    // 绘制蛇身
    painter.setBrush(Qt::green);
    painter.setPen(QPen(Qt::black, 1));
    for (const QPoint &point : m_snake) {
        painter.drawRect(point.x() * m_blockSize, point.y() * m_blockSize,
                         m_blockSize - 1, m_blockSize - 1);
    }

    // 绘制食物
    painter.setBrush(Qt::red);
    painter.drawEllipse(m_food.x() * m_blockSize, m_food.y() * m_blockSize,
                        m_blockSize - 1, m_blockSize - 1);

}

void SnakeGame::keyPressEvent(QKeyEvent *event)
{
    // 方向控制：禁止反向掉头（比如向右移动时不能直接向左转）
    switch (event->key()) {
    case Qt::Key_Up:
        if (m_dir != DOWN) m_dir = UP;
        break;
    case Qt::Key_Down:
        if (m_dir != UP) m_dir = DOWN;
        break;
    case Qt::Key_Left:
        if (m_dir != RIGHT) m_dir = LEFT;
        break;
    case Qt::Key_Right:
        if (m_dir != LEFT) m_dir = RIGHT;
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void SnakeGame::moveSnake()
{
    if (m_gameOver || m_paused) return;

    // 计算新蛇头位置
    QPoint head = m_snake.first();
    QPoint newHead = head;
    switch (m_dir) {
    case UP:    newHead.setY(newHead.y() - 1); break;
    case DOWN:  newHead.setY(newHead.y() + 1); break;
    case LEFT:  newHead.setX(newHead.x() - 1); break;
    case RIGHT: newHead.setX(newHead.x() + 1); break;
    }

    // 新蛇头插入到蛇身最前端
    m_snake.insert(0, newHead);

    // 吃到食物：保留尾巴，长度+1，重新生成食物；没吃到：删除尾巴，保持长度不变
    if (newHead == m_food) {
        generateFood();
    } else {
        m_snake.removeLast();
    }

    // 碰撞检测，触发游戏结束
    if (checkCollision()) {
        gameOver();
    }

    update(); // 触发界面重绘
}

void SnakeGame::generateFood()
{
    bool onSnake;
    // 循环生成直到得到一个不在蛇身上的位置
    do {
        int x = QRandomGenerator::global()->bounded(m_gridSize);
        int y = QRandomGenerator::global()->bounded(m_gridSize);
        m_food = QPoint(x, y);

        onSnake = false;
        for (const QPoint &point : m_snake) {
            if (point == m_food) {
                onSnake = true;
                break;
            }
        }
    } while (onSnake);
}

bool SnakeGame::checkCollision()
{
    QPoint head = m_snake.first();
    // 1. 撞墙检测：超出网格范围
    if (head.x() < 0 || head.x() >= m_gridSize || head.y() < 0 || head.y() >= m_gridSize) {
        return true;
    }
    // 2. 撞自身检测：蛇头碰到蛇身体
    for (int i = 1; i < m_snake.size(); i++) {
        if (head == m_snake[i]) {
            return true;
        }
    }
    return false;
}

void SnakeGame::gameOver()
{
    m_gameOver = true;
    m_timer->stop();
}