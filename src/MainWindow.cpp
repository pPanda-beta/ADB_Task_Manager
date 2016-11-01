#include "MainWindow.h"
#include "ui_MainWindow.h"

int threads=0;
thread_local int tid=++threads;

#define g qDebug()<<"\nDebug["<<tid<<"] @ "<<" #"<<__LINE__<<"\t"<<__PRETTY_FUNCTION__ ;




void setDataset(QTableWidget *tb,auto &&ll)
{
	tb->setRowCount(0);
	for(auto &cells:ll)
	{
		tb->insertRow(tb->rowCount());
		int j=0;
		for(auto &cl:cells)
			tb->setItem(tb->rowCount()-1,j++, new QTableWidgetItem(cl));
	}
}



MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	
	running=1;
	mode=-1;
	m_nop=ui->nop->value();
	m_del=ui->del->value();
	
	auto &&header=tr("  PID PR CPU% S  #THR     VSS     RSS PCY UID      Name")
			.split(QRegExp("\\s+"),QString::SkipEmptyParts);
	qDebug()<<header;
	ui->tableWidget->setColumnCount(header.size());
	ui->tableWidget->setHorizontalHeaderLabels(header);
	
	g
	ipc=std::thread ([&]
	{
		while(running)
		{
			if(mode==-1)
			{
				mode=0;
				ll.clear();
				stringstream cmd;
#ifdef Q_OS_WIN32
				cmd<<"adb shell ";
#endif
				cmd<<"top "<<" -m "<<m_nop<<" -d "<<m_del;
				
//				QProcess::execute("taskkill /IM adb.exe /F");
				qtop=make_shared<QProcess>();
				this_thread::yield();
				qtop->start(tr(cmd.str().data()));
				qtop->waitForStarted(); 
				qDebug()<<qtop->state()<<":"<<cmd.str().data();
				
			}
			
			
			QString ln;
			{
				if(not qtop->canReadLine())
					qtop->waitForReadyRead();
			
				if(mode==-1 or qtop->state()!= QProcess::Running)
					continue;
				unique_lock<mutex> lg(mt);
				ln=qtop->readLine();
			}
			bool isBlank=ln.trimmed().isEmpty();
		//	qDebug()<<ln<<" vs "<<isBlank;
		//	qDebug()<<"Mode begin :"<<mode;
				
			switch(mode)
			{
				case 0 :
						if(not isBlank)
							mode=(mode+1)%4;
						else
							break;
				case 1 :
					bl=ln;
					mode=(mode+1)%4;					
					break;
				case 2 :
					
					if(ln.contains("PID"))
						mode=(mode+1)%4;
					break;
				case 3 :
					auto &&row=ln.split(QRegExp("\\s+"),QString::SkipEmptyParts);
					if(row.size()<10)
						row.insert(7,"NA");
				//	if(not isBlank)
					ll.push_back(row);
				//	else
					if(ll.size()==m_nop)
					{
						mode=(mode+1)%4;
						QMetaObject::invokeMethod(ui->tableWidget,"cellActivated",
												  Qt::QueuedConnection,Q_ARG(int,-1),Q_ARG(int,-1));
					
						sem.acquire();
						ll.clear();
					}
			}
		//	qDebug()<<"Mode End :"<<mode;
		} 
	});
	
}


MainWindow::~MainWindow()
{
	running=0;
	mode=-1;
	sem.release(sem.available());
	unique_lock<mutex> lg(mt);
	qtop->kill();
	if(ipc.joinable())
		ipc.join();
	delete ui;
}

void MainWindow::restarttopProc()
{
	sem.release(sem.available());
	mode=-1;
	unique_lock<mutex> lg(mt);
	if(qtop.get())
		qtop->kill();
}

void MainWindow::on_tableWidget_cellActivated(int row, int column)
{
    if(row < 0 and column < 0 and running)
	{
		setDataset(ui->tableWidget,ll);

		
		ui->tableWidget->resizeColumnsToContents();
		ui->tableWidget->resizeRowsToContents();
		ui->statusBar->showMessage(bl);
		sem.release();
	}
}


