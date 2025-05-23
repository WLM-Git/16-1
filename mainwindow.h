#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia/QtMultimedia>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QMediaPlayer *playerl;                                  //播放器
    bool loopPlayer = true;                                 //是否循环播放
    QString durationTime;                                   //总时长,mm:ss 字符串
    QString positionTime;                                   //当前播放的时间节点,mm:ss 字符串
    QUrl getUrlFromItem;                                    //获取item的用户数据
    bool eventFilter(QObject *watched, QEvent *event);      //事件过滤处理
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void do_stateChanged(QMediaPlayer::PlaybackState state);        //播放器状态发生变化
    void do_sourceChanged(const QUrl &media);                       //播放源发生变化
    void do_durationChanged(qint64 duration);                       //播放时长发生变化
    void do_positionChanged(qint64 position);                       //播放位置发生变化
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
