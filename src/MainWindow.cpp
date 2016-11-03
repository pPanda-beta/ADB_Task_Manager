



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
	

	ui->Perf->xAxis->setLabel("Time -> ");
	ui->Perf->yAxis->setLabel("Cpu util %");
	ui->Perf->xAxis->setRange(1,mxX);
	ui->Perf->yAxis->setRange(0,100.0);
	
	ui->Perf->addGraph(ui->Perf->xAxis,ui->Perf->yAxis);
	ui->Perf->graph(0)->setBrush(QColor(100,100,255,100));
	ui->Perf->graph(0)->setName("Total");
	
	ui->Perf->addGraph(ui->Perf->xAxis,ui->Perf->yAxis);
	ui->Perf->graph(1)->setBrush(QColor(0,0,255,100));
	ui->Perf->graph(1)->setName("Kernel");
	

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
//			qDebug()<<ln<<" vs "<<isBlank;
	//		qDebug()<<"Mode begin :"<<mode;
				
			switch(mode)
			{
				case 0 :
						if(isBlank)
							break;
						else
						{
							mode=(mode+1)%4;
							auto &&row=ln.split(QRegExp("\\s+|%|,"),QString::SkipEmptyParts);
							u_u=row[1].toInt();//stoi(row[1].toStdString());
							s_u=row[3].toInt();//stoi(row[3].toStdString());
						}
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
						if(ks.size()<mxX)
							ks.push_back(ks.size()+1);
						vs[0].push_back((double)u_u+s_u);
						vs[1].push_back((double)s_u);
						while(vs[0].size()>mxX)
							for(auto &vl:vs)
								vl.pop_front();
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
		
		for(size_t i=0;i<sizeof(vs)/sizeof(vs[0]);i++)
			ui->Perf->graph(i)->setData(ks,vs[i],true);
		
		ui->Perf->replot();
	
		sem.release();
	}
}


