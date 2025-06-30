#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

//返回item的用户数据
QUrl MainWindow::getUrlFromItem(QListWidgetItem *item)
{
    QVariant itemData = item->data(Qt::UserRole);           //用户数据
    QUrl source = itemData.value<QUrl>();                   //QVariant转化为QUrl类型
    return source;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //不是KeyPress事件就退出
    if (event->type() != QEvent::KeyPress)
    {
        return QWidget::eventFilter(watched,event);
    }
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    //按下的不是Delete键就退出
    if (keyEvent->key() != Qt::Key_Delete)
    {
        return QWidget::eventFilter(watched,event);
    }
    if (watched == ui->listWidget_music)
    {
        QListWidgetItem *item = ui->listWidget_music->takeItem(ui->listWidget_music->currentRow());
        delete item;
    }
    return true;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listWidget_music->installEventFilter(this);                                     //安装事件过滤器
    ui->listWidget_music->setDragEnabled(true);                                         //允许拖放操作
    ui->listWidget_music->setDragDropMode(QAbstractItemView::InternalMove);             //列表项可在组件内部被拖放

    player = new QMediaPlayer(this);
    QAudioOutput *audioOutput = new QAudioOutput(this);                                 //音频输出对象
    player->setAudioOutput(audioOutput);                                                //设置播放器的音频输出对象
    connect(player, &QMediaPlayer::positionChanged,
            this, &MainWindow::do_positionChanged);                                     //播放位置发生变化
    connect(player, &QMediaPlayer::durationChanged,
            this, &MainWindow::do_durationChanged);                                     //播放总时长发生变化
    connect(player, &QMediaPlayer::sourceChanged,
            this, &MainWindow::do_sourceChanged);                                       //播放源发生变化
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &MainWindow::do_stateChanged);                                        //播放状态发生变化
    connect(player, &QMediaPlayer::metaDataChanged,
            this, &MainWindow::do_metaDataChanged);                                     //元数据发生变化
}



MainWindow::~MainWindow()
{
    delete ui;
}

//播放器状态发生变化时运行
void MainWindow::do_stateChanged(QMediaPlayer::PlaybackState state)
{
    ui->btnPlay->setEnabled(state != QMediaPlayer::PlayingState);
    ui->btnPause->setEnabled(state == QMediaPlayer::PlayingState);
    ui->btnStop->setEnabled(state == QMediaPlayer::PlayingState);
    //播放完一曲后，如果此时处于循环播放状态，则播放下一首音乐
    if (ui->btnLoop->isEnabled())
    {
        //获取当前歌曲所在列表行数
        int curRow = ui->listWidget_music->currentRow();
        //获取列表总行数
        int count = ui->listWidget_music->count();
        curRow++;
        curRow = curRow >= count?0:curRow;
        ui->listWidget_music->setCurrentRow(curRow);
        player->setSource(getUrlFromItem(ui->listWidget_music->currentItem()));
        player->play();
    }
}

//播放的文件发生变化时
void MainWindow::do_sourceChanged(const QUrl &media)
{
    ui->labCurMedia->setText(media.fileName());
}

//播放源时长发生变化时运行，更新进度显示
void MainWindow::do_durationChanged(qint64 duration)
{
    ui->sliderPosition->setMaximum(duration);
    int secs = duration/1000;                               //总秒数
    int mins = secs/60;                                     //分数
    secs %= 60;                                             //取余秒数
    durationTime = QString::asprintf("%d:%d",mins,secs);
    ui->labRatio->setText(positionTime+"/"+durationTime);
}

//播放位置发生变化时运行，更新进度显示
void MainWindow::do_positionChanged(qint64 position)
{
    if (ui->sliderPosition->isSliderDown())                             //当进度条正在被滑动时，直接忽略
    {
        return;
    }
    ui->sliderPosition->setSliderPosition(position);
    int secs = position/1000;                                           //当前进度的总秒数
    int mins = secs/60;                                                 //当前进度的分数
    secs %= 60;                                                         //当前进度的取余秒数
    positionTime = QString::asprintf("%d:d%",mins,secs);
    ui->labRatio->setText(positionTime+"/"+durationTime);
}

//元数据变化时运行，显示歌曲对应的图片
void MainWindow::do_metaDataChanged()
{
    QMediaMetaData metaData = player->metaData();                            //元数据对象
    QVariant metaImg = metaData.value(QMediaMetaData::ThumbnailImage);       //图片数据
    if (metaImg.isValid())
    {
        QImage img = metaImg.value<QImage>();                                //QVariant转化为QImage
        QPixmap musicPixmp = QPixmap::fromImage(img);
        ui->labPic->setPixmap(musicPixmp);
    }
    else
        ui->labPic->clear();
}

void MainWindow::on_btnAdd_clicked()
{
    QString curPath = QDir::homePath();             //获取系统当前目录
    QString dlgTitle = "选择音频文件";
    QString filter = "音频文件(*.mp3 *.wav *.wma);;所有文件(*.*)";        //文件过滤器
    QStringList fileList = QFileDialog::getOpenFileNames(this,dlgTitle,curPath,filter);
    if (fileList.count() < 1)
    {
        return;
    }
    for (int i = 0; i < fileList.size(); ++i)
    {
        QString aFile = fileList.at(i);
        QFileInfo fileInfo(aFile);
        QListWidgetItem *aItem = new QListWidgetItem(fileInfo.fileName());
        aItem->setIcon(QIcon(":/pitcure/images/musicFile.png"));
        aItem->setData(Qt::UserRole,QUrl::fromLocalFile(aFile));        //设置用户数据
        ui->listWidget_music->addItem(aItem);                           //添加到界面上的列表
    }

    //当前不处于播放就播放列表里的第一个文件
    if (player->PlayingState != QMediaPlayer::PlayingState)
    {
        ui->listWidget_music->setCurrentRow(0);
        QUrl source = getUrlFromItem(ui->listWidget_music->currentItem());      //获取播放媒介
        player->setSource(source);
    }
    player->play();
}

//“移除”按钮，移除列表中的当前项
void MainWindow::on_btnRemove_clicked()
{
    int index = ui->listWidget_music->currentRow();
    if (index >= 0)
    {
        QListWidgetItem *item = ui->listWidget_music->takeItem(index);
        delete item;
    }
}

//“清空”按钮，清空播放按钮
void MainWindow::on_btnClear_clicked()
{
    loopPlayer = false;                 //防止do_stateChanged()里切换曲目
    ui->listWidget_music->clear();
    player->stop();
}

//双击listWidget时切换曲目
void MainWindow::on_listWidget_music_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    loopPlayer = false;                 //防止do_stateChanged()里切换曲目
    player->setSource(getUrlFromItem(ui->listWidget_music->currentItem()));
    player->play();
    loopPlayer = ui->btnLoop->isChecked();
}

//前一曲
void MainWindow::on_btnPrevious_clicked()
{
    int curRow = ui->listWidget_music->currentRow();
    curRow--;
    curRow = curRow<0? 0:curRow;
    ui->listWidget_music->setCurrentRow(curRow);                    //设置当前行
    loopPlayer = false;                 //防止do_stateChanged()里切换曲目
    player->setSource(getUrlFromItem(ui->listWidget_music->currentItem()));
    player->play();
    loopPlayer = ui->btnLoop->isChecked();
}

//播放按钮
void MainWindow::on_btnPlay_clicked()
{
    //播放选中行的歌曲，若没有选中则播放第一首歌曲
    if (ui->listWidget_music->currentRow() < 0)
    {
        ui->listWidget_music->setCurrentRow(0);
    }
    //设置播放器的播放源
    player->setSource(getUrlFromItem(ui->listWidget_music->currentItem()));
    //设置循环播放按钮的值
    loopPlayer = ui->btnLoop->isEnabled();
    //播放器开始播放
    player->play();
}

//暂停播放
void MainWindow::on_btnPause_clicked()
{
    player->pause();
}

//停住播放
void MainWindow::on_btnStop_clicked()
{
    loopPlayer = false;
    player->stop();
}

//倍速
void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    player->setPlaybackRate(arg1);
}


//循环设置
void MainWindow::on_btnLoop_clicked(bool checked)
{
    loopPlayer = checked;
}

//播放进度控制
void MainWindow::on_sliderPosition_valueChanged(int value)
{
    player->setPosition(value);
}

//静音设置
void MainWindow::on_btnSound_clicked()
{
    //先获取当前的静音状态
    bool mute = player->audioOutput()->isMuted();
    //将当前的静音状态设为相反值
    player->audioOutput()->setMuted(!mute);
    //根据静音状态设置静音按钮的图标
    if (mute)
    {
        ui->btnSound->setIcon(QIcon(":/icon/images/volumn.bmp"));
    }
    else
    {
        ui->btnSound->setIcon(QIcon(":/icon/images/mute.bmp"));
    }
}

//调整音量
void MainWindow::on_sliderVolume_valueChanged(int value)
{
    player->audioOutput()->setVolume(value/100.0);
}

