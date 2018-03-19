#include <boost/circular_buffer.hpp>

#include "fitness_graph.h"
#include "ui_fitness_graph.h"

#include "../species.h"


fitness_graph::fitness_graph(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::fitness_graph)
{
	ui->setupUi(this);

	auto chart_fitness = new QtCharts::QChart();
	chart_fitness->setTheme(QtCharts::QChart::ChartThemeDark);

	series_fitness = new QtCharts::QLineSeries();
	series_min_fitness = new QtCharts::QLineSeries();
	series_max_fitness = new QtCharts::QLineSeries();


	series_fitness_avg = new QtCharts::QLineSeries();
	QPen pen;
	pen.setStyle(Qt::DashLine);
	pen.setColor("white");
	series_fitness_avg->setPen(pen);
	series_min_fitness_avg = new QtCharts::QLineSeries();
	series_min_fitness_avg->setPen(pen);
	series_max_fitness_avg = new QtCharts::QLineSeries();
	series_max_fitness_avg->setPen(pen);

	chart_fitness->addSeries(series_fitness);
	chart_fitness->addSeries(series_min_fitness);
	chart_fitness->addSeries(series_max_fitness);
	chart_fitness->addSeries(series_fitness_avg);
	chart_fitness->addSeries(series_min_fitness_avg);
	chart_fitness->addSeries(series_max_fitness_avg);


	chart_fitness->createDefaultAxes();
	chart_fitness->legend()->hide();
	ui->fitness->setChart(chart_fitness);
	ui->fitness->setRenderHint(QPainter::Antialiasing);
	ui->fitness->setRubberBand(QtCharts::QChartView::RectangleRubberBand);

	auto_range = true;

	score_window_max = 10;

	score_count = 0;
	score_total_avg = 0;
	score_total_min = 0;
	score_total_max = 0;

	scores_avg.set_capacity(score_window_max*2+1);
	scores_min.set_capacity(score_window_max*2+1);
	scores_max.set_capacity(score_window_max*2+1);
}

fitness_graph::~fitness_graph()
{
	delete ui;
}

void fitness_graph::insert_fitness(int epoch, double fitness, double minimum_fitness, double maximum_fitness) {
	series_fitness->append(epoch, fitness);
	series_min_fitness->append(epoch, minimum_fitness);
	series_max_fitness->append(epoch, maximum_fitness);
	append_avg(epoch, fitness, minimum_fitness, maximum_fitness);
	const auto [min, max] = std::minmax({ fitness, minimum_fitness, maximum_fitness });
	if(epoch < series_range.xmin) series_range.xmin = epoch;
	if(min < series_range.ymin) series_range.ymin = min;
	if(epoch > series_range.xmax) series_range.xmax = epoch;
	if(max > series_range.ymax) series_range.ymax = max;
	if(auto_range) {
		ui->fitness->chart()->axisX()->setRange(series_range.xmin, series_range.xmax);
		ui->fitness->chart()->axisY()->setRange(series_range.ymin, series_range.ymax);
	}
}

void fitness_graph::on_auto_range_clicked(bool checked) {
	ui->fitness->chart()->axisX()->setRange(series_range.xmin, series_range.xmax);
	ui->fitness->chart()->axisY()->setRange(series_range.ymin, series_range.ymax);
	auto_range = checked;
}

void fitness_graph::append_avg(double epoch, double fitness, double minimum_fitness, double maximum_fitness) {
	score_count++;
	//Remove oldest element from total scores if circular_list is full
	if(score_count >= score_window_max*2+1) {
		score_total_avg -= scores_avg[0];
		score_total_min -= scores_min[0];
		score_total_max -= scores_max[0];
	}
	//Add new elements to circular buffer
	scores_avg.push_back(fitness);
	scores_min.push_back(minimum_fitness);
	scores_max.push_back(maximum_fitness);

	//Add the new fitness to the totals
	score_total_avg += fitness;
	score_total_min += minimum_fitness;
	score_total_max += maximum_fitness;

	if(score_count >= score_window_max*2+1) {
		//Set the new values
		series_fitness_avg->append(epoch-score_window_max, score_total_avg / (score_window_max*2+1));
		series_max_fitness_avg->append(epoch-score_window_max, score_total_min / (score_window_max*2+1));
		series_min_fitness_avg->append(epoch-score_window_max, score_total_max / (score_window_max*2+1));
	}
}
