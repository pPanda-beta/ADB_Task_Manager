#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDebug>

#include <iostream>
#include <sstream>
#include <atomic>
#include <mutex>
#include <thread>

#include <QProcess>
#include <QSemaphore>

#include "qcustomplot.h"

using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
	int m_nop,m_del,u_u,s_u,mxX=25;
	std::thread ipc;
	std::mutex mt;
	atomic<int> running,mode;
	shared_ptr<QProcess> qtop=NULL;
	
	QList<QStringList> ll;
	QSemaphore sem;
	QString bl;
	
	QVector<double> ks,vs[2];
	void restarttopProc();
private slots:
	void on_tableWidget_cellActivated(int row, int column);
	
	void on_del_valueChanged(int arg1){	m_del=arg1; restarttopProc();	}
	void on_nop_valueChanged(int arg1){	m_nop=arg1; restarttopProc();	}
	
private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
